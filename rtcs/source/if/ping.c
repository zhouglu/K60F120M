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
* $FileName: ping.c$
* $Version : 3.8.11.0$
* $Date    : Jun-14-2012$
*
* Comments:
*
*   This file contains the implementation of a minimal
*   ICMP echo request.
*
*END************************************************************************/

#include <rtcs.h>

#if RTCSCFG_ENABLE_ICMP 
#include "rtcs_prv.h"
#include "tcpip.h"
#include "icmp_prv.h"
#include "arp.h"


boolean ICMP_expire_echo (TCPIP_EVENT_PTR);


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_ping
* Returned Value  : error code
* Comments  :  Send an ICMP echo request, and wait for a reply.
*
*END*-----------------------------------------------------------------*/
uint_32 RTCS_ping  (PING_PARAM_STRUCT_PTR params)
{
    uint_32 error = RTCS_ERROR;

    if(params)
    {
        ICMP_ECHO_PARAM     icmp_echo_params;
        void                (*cmd)(ICMP_ECHO_PARAM_PTR);

        icmp_echo_params.ping_param = params;

    #if RTCSCFG_ENABLE_IP4
        if(params->addr.sa_family == AF_INET)
        {
            cmd = ICMP_send_echo;
        }
        else 
    #endif
    #if RTCSCFG_ENABLE_IP6
        if(params->addr.sa_family == AF_INET6)
        {
            cmd = ICMP6_send_echo;
        }
        else
    #endif
        {
            cmd = NULL;
        }
        
        if(cmd)
            error = RTCSCMD_issue(icmp_echo_params, cmd);
    }

    return error;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : ICMP_expire_echo
* Returned Values :
* Comments  :
*     Called by the Timer.  Expire a ICMPv4 echo request.
*
*END*-----------------------------------------------------------------*/
#if RTCSCFG_ENABLE_IP4
boolean ICMP_expire_echo( TCPIP_EVENT_PTR   event )
{
   ICMP_ECHO_PARAM_PTR   parms = event->PRIVATE;
   _ip_address           src_addr, dest_addr;
   _ip_address           kill_addr = ((sockaddr_in *)(&parms->ping_param->addr))->sin_addr.s_addr;
   IP_IF_PTR             if_ptr;
   RTCSPCB_PTR           waiting_ptr, tmp_ptr;
   IP_HEADER_PTR         iphead;
   uint_16               protocol, test_id, test_seq;
   ICMP_ECHO_HEADER_PTR  icmphead;
   uchar                 type, hdrlen;
  
   /* kill the (possible) arp cache entry if it matches our ping request */
   src_addr = IP_route_find(kill_addr, 1);
   if_ptr = IP_find_if (src_addr);
    
   if (if_ptr && if_ptr->ARP) {
      waiting_ptr = (RTCSPCB_PTR)ARP_find_waiting(if_ptr, kill_addr);
      while (waiting_ptr) {
         tmp_ptr = waiting_ptr;
         if (src_addr == IP_source(waiting_ptr)) {
            dest_addr   = IP_dest(waiting_ptr);
            iphead = (IP_HEADER_PTR)RTCSPCB_DATA_NETWORK(waiting_ptr);
            hdrlen = (ntohc(iphead->VERSLEN) & 0x0F) << 2;
            protocol = RTCSPCB_TRANSPORT_PROTL(waiting_ptr);
            if ( protocol == IPPROTO_ICMP ) {
               icmphead = (ICMP_ECHO_HEADER_PTR)RTCSPCB_DATA_TRANSPORT(waiting_ptr);
               type = icmphead->HEAD.TYPE[0];
               if ( ICMPTYPE_ISQUERY(type)) {
                  test_id  = ntohs(icmphead->ID);  
                  test_seq = ntohs(icmphead->SEQ);
                  /* found it */
                  if (test_id  == parms->ping_param->id  &&
                     test_seq == parms->seq &&
                     kill_addr == dest_addr) {
                     /* kill it */
                     ARP_kill_entry (if_ptr, kill_addr);
                  } /* Endif */
               } /* Endif */
            } /* Endif */
         } /* Endif */
         waiting_ptr = (RTCSPCB_PTR)ARP_find_waiting(if_ptr, kill_addr);
         if (waiting_ptr == tmp_ptr) 
             waiting_ptr = NULL;
      } /* While */
   } /* Endif */

   /* inform the application of timeout */
   if (parms->NEXT) {
      parms->NEXT->PREV = parms->PREV;
   } /* Endif */
   *parms->PREV = parms->NEXT;

   RTCSCMD_complete(parms, RTCSERR_ICMP_ECHO_TIMEOUT);

   return FALSE;
}
#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : ICMP_send_echo
* Parameters      :
*
*     _ip_address       ipaddress   [IN] destination IP address
*     uint_32           timeout     [IN/OUT] timeout/rtt
*     uint_16           id          [IN] identification for echo message.
*     uint_16           seq         not used
*
* Comments        :
*     Send an ICMPv4 Echo Request packet.
*
*END*-----------------------------------------------------------------*/
void ICMP_send_echo(ICMP_ECHO_PARAM_PTR     parms)
{ 

#if RTCSCFG_ENABLE_IP4

   ICMP_CFG_STRUCT_PTR     ICMP_cfg_ptr = RTCS_getcfg(ICMP);
   RTCSPCB_PTR             pcb;
   ICMP_ECHO_HEADER_PTR    packet;
   uint_16                 chksum;
   uint_32                 error;

   IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_TX_TOTAL++);
   parms->seq = ICMP_cfg_ptr->ECHO_SEQ++;

   pcb = RTCSPCB_alloc_send();
   if (pcb == NULL) {
      IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
      RTCSCMD_complete(parms, RTCSERR_PCB_ALLOC);
      return;
   }

    if(parms->ping_param->data_buffer && parms->ping_param->data_buffer_size)
    {
        error = RTCSPCB_append_fragment(pcb, parms->ping_param->data_buffer_size, parms->ping_param->data_buffer);
        if (error) 
        {
            IF_ICMP6_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_TX_ERRORS++);
            RTCSLOG_PCB_FREE(pcb, error);
            RTCSPCB_free(pcb);
            RTCSCMD_complete(parms, error);
            return;
        }
   }

   //RTCSLOG_PCB_ALLOC(pcb);

   error = RTCSPCB_insert_header(pcb, sizeof(ICMP_ECHO_HEADER));
   if (error) {
      IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_TX_ERRORS++);
      IF_ICMP_STATS_ENABLED(RTCS_seterror(&ICMP_cfg_ptr->STATS.ERR_TX, error, (uint_32)pcb));
      RTCSLOG_PCB_FREE(pcb, error);
      RTCSPCB_free(pcb);
      RTCSCMD_complete(parms, error);
      return;
   } /* Endif */

   RTCSLOG_PCB_WRITE(pcb, RTCS_LOGCTRL_PROTO(IPPROTO_ICMP), 0);

   packet = (ICMP_ECHO_HEADER_PTR)RTCSPCB_DATA(pcb);
   pcb->TRANSPORT_LAYER = (uchar_ptr)packet;
   htonc(packet->HEAD.TYPE, ICMPTYPE_ECHO_REQ);
   htonc(packet->HEAD.CODE, 0);
   htons(packet->HEAD.CHECKSUM, 0);
   htons(packet->ID,  parms->ping_param->id);
   htons(packet->SEQ, parms->seq);

   chksum = IP_Sum_PCB(0, pcb);
   chksum = IP_Sum_invert(chksum);
   htons(packet->HEAD.CHECKSUM, chksum);
   pcb->IP_SUM_PTR = NULL;

   parms->EXPIRE.TIME    = parms->ping_param->timeout;
   parms->EXPIRE.EVENT   = ICMP_expire_echo;
   parms->EXPIRE.PRIVATE = parms;
   parms->start_time = RTCS_time_get();   /* get timestamp */

   error = IP_send(pcb, IPPROTO_ICMP|IPTTL(parms->ping_param->hop_limit), INADDR_ANY, ((sockaddr_in *)(&parms->ping_param->addr))->sin_addr.s_addr, 0);

   if (error != RTCS_OK) {
      IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
      RTCSCMD_complete(parms, error);
      return;
   } /* Endif */

   /*
   ** If there is no error add the parm struct to config struct
   ** and initiate the event timer
   */

   IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.ST_TX_ECHO_REQ++);

   /* add the prameter structure to ICMP  cfg */
   parms->NEXT = ICMP_cfg_ptr->ECHO_PARAM_HEAD;
   if (parms->NEXT) {
      parms->NEXT->PREV = &parms->NEXT;
   } /* Endif */
   ICMP_cfg_ptr->ECHO_PARAM_HEAD = parms;
   parms->PREV = &ICMP_cfg_ptr->ECHO_PARAM_HEAD;

   TCPIP_Event_add(&parms->EXPIRE);
   
#else

    RTCSCMD_complete(parms, RTCSERR_IP_IS_DISABLED);
    return;

#endif /* RTCSCFG_ENABLE_IP4 */   

} 

#endif

/* EOF */
