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
* $FileName: httpsrv_prv.h$
*
* Comments:
*
*   Header for HTTPSRV.
*
*END************************************************************************/

#ifndef HTTPSRV_PRV_H_
#define HTTPSRV_PRV_H_

#include "message.h"

#define HTTPSRV_SERVER_STACK_SIZE     (850)
#define HTTPSRV_SESSION_STACK_SIZE    (1300)
#define HTTPSRV_PRODUCT_STRING        "MQX HTTPSRV/2.0 - Freescale Embedded Web Server v2.0"
#define HTTPSRV_PROTOCOL_STRING       "HTTP/1.1"
#define HTTPSRV_CGI_VERSION_STRING    "CGI/1.1"
#define HTTPSRV_SESSION_VALID         1
#define HTTPSRV_SESSION_INVALID       0
#define HTTPSRV_CGI_INVALID           (-738)
/*
**  Wildcard typedef for CGI/SSI callback prototype
*/
typedef _mqx_int(*HTTPSRV_FN_CALLBACK)(pointer param);

/*
**  Wildcard data type for CGI/SSI callback link structure
*/
typedef struct httpsrv_fn_link_struct
{
    char* fn_name;
    HTTPSRV_FN_CALLBACK callback;
} HTTPSRV_FN_LINK_STRUCT;

/*
** Types of callbacks
*/
typedef enum httpsrv_callback_type
{
    HTTPSRV_CGI_CALLBACK,
    HTTPSRV_SSI_CALLBACK
}HTTPSRV_CALLBACK_TYPE;

/*
* http session state machine status
*/
typedef enum httpsrv_ses_state
{
    HTTPSRV_SES_WAIT_REQ,
    HTTPSRV_SES_PROCESS_REQ,
    HTTPSRV_SES_END_REQ,
    HTTPSRV_SES_CLOSE,
    HTTPSRV_SES_RESP
} HTTPSRV_SES_STATE;

/*
** Parameter for searching callback in table by name 
*/
typedef struct httpsrv_call_param
{
    HTTPSRV_FN_LINK_STRUCT  *table;     /* Search table */
    pointer                 param;      /* Parameter for callback function*/
    char                    *fn_name;   /* Search name */
    HTTPSRV_CALLBACK_TYPE   type;       /* Type of callback (CGI/SSI) */
}HTTPSRV_CALL_PARAM;

/*
* http request parameters
*/
typedef struct httpsrv_req_struct
{
    HTTPSRV_REQ_METHOD       method;          /* Request method (GET, POST, HEAD) */
    int                      content_type;    /* Request entity content type */
    _mqx_int                 content_length;  /* Content length */
    boolean                  process_header;  /* Flag for indication of header processing */
    char                     *path;           /* Requested path */
    char                     *query;          /* Data send in URL */
    HTTPSRV_AUTH_USER_STRUCT auth;            /* Authentication credentials received from client */
} HTTPSRV_REQ_STRUCT;

/*
* http response parameters
*/
typedef struct httpsrv_res_struct
{
    MQX_FILE*                  file;          /* Handle to a file to send */
    _mqx_int                   status_code;   /* Status code - httpsrv_sendhdr transforms it to a text */
    _mqx_int                   len;           /* Response length */
    boolean                    cacheable;     /* Determines if response is cacheable */
    HTTPSRV_AUTH_REALM_STRUCT* auth_realm;    /* Authentication realm */
    int                        content_type;  /* Content type */
    char                       script_token;  /* Server side include found flag */
    char                       hdrsent;       /* Protection against multiple header sending */
} HTTPSRV_RES_STRUCT;

/*
* http session buffer
*/
typedef struct httpsrv_buffer
{
    uint_32 offset;          /* Write offset */
    char*   data;            /* Buffer data */
}HTTPSRV_BUFF_STRUCT;

/*
* http session structure
*/
typedef struct httpsrv_session_struct
{
    HTTPSRV_SES_STATE   state;              /* http session state */
    _mqx_int            valid;              /* non zero (1) = session is valid - data in this entry is valid */
    _mqx_uint           keep_alive;         /* Keep-alive status for the session */
    boolean             keep_alive_enabled; /* Keep-alive enabled/disabled for session */
    _mqx_int            sock;               /* Session socket */
    uint_32             start_time;         /* System time when session started. Used for timeout detection. */
    uint_32             timeout;            /* Session timeout in ms. timeout_time = start_time + timeout */
    HTTPSRV_BUFF_STRUCT buffer;             /* Session internal read/write buffer */
    HTTPSRV_REQ_STRUCT  request;            /* Data read from the request */
    HTTPSRV_RES_STRUCT  response;           /* Response data */
} HTTPSRV_SESSION_STRUCT;

typedef struct httpsrv_script_msg
{
    MESSAGE_HEADER_STRUCT   header;     /* Message header */
    HTTPSRV_SESSION_STRUCT* session;    /* Session requesting script */
    HTTPSRV_CALLBACK_TYPE   type;       /* Type of callback */
    char*                   name;       /* Function name */
    _task_id                ses_tid;    /* Session task id */
}HTTPSRV_SCRIPT_MSG;

/*
** http server main structure.
*/
typedef struct httpsrv_struct
{
    HTTPSRV_PARAM_STRUCT  params;       /* server parameters */
    /* 
    ** ------------------------------!!----------------------------------------
    ** Do not change order of following two variables (sock_v4 and sock_v6)
    ** sock_v6 MUST always follow sock_v4.
    ** ------------------------------!!----------------------------------------
    */
    _mqx_int               sock_v4;         /* listening socket for IPv4 */
    _mqx_int               sock_v6;         /* listening socket for IPv6 */
    _mqx_int               run;             /* run flag */
    HTTPSRV_SESSION_STRUCT **session;       /* array of pointers to sessions */
    _task_id               server_tid;      /* Server task ID */
    _task_id*              ses_tid;         /* Session task IDs */
    _task_id               script_tid;      /* CGI task ID */
    _pool_id               script_msg_pool; /* Message pool for CGI */
    _queue_id              script_msgq;     /* Message queue for CGI */
    LWSEM_STRUCT           tid_sem;         /* Semaphore for session TID array locking */
    LWSEM_STRUCT           ses_cnt;         /* Session counter */
} HTTPSRV_STRUCT;

/*
** Parameter for session task
*/
typedef struct
{
    HTTPSRV_STRUCT *server;  /* Pointer to server structure */
    uint_32 sock;            /* Socket to be used by session */
} HTTPSRV_SES_TASK_PARAM;

/*
** HTTP find function and call it.
*/
_mqx_int httpsrv_call_fn(HTTPSRV_CALL_PARAM* param);

void httpsrv_server_task(pointer init_ptr, pointer creator);
void httpsrv_script_task(pointer param, pointer creator);
static void httpsrv_process_cgi(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, char* cgi_name);
#endif /* HTTP_PRV_H_ */
