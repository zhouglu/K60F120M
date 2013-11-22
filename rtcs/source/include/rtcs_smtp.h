#ifndef __rtcs_smtp_h__
#define __rtcs_smtp_h__

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
* $FileName: rtcs_smtp.h$
* $Version : 4.0.1.0$
* $Date    : Jan-24-2013$
*
* Comments:
*
*   Simple Mail Transfer Protocol definitions.
*
*END************************************************************************/

#include <mqx.h>
#include <rtcs.h>

#define SMTP_OK 0
#define SMTP_ERR_BAD_PARAM 1
#define SMTP_ERR_CONN_FAILED 2
#define SMTP_WRONG_RESPONSE 3
#define SMTP_RESPONSE_BUFFER_SIZE 512
#define SMTP_COMMAND_BUFFER_SIZE 128

#define SET_ERR_STR(x,y,l) if(x != NULL) snprintf(x, l, "%s", y+4);

typedef struct smtp_email_envelope
{
    char_ptr    from;
    char_ptr    to;
}SMTP_EMAIL_ENVELOPE, _PTR_ SMTP_EMAIL_ENVELOPE_PTR;

typedef struct smtp_param_struct 
{
    SMTP_EMAIL_ENVELOPE envelope;
    char_ptr text;
    struct sockaddr server;
    char_ptr login;
    char_ptr pass;
}SMTP_PARAM_STRUCT, _PTR_ SMTP_PARAM_STRUCT_PTR;

_mqx_int SMTP_send_email (SMTP_PARAM_STRUCT_PTR param, char_ptr err_string, uint_32 err_string_size);

#endif
