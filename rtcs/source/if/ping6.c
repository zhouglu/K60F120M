/**HEADER********************************************************************
* 
* Copyright (c) 2011 Freescale Semiconductor;
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
* $FileName: ping6.c$
* $Version : 3.8.1.0$
* $Date    : Jun-14-2012$
*
* Comments:
*
*   This file contains the implementation of a minimal
*   ICMPv6 echo request.
*
*END************************************************************************/

#include <rtcs.h>

#include "rtcs_prv.h"
#include "tcpip.h"
#include "icmp6_prv.h"
#include "arp.h"

boolean ICMP6_expire_echo (TCPIP_EVENT_PTR);

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : ICMP_expire_echo
* Returned Values :
* Comments  :
*     Called by the Timer.  Expire a ICMP echo request.
*
*END*-----------------------------------------------------------------*/
boolean ICMP6_expire_echo(TCPIP_EVENT_PTR   event)
{
   ICMP_ECHO_PARAM_PTR         parms = event->PRIVATE;

   RTCSCMD_complete(parms, RTCSERR_ICMP_ECHO_TIMEOUT);

   return FALSE;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : ICMP6_send_echo
* Parameters      :
*
*     _ip_address       ipaddress   [IN] destination IP address
*     uint_32           timeout     [IN/OUT] timeout/rtt
*     uint_16           id          [IN] identification for echo message.
*     uint_16           seq         not used
*
* Comments        :
*     Send an ICMP Echo Request packet.
*
*END*-----------------------------------------------------------------*/
void ICMP6_send_echo(ICMP_ECHO_PARAM_PTR     parms)
{

#if RTCSCFG_ENABLE_IP6

   ICMP6_CFG_STRUCT_PTR     ICMP6_cfg_ptr = RTCS_getcfg(ICMP6);
   RTCSPCB_PTR              pcb;
   ICMP6_ECHO_HEADER_PTR    packet;
   _rtcs_if_handle          ihandle_dest = NULL; 
   uint_32                  error;


   IF_ICMP6_STATS_ENABLED(ICMP6_cfg_ptr->STATS.COMMON.ST_TX_TOTAL++);
   parms->seq = ICMP6_cfg_ptr->ECHO_SEQ++;

   pcb = RTCSPCB_alloc_send();
   if (pcb == NULL)
   {
      IF_ICMP6_STATS_ENABLED(ICMP6_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
      RTCSCMD_complete(parms, RTCSERR_PCB_ALLOC);
      return;
   } 
        
    if(parms->ping_param->data_buffer && parms->ping_param->data_buffer_size)
    {
        error = RTCSPCB_append_fragment(pcb, parms->ping_param->data_buffer_size, parms->ping_param->data_buffer);
        if (error) 
        {
            IF_ICMP6_STATS_ENABLED(ICMP6_cfg_ptr->STATS.COMMON.ST_TX_ERRORS++);
            RTCSLOG_PCB_FREE(pcb, error);
            RTCSPCB_free(pcb);
            RTCSCMD_complete(parms, error);
            return;
        }
   }

   error = RTCSPCB_insert_header(pcb, sizeof(ICMP6_ECHO_HEADER));
   if (error)
   {
      IF_ICMP6_STATS_ENABLED(ICMP6_cfg_ptr->STATS.COMMON.ST_TX_ERRORS++);
      IF_ICMP6_STATS_ENABLED(RTCS_seterror(&ICMP6_cfg_ptr->STATS.ERR_TX, error, (uint_32)pcb));
      RTCSLOG_PCB_FREE(pcb, error);
      RTCSPCB_free(pcb);
      RTCSCMD_complete(parms, error);
      return;
   } 

   RTCSLOG_PCB_WRITE(pcb, RTCS_LOGCTRL_PROTO(IPPROTO_ICMPV6), 0);

   packet = (ICMP6_ECHO_HEADER_PTR)RTCSPCB_DATA(pcb);
   pcb->TRANSPORT_LAYER = (uchar_ptr)packet;
   htonc(packet->HEAD.TYPE, ICMP6_TYPE_ECHO_REQ);
   htonc(packet->HEAD.CODE, 0);
   htons(packet->HEAD.CHECKSUM, 0);
   htons(packet->ID,  parms->ping_param->id);
   htons(packet->SEQ, parms->seq);

   pcb->IP_SUM     = IP_Sum_PCB(RTCSPCB_SIZE(pcb), pcb);
   pcb->IP_SUM_PTR = packet->HEAD.CHECKSUM;
  
   if(((sockaddr_in6 *)(&parms->ping_param->addr))->sin6_scope_id)
   {
        ihandle_dest = ip6_if_get_by_scope_id(((sockaddr_in6 *)(&parms->ping_param->addr))->sin6_scope_id);
   }

   parms->EXPIRE.TIME    = parms->ping_param->timeout;
   parms->EXPIRE.EVENT   = ICMP6_expire_echo;
   parms->EXPIRE.PRIVATE = parms;
   parms->start_time = RTCS_time_get();   /* get timestamp */

   error = IP6_send(pcb, IPPROTO_ICMPV6|IPTTL(parms->ping_param->hop_limit), NULL/*ipsrc*/, &((sockaddr_in6 *)(&parms->ping_param->addr))->sin6_addr/*ipdest*/, ihandle_dest, 0);


   if (error != RTCS_OK)
   {
      IF_ICMP6_STATS_ENABLED(ICMP6_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
      RTCSCMD_complete(parms, error);
      return;
   }

   /*
   ** If there is no error add the parm struct to config struct
   ** and initiate the event timer
   */

   IF_ICMP6_STATS_ENABLED(ICMP6_cfg_ptr->STATS.ST_TX_ECHO_REQ++);

   /* add the prameter structure to ICMP  cfg */
   parms->NEXT = ICMP6_cfg_ptr->ECHO_PARAM_HEAD;
   if (parms->NEXT)
   {
      parms->NEXT->PREV = &parms->NEXT;
   } /* Endif */
   ICMP6_cfg_ptr->ECHO_PARAM_HEAD = parms;
   parms->PREV = &ICMP6_cfg_ptr->ECHO_PARAM_HEAD;

   TCPIP_Event_add(&parms->EXPIRE);

#else
    RTCSCMD_complete(parms, RTCSERR_IP_IS_DISABLED);
#endif /* RTCSCFG_ENABLE_IP6 */
}

