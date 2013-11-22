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
* $FileName: httpsrv.h$
*
* Comments:
*
*   Header for HTTPSRV.
*
*END************************************************************************/

#ifndef HTTPSRV_H_
#define HTTPSRV_H_

#include <rtcs.h>

/*
** HTTP server status codes
*/
#define HTTPSRV_OK 0
#define HTTPSRV_BIND_FAIL 1 
#define HTTPSRV_LISTEN_FAIL 2
#define HTTPSRV_ERR 3
#define HTTPSRV_CREATE_FAIL 4
#define HTTPSRV_BAD_FAMILY 5
#define HTTPSRV_SOCKOPT_FAIL 6

/*
** Authentication types
*/
typedef enum httpstv_auth_type
{
    HTTPSRV_AUTH_INVALID,
    HTTPSRV_AUTH_BASIC,
    HTTPSRV_AUTH_DIGEST /* Not supported yet! */
} HTTPSRV_AUTH_TYPE;

/*
* http request method type
*/
typedef enum httpsrv_req_method
{
    HTTPSRV_REQ_UNKNOWN,
    HTTPSRV_REQ_GET,
    HTTPSRV_REQ_POST,
    HTTPSRV_REQ_HEAD
} HTTPSRV_REQ_METHOD;

/*
* http content type
*/
typedef enum httpsrv_content_type
{
    HTTPSRV_CONTENT_TYPE_OCTETSTREAM = 1,
    HTTPSRV_CONTENT_TYPE_PLAIN,
    HTTPSRV_CONTENT_TYPE_HTML,
    HTTPSRV_CONTENT_TYPE_CSS,
    HTTPSRV_CONTENT_TYPE_GIF,
    HTTPSRV_CONTENT_TYPE_JPG,
    HTTPSRV_CONTENT_TYPE_PNG,
    HTTPSRV_CONTENT_TYPE_JS,
    HTTPSRV_CONTENT_TYPE_ZIP,
    HTTPSRV_CONTENT_TYPE_PDF,
} HTTPSRV_CONTENT_TYPE;

/*
** Authentication user structure
*/
typedef struct httpsrv_auth_user_struct
{
    char* user_id;   /* User ID - usually name*/
    char* password;  /* Password */
}HTTPSRV_AUTH_USER_STRUCT;

/*
** Authentication realm structure
*/
typedef struct httpsrv_auth_realm_struct
{
    char*                     name;       /* Name of realm. Send to client so user know which login/pass should be used. */
    char*                     path;       /* Path to file/directory to protect. Relative to root directory */
    HTTPSRV_AUTH_TYPE         auth_type;  /* Authentication type to use. */
    HTTPSRV_AUTH_USER_STRUCT* users;      /* Table of allowed users. */
} HTTPSRV_AUTH_REALM_STRUCT;

/*
** CGI request structure. Contains variables specified in RFC3875 (The Common Gateway Interface (CGI) Version 1.1).
** Structure is extended by session handle.
*/
typedef struct httpsrv_cgi_request_struct
{
    uint_32              ses_handle;         /* Session handle required for various read/write operations*/
    /* 
    *  Following is subset of variables from RFC3875. 
    ** Please see http://tools.ietf.org/html/rfc3875#section-4.1 for details
    */
    HTTPSRV_REQ_METHOD   request_method;     /* Request method (GET, POST, HEAD) see HTTPSRV_REQ_METHOD enum */
    HTTPSRV_CONTENT_TYPE content_type;       /* Content type */
    uint_32              content_length;     /* Content length */
    uint_32              server_port;        /* Local connection port */
    char*                remote_addr;        /* Remote client address */
    char*                server_name;        /* Server hostname/IP */
    char*                script_name;        /* CGI name */
    char*                server_protocol;    /* Server protocol name and version (HTTP/1.0) */
    char*                server_software;    /* Server software identification string */
    char*                query_string;       /* Request query string */
    char*                gateway_interface;  /* Gateway interface type and version (CGI/1.1)*/
    char*                remote_user;        /* Remote user name  */
    HTTPSRV_AUTH_TYPE    auth_type;          /* Auth type */
}HTTPSRV_CGI_REQ_STRUCT;

/*
** CGI response struct. This structure is filled by CGI function.
*/
typedef struct httpsrv_cgi_response_struct
{
    uint_32              ses_handle;              /* Session handle for reading/writing */
    HTTPSRV_CONTENT_TYPE content_type;            /* Response content type */
    uint_32              content_length;          /* Response content length */
    uint_32              status_code;             /* Status code (200, 404, etc.)*/
    char*                data;                    /* Pointer to data to write */
    uint_32              data_length;             /* Length of data in bytes */
}HTTPSRV_CGI_RES_STRUCT;

/*
** Directory aliases
*/
typedef struct httpsrv_alias
{
    char* alias;
    char* path;
}HTTPSRV_ALIAS;

/*
** Server side include parameter structure.
** Passed to user SSI function.
*/
typedef struct httpsrv_ssi_param_struct
{
    uint_32 ses_handle;                 /* Session handle for reading/writing */
    char*   com_param;                  /* Server side include command parameter (separated from command by ":") */
}HTTPSRV_SSI_PARAM_STRUCT;

/*
** Server side include callback prototype
*/
typedef _mqx_int(*HTTPSRV_SSI_CALLBACK_FN)(HTTPSRV_SSI_PARAM_STRUCT* param);

/*
** SSI callback link structure
*/
typedef struct httpsrv_ssi_link_struct
{
    char* fn_name;                              /* Function name */
    HTTPSRV_SSI_CALLBACK_FN callback;           /* Pointer to user function */
} HTTPSRV_SSI_LINK_STRUCT;

/*
** CGI callback prototype
*/
typedef _mqx_int(*HTTPSRV_CGI_CALLBACK_FN)(HTTPSRV_CGI_REQ_STRUCT* param);

/*
** CGI callback link structure
*/
typedef struct httpsrv_cgi_link_struct
{
    char* fn_name;                              /* Function name */
    HTTPSRV_CGI_CALLBACK_FN callback;           /* Pointer to user function */
} HTTPSRV_CGI_LINK_STRUCT;

/*
** HTTP server parameters
*/
typedef struct httpsrv_param_struct
{
    unsigned short             port;           /* Listening port */
  #if RTCSCFG_ENABLE_IP4
    in_addr                    ipv4_address;   /* Listening IPv4 address */
  #endif
  #if RTCSCFG_ENABLE_IP6    
    in6_addr                   ipv6_address;   /* Listening IPv6 address */
    uint_32                    ipv6_scope_id;  /* Scope ID for IPv6 */
  #endif
    _mqx_uint                  max_uri;        /* maximal URI length */
    _mqx_uint                  max_ses;        /* maximal sessions count */
    boolean                    use_nagle;      /* enable/disable nagle algorithm for server sockets */
    HTTPSRV_CGI_LINK_STRUCT*   cgi_lnk_tbl;    /* cgi callback table */
    HTTPSRV_SSI_LINK_STRUCT*   ssi_lnk_tbl;    /* function callback table (dynamic web pages) */
    HTTPSRV_ALIAS*             alias_tbl;      /* table od directory aliases */
    uint_32                    server_prio;    /* server main task priority */
    uint_32                    script_prio;    /* script handler priority */
    uint_32                    script_stack;   /* script handler stack */
    char*                      root_dir;       /* root directory */
    char*                      index_page;     /* index page full path and name */
    HTTPSRV_AUTH_REALM_STRUCT* auth_table;     /* Table of authentication realms */
    uint_16                    af;             /* Inet protocol family */
} HTTPSRV_PARAM_STRUCT;

/*
** Initialize and run HTTP server
** Returns server handle when successful, zero otherwise.
*/
uint_32 HTTPSRV_init(HTTPSRV_PARAM_STRUCT *params);

/*
** Stop and release HTTP server
** Returns RTCS_OK when successful, RTCS_ERR otherwise.
*/
uint_32 HTTPSRV_release(uint_32 server_h);

uint_32 HTTPSRV_cgi_write(HTTPSRV_CGI_RES_STRUCT* response);
uint_32 HTTPSRV_cgi_read(uint_32 ses_handle, char* buffer, uint_32 length);
uint_32 HTTPSRV_ssi_write(uint_32 ses_handle, char* data, uint_32 length);
#endif /* HTTP_H_ */
