/**HEADER********************************************************************
*
* Copyright (c) 2008 Freescale Semiconductor;
* All Rights Reserved
*
* Copyright (c) 2004-2008 Embedded Access Inc.;
* All Rights Reserved
*
* Copyright (c) 1989-2008 ARC International;
* All Rights Reserved
*
***************************************************************************
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* $FileName: sosel.c$
* $Version : 3.8.11.0$
* $Date    : Jun-7-2012$
*
* Comments:
*
*   This file contains the RTCS_select() implementation.
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "socket.h"
#include "tcpip.h"
#include "tcp_prv.h"    /* for TCP internal definitions */
#include "udp_prv.h"    /* for UDP internal definitions */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_select_activity
* Returned Value  : TRUE or FALSE
* Comments        : Determine whether there is any activity on a socket.
*
*END*-----------------------------------------------------------------*/

boolean SOCK_select_activity
   (
      SOCKET_STRUCT_PTR    socket_ptr,
      int flag                          // direction flag - 0 recv, non zero send
   )
{
   TCB_STRUCT_PTR tcb_ptr;
   boolean        activity = FALSE;

   /* Check closing and connected stream sockets for data */
   if (socket_ptr->TCB_PTR)
   {
      tcb_ptr = socket_ptr->TCB_PTR;
   
      if (!flag)
      {
         /*
         ** Check that there is data in the receive ring or
         ** that the socket has been closed by the peer
         */
         if ((tcb_ptr->conn_pending) || (tcb_ptr->state == CLOSED) || (GT32(tcb_ptr->rcvnxt, tcb_ptr->rcvbufseq)))
         {
            activity = TRUE;
         }
      }
      else
      {
         if (!tcb_ptr->sndlen) // TODO - remake for partialy empty send buffers
         {    
            activity = TRUE;
         }
      }

   /* Check datagram sockets for data */
   } 
   else if (socket_ptr->UCB_PTR)
   {
      /*
      ** Check that there is queued data
      */
      if (socket_ptr->UCB_PTR->PHEAD)
      {
         activity = TRUE;
      } /* Endif */

   /* TCB=UCB=NULL is a TCP connection reset by peer */
   }
   else
   {
      activity = TRUE;
   }

   return activity;

}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_select_block
* Returned Values :
* Comments  :
*     Enqueues a select() call on the select queue.
*
*END*-----------------------------------------------------------------*/

void SOCK_select_block
   (
      SOCKSELECT_PARM_PTR  parms
   )
{ /* Body */
   SOCKET_CONFIG_STRUCT_PTR   socket_cfg_ptr = RTCS_getcfg(SOCKET);

   parms->NEXT = socket_cfg_ptr->SELECT_HEAD;
   if (parms->NEXT) {
      parms->NEXT->PREV = &parms->NEXT;
   } /* Endif */
   socket_cfg_ptr->SELECT_HEAD = parms;
   parms->PREV = &socket_cfg_ptr->SELECT_HEAD;

   if (parms->timeout) {
      parms->EXPIRE.TIME    = parms->timeout;
      parms->EXPIRE.EVENT   = SOCK_select_expire;
      parms->EXPIRE.PRIVATE = parms;
      TCPIP_Event_add(&parms->EXPIRE);
   } /* Endif */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_select_expire
* Returned Values :
* Comments  :
*     Called by the Timer.  Expire a select call.
*
*END*-----------------------------------------------------------------*/

boolean SOCK_select_expire
   (
      TCPIP_EVENT_PTR   event
   )
{ /* Body */
   SOCKSELECT_PARM_PTR  parms = event->PRIVATE;

   if (parms->NEXT) {
      parms->NEXT->PREV = parms->PREV;
   } /* Endif */
   *parms->PREV = parms->NEXT;

   parms->sock = 0;
   RTCSCMD_complete(parms, RTCS_OK);

   return FALSE;
} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_select_unblock
* Returned Values :
* Comments  :
*     Dequeues a select() call from the select queue.
*
*END*-----------------------------------------------------------------*/

void SOCK_select_unblock
   (
      SOCKSELECT_PARM_PTR  parms,
      uint_16  state      // TCP state or UDP socket flag
   )
{

   if (parms->NEXT) {
      parms->NEXT->PREV = parms->PREV;
   } 

   *parms->PREV = parms->NEXT;

   if (parms->timeout)
   {
      TCPIP_Event_cancel(&parms->EXPIRE);
   } 

   /* Check TCP state and UDP socket flag */
   if ( (state == CLOSED) || (state == UDP_SOCKET_CLOSE) )
   {
      RTCSCMD_complete(parms, RTCSERR_SOCK_CLOSED);
   } 
   else
   {
      RTCSCMD_complete(parms, RTCS_OK);
   }
    

}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_select_signal
* Returned Values :
* Comments  :
*     Unblocks all select() calls blocked on a specific socket.
*
*END*-----------------------------------------------------------------*/

void _SOCK_select_signal
   (
      uint_32  sock,
      uint_16  state,      // TCP state or UDP socket flag
      uint_32  flag        // direction flag - 0 recv, non zero send
   )
{ /* Body */
   SOCKET_CONFIG_STRUCT_PTR   socket_cfg_ptr = RTCS_getcfg(SOCKET);
   SOCKSELECT_PARM_PTR        parms;
   SOCKSELECT_PARM_PTR        nextparms;
   uint_32                    i;

   if (!sock)
   {
      return;
   } /* Endif */

   for (parms = socket_cfg_ptr->SELECT_HEAD; parms; parms = nextparms)
   {
      nextparms = parms->NEXT;
      if (parms->owner)
      {
         if (SOCK_Is_owner((SOCKET_STRUCT_PTR)sock, parms->owner))
         {
            parms->sock = sock;
            SOCK_select_unblock(parms, state);
         } /* Endif */
      } 
      else 
      {
         /* check if sock is on the select waiting list (array) */
         if (!flag)
         {         
            for (i = 0; i < parms->sock_count; i++)
            {
                if (parms->sock_ptr[i] == sock)
                {
                    break;
                }  
            }
               
            if(i >= parms->sock_count)
            {
                continue;
            }
               
         }         
         /* now, it is sure the select will be awaken. 
            Just zero out the others in both arrays */
         parms->sock = sock;
         
         SOCK_select_unblock(parms, state);
      } /* Endif */

   } /* Endfor */

   /*
   ** Finally, call the application callback, if one exists.
   */
   if (((SOCKET_STRUCT_PTR)sock)->APPLICATION_CALLBACK)
   {
      ((SOCKET_STRUCT_PTR)sock)->APPLICATION_CALLBACK(sock);
   } /* Endif */

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : _RTCS_selectset
* Returned Value  : socket handle
* Comments  :  Wait for data or connection requests on any socket
*      in a specified set.
*
*END*-----------------------------------------------------------------*/
uint_32 RTCS_selectset(pointer sockset, uint_32 size, uint_32 timeout)
{
   SOCKSELECT_PARM   parms;
   uint_32           error;

   parms.sock_ptr   = sockset;
   parms.sock_count = size;
   parms.timeout    = timeout;

   error = RTCSCMD_issue(parms, SOCK_selectset);
   if (error)
   {
      return RTCS_SOCKET_ERROR;
   }
   return parms.sock;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_select
* Parameters      :
*
*     _rtcs_taskid      owner            not used
*     uint_32 _PTR_     sock_ptr         [IN] array of socket handles
*     uint_32           sock_count       [IN] size of socket array
*     uint_32           timeout          [IN] timeout, 0=forever, -1=none
*     uint_32           sock             [OUT] socket with activity
*
* Comments  :  Wait for data or connection requests on any socket
*      in array.
*
*END*-----------------------------------------------------------------*/
void SOCK_selectset(SOCKSELECT_PARM_PTR  parms)
{ 
   uint_32                    size = parms->sock_count;
   SOCKET_STRUCT_PTR          socket_ptr = NULL;
   int                        i = 0;
   
   for (i = 0; i < size; i++) 
   {
      if ((parms->sock_ptr+i) && ((uint_32)(parms->sock_ptr+i) != RTCS_SOCKET_ERROR) && (*(parms->sock_ptr+i) != 0))
      {
         if (SOCK_select_activity((SOCKET_STRUCT_PTR) *(parms->sock_ptr+i), 0))
         {  
            socket_ptr = (SOCKET_STRUCT_PTR) *(parms->sock_ptr+i);
            break;
         }
      }
   }

   if (socket_ptr)
   {
      parms->sock = (uint_32) socket_ptr;
      RTCSCMD_complete(parms, RTCS_OK);
      return;
   }

   if (parms->timeout == (uint_32)-1) 
   {
      parms->sock = 0;
      RTCSCMD_complete(parms, RTCS_OK);
      return;
   }

   SOCK_select_block(parms);
} 

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_selectall
* Returned Value  : socket handle
* Comments  :  Wait for data or connection requests on any socket
*      owned by this task.
*
*END*-----------------------------------------------------------------*/

uint_32 RTCS_selectall
   (
      uint_32     timeout
         /* [IN] specifies the maximum amount of time to wait for data */
   )
{ /* Body */
   SOCKSELECT_PARM   parms;
   uint_32           error;

   parms.owner   = RTCS_task_getid();
   parms.timeout = timeout;
   error = RTCSCMD_issue(parms, SOCK_selectall);
   if (error) return RTCS_SOCKET_ERROR;

   return parms.sock;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_selectall
* Parameters      :
*
*     _rtcs_taskid      owner       [IN] socket owner
*     uint_32 _PTR_     sock_ptr    not used
*     uint_32           sock_count  not used
*     uint_32           timeout     [IN] timeout, 0=forever, -1=none
*     uint_32           sock        [OUT] socket with activity
*
* Comments  :  Wait for data or connection requests on any socket
*      owned by this task.
*
*END*-----------------------------------------------------------------*/

void SOCK_selectall
   (
      SOCKSELECT_PARM_PTR  parms
   )
{
   SOCKET_CONFIG_STRUCT_PTR   socket_cfg_ptr = RTCS_getcfg(SOCKET);
   SOCKET_STRUCT_PTR          socket_ptr;

   /* cycle through sockets looking for one owned by this task */
   for (socket_ptr = socket_cfg_ptr->SOCKET_HEAD;
        socket_ptr;
        socket_ptr = socket_ptr->NEXT) 
   {
      if (SOCK_Is_owner(socket_ptr, parms->owner)
         && SOCK_select_activity(socket_ptr, 0)) 
      {   
         break;
      } /* Endif */
   } /* Endfor */

   if (socket_ptr) 
   {
      parms->sock = (uint_32)socket_ptr;
      RTCSCMD_complete(parms, RTCS_OK);
      return;
   } /* Endif */

   if (parms->timeout == (uint_32)-1) 
   {
      parms->sock = 0;
      RTCSCMD_complete(parms, RTCS_OK);
      return;
   } /* Endif */

   SOCK_select_block(parms);
}

/* EOF */
