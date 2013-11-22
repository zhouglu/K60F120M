/**HEADER********************************************************************
* 
* Copyright (c) 2008 Freescale Semiconductor;
* All Rights Reserved
*
* Copyright (c) 2004-2008 Embedded Access Inc.;
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
* $Version : 3.8.9.0$
* $Date    : Jun-14-2012$
*

* Comments:
*
*   Configurable information for the RTCS examples.
*
*END************************************************************************/


/*
** Define IP address and IP network mask
*/
#ifndef ENET_IPADDR
   #define ENET_IPADDR  IPADDR(192,168,1,202) 
#endif

#ifndef ENET_IPMASK
   #define ENET_IPMASK  IPADDR(255,255,255,0) 
#endif

#ifndef ENET_IPGATEWAY
   #define ENET_IPGATEWAY  IPADDR(0,0,0,0) 
#endif

#ifndef ENET_MAC
   #define ENET_MAC  {0x00,0xA7,0xC5,0xF1,0x11,0x90}
#endif

#ifndef DEMO_PORT
    #define DEMO_PORT IPPORT_TELNET
#endif

#define SERIAL_DEVICE BSP_DEFAULT_IO_CHANNEL
/* EOF */
