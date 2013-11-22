/**HEADER********************************************************************
* 
* Copyright (c) 2013 Freescale Semiconductor;
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
* $FileName: httpsrv_supp.h$
*
* Comments:
*
*   HTTPSRV support functions header.
*
*END************************************************************************/

#ifndef HTTP_SUPP_H_
#define HTTP_SUPP_H_

#define ERR_PAGE_FORMAT "<HTML><HEAD><TITLE>%s</TITLE></HEAD>\n<BODY><H1>%s</H1>\n</BODY></HTML>\n"
#include "httpsrv_prv.h"
#include "httpsrv.h"

_mqx_int httpsrv_read(HTTPSRV_SESSION_STRUCT *session, char *dst, _mqx_int len);
uint_32 httpsrv_send_buffer(HTTPSRV_SESSION_STRUCT *session);

void httpsrv_sendhdr(HTTPSRV_SESSION_STRUCT *session, _mqx_int content_len, boolean has_entity);
void httpsrv_sendfile(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session);
void httpsrv_send_err_page(HTTPSRV_SESSION_STRUCT *session, const char* title, const char* text);
void httpsrv_process_req_hdr_line(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT* session, char* buffer);
void httpsrv_process_req_method(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, char* buffer);

#endif /* HTTP_SUPP_H_ */
