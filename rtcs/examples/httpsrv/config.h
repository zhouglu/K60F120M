/**HEADER********************************************************************
* 
* Copyright (c) 2008 Freescale Semiconductor;
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
* $FileName: config.h$
* $Version : 3.8.2.0$
* $Date    : Sep-25-2012$
*
* Comments:
*
*   Configurable information for the RTCS examples.
*
*END************************************************************************/
#ifndef _config_h_
#define _config_h_

/* IP address macros */
#define A 192
#define B 168
#define C 1
#define D 202

/* Use this define to tell example if only one server should be used for all interfaces */
#define HTTP_USE_ONE_SERVER 1

#ifndef ENET_IPMASK
    #define ENET_IPMASK  IPADDR(255,255,255,0) 
#endif

#if RTCSCFG_ENABLE_IP6   
    #if RTCSCFG_ENABLE_IP4   
        /* 
        **  HTTP_INET_AF is AF_INET+AF_INET for http support IPv4+IPv6.
        **  HTTP_INET_AF is AF_INET  for http support IPv4 only.
        **  HTTP_INET_AF is AF_INET6 for http support IPv6 only.
        */
        #define HTTP_INET_AF    (AF_INET | AF_INET6)
    #else
        #define HTTP_INET_AF    AF_INET6        
    #endif
#else
        #define HTTP_INET_AF    AF_INET         
#endif
    
    //#define HTTP_INET_AF           AF_INET
    #define HTTP_SCOPE_ID          0 /* For any IF. */


#endif // _config_h_
