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
* $FileName: httpsrv_supp.c$
*
* Comments:
*
*   HTTPSRV support functions.
*
*END************************************************************************/

#include "httpsrv.h"
#include "httpsrv_prv.h"
#include "httpsrv_supp.h"
#include <string.h>
#include <fio.h>
#include <posix.h>
#include <stdlib.h>
#include <stdarg.h>
#include <rtcs_base64.h>
#include <ctype.h>

/*
* string table item
*/
typedef struct
{
    int  id;
    char *str;
}tbl_str_itm_t;

/* Structure defining MIME types table row */
typedef struct
{
    uint_32 length;         /* extension string length (strlen) */
    char*   ext;            /* extension string */
    int     content_type;   /* extension MIME type string */
    boolean use_cache;      /* cache use */
}httpsrv_content_table_row;

/*
* content type
*/
static const tbl_str_itm_t content_type[] = {
        { HTTPSRV_CONTENT_TYPE_PLAIN,       "text/plain" },
        { HTTPSRV_CONTENT_TYPE_HTML,        "text/html" },
        { HTTPSRV_CONTENT_TYPE_CSS,         "text/css" },
        { HTTPSRV_CONTENT_TYPE_GIF,         "image/gif" },
        { HTTPSRV_CONTENT_TYPE_JPG,         "image/jpeg" },
        { HTTPSRV_CONTENT_TYPE_PNG,         "image/png" },
        { HTTPSRV_CONTENT_TYPE_JS,          "application/javascript" },
        { HTTPSRV_CONTENT_TYPE_ZIP,         "application/zip" },
        { HTTPSRV_CONTENT_TYPE_PDF,         "application/pdf" },
        { HTTPSRV_CONTENT_TYPE_OCTETSTREAM, "application/octet-stream" },
        { 0,    0 }
};

/*
* Response status to reason conversion table
*/
static const tbl_str_itm_t reason_phrase[] = {
        { 200, "OK" },
        { 400, "Bad Request"},
        { 401, "Unauthorized" },
        { 403, "Forbidden" },
        { 404, "Not Found" },
        { 411, "Length Required"},
        { 414, "Request-URI Too Long"},
        { 500, "Internal Server Error"},
        { 501, "Not Implemented" },
        { 0,   "" }
};

/*
** Extension -> content type conversion table.
** This table rows MUST be ordered by size and alphabetically
** so we can list througth it quicly
*/
static httpsrv_content_table_row content_tbl[] = {
    /* Size,          extension, MIME type,                        Cache? */
    {sizeof("js")-1 ,   "js",    HTTPSRV_CONTENT_TYPE_JS,          TRUE},
    {sizeof("css")-1,   "css",   HTTPSRV_CONTENT_TYPE_CSS,         TRUE},
    {sizeof("gif")-1,   "gif",   HTTPSRV_CONTENT_TYPE_GIF,         TRUE}, 
    {sizeof("htm")-1,   "htm",   HTTPSRV_CONTENT_TYPE_HTML,        TRUE},
    {sizeof("jpg")-1,   "jpg",   HTTPSRV_CONTENT_TYPE_JPG,         TRUE},
    {sizeof("pdf")-1,   "pdf",   HTTPSRV_CONTENT_TYPE_PDF,         FALSE}, 
    {sizeof("png")-1,   "png",   HTTPSRV_CONTENT_TYPE_PNG,         TRUE},
    {sizeof("txt")-1,   "txt",   HTTPSRV_CONTENT_TYPE_PLAIN,       FALSE},
    {sizeof("zip")-1,   "zip",   HTTPSRV_CONTENT_TYPE_ZIP,         FALSE},
    {sizeof("html")-1,  "html",  HTTPSRV_CONTENT_TYPE_HTML,        TRUE},
    {sizeof("shtm")-1,  "shtm",  HTTPSRV_CONTENT_TYPE_HTML,        FALSE},
    {sizeof("shtml")-1, "shtml", HTTPSRV_CONTENT_TYPE_HTML,        FALSE},
    /* Following row MUST have length set to zero so we have proper array termination */
    {0,                      "", HTTPSRV_CONTENT_TYPE_OCTETSTREAM, FALSE}
};

static _mqx_int httpsrv_sendextstr(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, char *str, uint_32 length);
static _mqx_int httpsrv_write(HTTPSRV_SESSION_STRUCT *session, char *src, _mqx_int len);

static void httpsrv_urldecode(char* url);
static void httpsrv_urlcleanup(char* url);
static void httpsrv_print_to_buffer(HTTPSRV_SESSION_STRUCT *session, char* format, ...);
static char* get_tbl_string(tbl_str_itm_t *tbl, const _mqx_int id);
static int get_tbl_id(tbl_str_itm_t *tbl, char* str, uint_32 len);

static void httpsrv_process_file_type(char* extension, HTTPSRV_SESSION_STRUCT* session);
/*
** Call function by name. Used for dynamic web pages.
**
** IN:
**      HTTPSRV_CALL_PARAM* param - parameters structure:
**          HTTPSRV_FN_LINK_STRUCT *table   - table containing function names and poiters to callbacks
**          pointer                param    - parameter for callback
**          char                   *fn_name - name of called function
**          HTTPSRV_CALLBACK_TYPE  type     - callback type (CGI/SSI)
**
** OUT:
**      none
**
** Return Value:
**      none
*/
_mqx_int httpsrv_call_fn(HTTPSRV_CALL_PARAM* param)
{
    _mqx_int retval = HTTPSRV_CGI_INVALID;

    if ((param->table == NULL) || (param->fn_name == NULL))
    {
        return(retval);
    }
    
    while (*(param->table->callback))
    {
        if (0 == strcmp(param->fn_name, param->table->fn_name))
        {
            if (param->type == HTTPSRV_CGI_CALLBACK)
            {
                retval = param->table->callback((HTTPSRV_CGI_REQ_STRUCT*) param->param);
                break;
            }
            else if (param->type == HTTPSRV_SSI_CALLBACK)
            {
                retval = param->table->callback((HTTPSRV_SSI_PARAM_STRUCT*) param->param);
                break;
            }
        }
        param->table++;
    }
    return(retval);
}

/*
** Send extended string to socket (dynamic webpages).
**
** IN:
**      HTTPSRV_STRUCT         *server - server structure.
**      HTTPSRV_SESSION_STRUCT *session - session for sending.
**      char                   *str - string to send.
**      uint_32                length - length of source string.
**
** OUT:
**      none
**
** Return Value:
**      int - number of bytes processed.
*/
static _mqx_int httpsrv_sendextstr(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, char *str, uint_32 length)
{
    char *src;
    int len, res;
    int add = 0;
    char fname[HTTPSRVCFG_MAX_SCRIPT_LN + 1];
    uint_32 old_data = session->buffer.offset;
        
    src = str;
    fname[0] = 0;
    
    if (session->response.script_token)
    {
        // script token found
        len = (int)strcspn(src, " ;%<>\r\n\t");

        if (len > 1 && len < HTTPSRVCFG_MAX_SCRIPT_LN)
        {
            HTTPSRV_SCRIPT_MSG* msg_ptr;

            snprintf(fname, len+1, "%s", src);
            /* Form up message for handler task and send it */
            msg_ptr = _msg_alloc(server->script_msg_pool);
            if (msg_ptr != NULL)
            {
                msg_ptr->header.TARGET_QID = server->script_msgq;
                msg_ptr->header.SOURCE_QID = server->script_msgq;
                msg_ptr->header.SIZE = sizeof(HTTPSRV_SCRIPT_MSG);
                msg_ptr->session = session;
                msg_ptr->type = HTTPSRV_SSI_CALLBACK;
                msg_ptr->name = fname;
                msg_ptr->ses_tid = _task_get_id();
                _msgq_send(msg_ptr);
                /* wait until SSI is processed */
                _task_block();
            }
        }

        if (src[len] == '%' && src[len + 1] == '>')
        {
            session->response.script_token = 0;
            len += 1;
        }
        len++;
    }
    else
    {
        for (len = 0; *src && len < length; src++, len++)
        {
            if (src[0] == '<' && src[1] == '%')
            {
                session->response.script_token = 1;
                src += 2;
                add = 2;
                break;
            }
        }
        res = send(session->sock, str, len, 0);
        session->buffer.offset = 0;
        _mem_zero(session->buffer.data, (HTTPSRVCFG_SES_BUFFER_SIZE & PSP_MEMORY_ALIGNMENT_MASK));
        if (res < 0)
        {
            session->response.script_token = 0;
            res = errno;

            if (res != EAGAIN)
            {
                len = 0;
            }  
        }
        else if (len != res)
        {
            session->response.script_token = 0;
        }
    }
    return ((len+add)-old_data);
}

/*
** Read data from HTTP server.
**
** First copy data from session buffer if there are any and than read rest from socket if required.
**
** IN:
**      HTTPSRV_SESSION_STRUCT *session - session to use for reading.
**      char                   *dst - user buffer to read to.
**      _mqx_int               len - size of user buffer.
**
** OUT:
**      none
**
** Return Value:
**      int - number of bytes read.
*/
_mqx_int httpsrv_read(HTTPSRV_SESSION_STRUCT *session, char *dst, _mqx_int len)
{
    int read = 0;
    uint_32 data_size = session->buffer.offset;
    uint_32 length = (data_size < len) ? data_size : len;
    uint_32 received = 0;

    /* If there are any data in buffer copy them to user buffer */
    if (data_size > 0)
    {
        uint_32 tail = (HTTPSRVCFG_SES_BUFFER_SIZE & PSP_MEMORY_ALIGNMENT_MASK)-length;
        _mem_copy(session->buffer.data, dst, length);
        memmove(session->buffer.data, session->buffer.data+length, tail);
        _mem_zero(session->buffer.data+tail, length);
        session->buffer.offset -= length;
        read = length;
    }

    /* If there is some space remaining in user buffer try to read from socket */
    while (read < len)
    {
        received = recv(session->sock, dst+read, len-read, 0);
        
        if (RTCS_ERROR != received)
        {
            read += received;
        }
        else
        {
            break;
        }
    }
    
    return read;
}

/*
** Write data to buffer. If buffer is full during write flush it to client.
**
** IN:
**      HTTPSRV_SESSION_STRUCT *session - session used for write.
**      char*                   src     - pointer to data to send.
**      _mqx_int                len     - length of data in bytes.
**
** OUT:
**      none
**
** Return Value:
**      _mqx_int - number of bytes written.
*/
static _mqx_int httpsrv_write(HTTPSRV_SESSION_STRUCT *session, char *src, _mqx_int len)
{
    uint_32 space = (HTTPSRVCFG_SES_BUFFER_SIZE & PSP_MEMORY_ALIGNMENT_MASK) - session->buffer.offset;
    uint_32 retval = len;

    /* User buffer is bigger than session buffer - send user data directly */
    if (len > (HTTPSRVCFG_SES_BUFFER_SIZE & PSP_MEMORY_ALIGNMENT_MASK))
    {
        /* If there are some data already buffered send them to client first */
        if (session->buffer.offset != 0)
        {
            httpsrv_send_buffer(session);
        }
        return(send(session->sock, src, len, 0));
    }

    /* No space in buffer - make some */
    if ((space == 0) || (space < len))
    {
        httpsrv_send_buffer(session);
        space = (HTTPSRVCFG_SES_BUFFER_SIZE & PSP_MEMORY_ALIGNMENT_MASK);
    }

    /* Now we can save user data to buffer and eventually send them to client */
    _mem_copy(src, session->buffer.data+session->buffer.offset, len);
    session->buffer.offset += len;
    
    if (session->buffer.offset >= (HTTPSRVCFG_SES_BUFFER_SIZE & PSP_MEMORY_ALIGNMENT_MASK))
    {
        httpsrv_send_buffer(session);
    }

    return(retval);
}

/*
** Send data from session buffer to client.
**
** IN:
**      HTTPSRV_SESSION_STRUCT *session - session to use.
**
** OUT:
**      none
**
** Return Value:
**      int - number of bytes send.
*/
uint_32 httpsrv_send_buffer(HTTPSRV_SESSION_STRUCT *session)
{
    uint_32 length = 0;

    length = send(session->sock, session->buffer.data, session->buffer.offset, 0);
    
    if (length != RTCS_ERROR)
    {
        _mem_zero(session->buffer.data, session->buffer.offset);
        session->buffer.offset = 0;
    }
    else
    {
        length = 0;
    }
    return(length);
}

/*
** Get string for ID from table.
**
** IN:
**      tbl_str_itm_t     *tbl - table to be searched
**      _mqx_int          id - search ID
**
** OUT:
**      none
**
** Return Value:
**      char* - pointer to result. NULL if not found.
*/
static char* get_tbl_string(tbl_str_itm_t *tbl, const _mqx_int id)
{
    tbl_str_itm_t *ptr = tbl;

    while ((ptr->str) && (id != ptr->id))
    {
        ptr++;
    }
    return ptr->str;
}

/*
** Get ID for string from table
**
** IN:
**      tbl_str_itm_t     *tbl - table to be searched
**      char*             str - search string
**      uint_32           len - length of string
**
** OUT:
**      none
**
** Return Value:
**      ID corresponding to searched string. Zero if not found.
*/
static int get_tbl_id(tbl_str_itm_t *tbl, char* str, uint_32 len)
{
    tbl_str_itm_t *ptr = tbl;

    while ((ptr->id) && (strncmp(str, ptr->str, len)))
    {
        ptr++;
    }
    return (ptr->id);
}

/*
** Send http header according to the session response structure.
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session used for transmission
**      _mqx_int                content_len - content length
**      boolean                 has_entity - flag indicating if HTTP entity is going to be send following header.
**
** OUT:
**      none
**
** Return Value:
**      none
*/
void httpsrv_sendhdr(HTTPSRV_SESSION_STRUCT *session, _mqx_int content_len, boolean has_entity)
{ 
    if (session->response.hdrsent)
    {
         return;
    }
    
    httpsrv_print_to_buffer(session, "%s %d %s\r\n", HTTPSRV_PROTOCOL_STRING, session->response.status_code,
            get_tbl_string((tbl_str_itm_t*)reason_phrase, session->response.status_code));
    
    httpsrv_print_to_buffer(session, "Server: %s\r\n", HTTPSRV_PRODUCT_STRING);
    
    /* Check authorization */
    if (session->response.status_code == 401)
    {
        httpsrv_print_to_buffer(session, "WWW-Authenticate: Basic realm=\"%s\"\r\n", session->response.auth_realm->name);
    }
    httpsrv_print_to_buffer(session, "Connection: %s\r\n", session->keep_alive ? "Keep-Alive":"close");
    
    /* If there will be entity body send content type */
    if (has_entity)
    {
        httpsrv_print_to_buffer(session, "Content-Type: %s\r\n", get_tbl_string((tbl_str_itm_t*)content_type, session->response.content_type));
    }

    httpsrv_print_to_buffer(session, "Cache-Control: ");
    if (session->response.cacheable)
    {
        httpsrv_print_to_buffer(session, "max-age=%d\r\n", HTTPSRVCFG_CACHE_MAXAGE);
    }
    else
    {
        if (session->response.auth_realm != NULL)
        {
            httpsrv_print_to_buffer(session, "no-store\r\n");
        }
        else
        {
            httpsrv_print_to_buffer(session, "no-cache\r\n");
        }
    }

    /* Only non zero length cause sending Content-Length header field */
    if (content_len > 0)
    {
        httpsrv_print_to_buffer(session, "Content-Length: %d\r\n", content_len);
    }
    /* End of header */
    httpsrv_print_to_buffer(session, "\r\n");
    
    //if ((content_len == 0) && (!has_entity))
    //{
        httpsrv_send_buffer(session);
    //}
    session->response.hdrsent = 1;
}

/*
** Print data to session buffer
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session used for transmission
**      char*                   format - format for snprintf function
**      void                    ...    - parameters to print
**
** OUT:
**      none
**
** Return Value:
**      none
*/
static void httpsrv_print_to_buffer(HTTPSRV_SESSION_STRUCT *session, char* format, ...)
{	
    va_list ap;
    uint_32 req_space = 0;
    char* buffer = session->buffer.data;
    int buffer_space = (HTTPSRVCFG_SES_BUFFER_SIZE & PSP_MEMORY_ALIGNMENT_MASK)-session->buffer.offset;

    va_start(ap, format);
    /*
    ** First we always test if there is enough space in buffer. If there is not, 
    ** we flush it first and than write.
    */
    req_space = vsnprintf(NULL, 0, format, ap);
    va_end(ap);
    
    va_start(ap, format);
    if (req_space > buffer_space)
    {
        httpsrv_send_buffer(session);
        buffer_space = (HTTPSRVCFG_SES_BUFFER_SIZE & PSP_MEMORY_ALIGNMENT_MASK);
    }
    session->buffer.offset += vsnprintf(buffer+session->buffer.offset, buffer_space, format, ap);
    va_end(ap);
}

/*
** Convert file extension to content type and determine what kind of cache control should be used.
**
** IN:
**      char* extension - extension to convert
**
** IN/OUT:
**      HTTPSRV_SESSION_STRUCT* session - session pointer
**
** Return Value:
**      none
*/
static void httpsrv_process_file_type(char* extension, HTTPSRV_SESSION_STRUCT* session)
{
    httpsrv_content_table_row* row = content_tbl;
    uint_32 length = 0;

    if (extension != NULL)
    {
        length = strlen(extension);
    }

    /* List throught table rows until length match */
    while ((row->length) && (row->length < length))
    {
        row++;
    }

    /* Do a search in valid rows */
    while (row->length == length)
    {
        if (strcasecmp(extension, row->ext) == 0)
        {
            session->response.content_type = row->content_type;
            session->response.cacheable = row->use_cache;

            if (session->response.auth_realm != NULL)
            {
                /* If authentication is required, then response MUST NOT be cached */
                session->response.cacheable = FALSE;
            }
            return;
        }
        row++;
    }

    session->response.content_type = HTTPSRV_CONTENT_TYPE_OCTETSTREAM;
    session->response.cacheable = FALSE;
}

/*
** Send file to client
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session used for transmission
**      HTTPSRV_STRUCT*         server - server structure
**
** OUT:
**      none
**
** Return Value:
**      none
*/
void httpsrv_sendfile(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session)
{
    char *ext;
    int expand = 0;
    int len;
    char* buf = session->buffer.data;

    ext = strrchr(session->request.path, '.');

    if (ext != NULL)
    {
        httpsrv_process_file_type(ext+1, session);
    }
    else
    {
        session->response.content_type = HTTPSRV_CONTENT_TYPE_OCTETSTREAM;
    }

    /* Check if file has server side includes */
    if ((0 == strcasecmp(ext, ".shtml")) || (0 == strcasecmp(ext, ".shtm")))
    {
        expand = 1;
        /* Disable keep-alive for this session otherwise we would have to wait for session timeout */
        session->keep_alive = 0;
        /* If there will be any expansion there's no way how to calculate correct length
        ** zero length prevents sending Content-Length header field. */
        httpsrv_sendhdr(session, 0, 1);
    }
    else
    {
        httpsrv_sendhdr(session, session->response.file->SIZE, 1);
    }

    if (expand)
    {
        len = read(session->response.file, buf+session->buffer.offset, (HTTPSRVCFG_SES_BUFFER_SIZE & PSP_MEMORY_ALIGNMENT_MASK)-session->buffer.offset);
        
        if (len > 0)
        {
            len = httpsrv_sendextstr(server, session, buf, len);
            if (!len)
            {
                session->state = HTTPSRV_SES_END_REQ;
            }
            else
            {
                session->response.len += len;
                fseek(session->response.file, session->response.len, IO_SEEK_SET);
            }
        }
        else
        {
            session->state = HTTPSRV_SES_END_REQ;
        }    
    }
    else
    {
        fseek(session->response.file, session->response.len, IO_SEEK_SET);
        len = read(session->response.file, buf+session->buffer.offset, (HTTPSRVCFG_SES_BUFFER_SIZE & PSP_MEMORY_ALIGNMENT_MASK)-session->buffer.offset);
        
        if (len > 0)
        {
            uint_32 retval = 0;
            
            session->buffer.offset += len;
            retval = httpsrv_send_buffer(session);
            if (retval != RTCS_ERROR)
            {
                session->response.len += len;
            } 
        }
        else
        {
             session->state = HTTPSRV_SES_END_REQ;
        }    
    }
}

/*
** Send error page to client
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session used for transmission
**      const char*             title - title of error page
**      const char*             text - text displayed on error page
**
** OUT:
**      none
**
** Return Value:
**      none
*/
void httpsrv_send_err_page(HTTPSRV_SESSION_STRUCT *session, const char* title, const char* text)
{
    uint_32 length;
    char* page;
    
    length = snprintf(NULL, 0, ERR_PAGE_FORMAT, title, text);
    length++;
    page = _mem_alloc(length*sizeof(char));

    session->response.content_type = HTTPSRV_CONTENT_TYPE_HTML;

    if (page != NULL)
    {
        snprintf(page, length, ERR_PAGE_FORMAT, title, text);
        httpsrv_sendhdr(session, strlen(page), 1);
        httpsrv_write(session, page, strlen(page));
        httpsrv_send_buffer(session);
        _mem_free(page);
    }
    else
    {
        httpsrv_sendhdr(session, 0, 0);
    }
}

/*
** Process one line of http request header
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer.
**      HTTPSRV_STRUCT *server - pointer to server structure (needed for session parameters).
**      char* buffer - pointer to begining of line with request.
**
** OUT:
**      none
**
** Return Value:
**      none
*/
void httpsrv_process_req_hdr_line(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT* session, char* buffer)
{
    char* param_ptr = NULL;

    if (strncmp(buffer, "Connection: ", 12) == 0)
    {
        param_ptr = buffer+12;

        if ((session->keep_alive_enabled) &&
           ((strncmp(param_ptr, "keep-alive", 10) == 0) || 
            (strncmp(param_ptr, "Keep-Alive", 10) == 0)))
        {
            /* You should keep in mind that HTTP 1.0 does not officialy support keep alive */
            /* This is only work around */
            session->keep_alive = 1;
        }
        else
        {
            session->keep_alive = 0;
        }
    }
    else if (strncmp(buffer, "Content-Length: ", 16) == 0)
    {
        param_ptr = buffer+16;
        session->request.content_length = strtoul(param_ptr, NULL, 10);
    }
    else if (strncmp(buffer, "Content-Type: ", 14) == 0)
    {
        param_ptr = buffer+14;
        session->request.content_type = get_tbl_id((tbl_str_itm_t*) content_type, param_ptr, strlen(param_ptr)-2);
        if (session->request.content_type == 0)
        {
            session->request.content_type = HTTPSRV_CONTENT_TYPE_OCTETSTREAM;
        }
    }
    else if (strncmp(buffer, "Authorization: ", 15) == 0)
    {
        param_ptr = buffer+15;
        if (strncmp(param_ptr, "Basic ", 6) == 0)
        {
            uint_32 decoded_length = 0;
            char* user = NULL;
            char* pass = NULL;
            param_ptr += 6;

            /* evaluate number of bytes required for worst case (no padding) */
            decoded_length = (strlen(param_ptr)/4) * 3 + 1;
            user = _mem_alloc_zero(sizeof(char)*decoded_length);
            if (user != NULL)
            {
                _mem_set_type(user, MEM_TYPE_HTTPSRV_AUTH);
                base64_decode(user, param_ptr, decoded_length);
                session->request.auth.user_id = user;
            }
            else
            {
                return;
            }

            pass = strchr(user, ':');
            if (param_ptr)
            {
                *pass = '\0';
                pass = pass + 1;
                session->request.auth.password = pass;
            }

        }
    }
}

/*
** Read http method 
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer.
**      HTTPSRV_STRUCT *server - pointer to server structure (needed for session parameters).
**      char* buffer - pointer to begining of line with request.
**
** OUT:
**      none
**
** Return Value:
**      none
*/
void httpsrv_process_req_method(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, char* buffer)
{
    char* uri_begin  = NULL;
    char* uri_end = NULL;
    char method[5] = {'\0'};
    uint_32 written;

    snprintf(method, sizeof(method), "%s", buffer);
    if (strncmp(method, "GET", 3) == 0)
    {
        session->request.method = HTTPSRV_REQ_GET;
    }
    else if (strncmp(method, "POST", 4) == 0)
    {
        session->request.method = HTTPSRV_REQ_POST;
    }
    else /* Unknown method - not implemented response */
    {  
        session->request.method = HTTPSRV_REQ_UNKNOWN;
        return;
    }

    /* Parse remaining part of line */
    uri_begin = strchr(buffer, ' ') + 1;

    if (uri_begin != NULL)
    {
        uri_end = strchr(uri_begin, ' ');
        if (uri_end != NULL)
        {
            *uri_end = '\0';
        }
    }
    /* Preprocess URI */
    httpsrv_urldecode(uri_begin);
    httpsrv_urlcleanup(uri_begin);

    written = snprintf(session->request.path, server->params.max_uri, "%s", uri_begin);
    /* Check if whole URI is saved in buffer */
    if (written > server->params.max_uri-1)
    {
        session->request.path[0] = '\0';
        /* URI is too long so we set proper status code for response */
        session->response.status_code = 414;
    }
}

/*
** Decode percent encoded string (inplace)
**
** IN:
**      char* url - string to decode
**
** OUT:
**      none
**
** Return Value:
**      none
*/
static void httpsrv_urldecode(char* url)
{
    char* src = url;
    char* dst = url;

    while(*src != '\0')
    {
        if ((*src == '%') && (isxdigit(*(src+1))) && (isxdigit(*(src+2))))
        {
            *src = *(src+1);
            *(src+1) = *(src+2);
            *(src+2) = '\0';
            *dst++ = strtol(src, NULL, 16);
            src += 3;
        }
        else
        {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

/*
** URL cleanup (remove invalid path segments - /./ and /../)
**
** IN:
**      char* url - string to decode
**
** OUT:
**      none
**
** Return Value:
**      none
*/
static void httpsrv_urlcleanup(char* url)
{
    char* src = url;
    char* dst = url;

    while(*src != '\0')
    {
        if ((src[0] == '/') && (src[1] == '.'))
        {
            if (src[2] ==  '/')
            {
                src += 2;
            }
            else if ((src[2] == '.') && (src[3] == '/'))
            {
                src += 3;
            }
        }
        *dst++ = *src++;
    }
    *dst = '\0';
}

/*
** Write data to client from CGI script
**
** IN:
**      HTTPSRV_CGI_RES_STRUCT* response - CGI response structure used for forming response
**
** OUT:
**      none
**
** Return Value:
**      uint_32 - Number of bytes written
*/
uint_32 HTTPSRV_cgi_write(HTTPSRV_CGI_RES_STRUCT* response)
{
    HTTPSRV_SESSION_STRUCT* session = (HTTPSRV_SESSION_STRUCT*) response->ses_handle;
    uint_32 retval = 0;

    if (session == NULL)
    {
        return(0);
    }

    if (session->response.hdrsent == 0)
    {
        session->response.status_code = response->status_code;
        session->response.content_type = response->content_type;
        session->response.len = response->content_length;
        /* 
        ** If there is no content length we have to disable keep alive.
        ** Otherwise we would have to wait for session timeout.
        */
        if (session->response.len == 0)
        {
            session->keep_alive = 0;
        }
        httpsrv_sendhdr(session, response->content_length, 1);
    }
    if ((response->data != NULL) && (response->data_length))
    {
        retval = httpsrv_write(session, response->data, response->data_length);
    }
    
    return(retval);
}

/*
** Read data from client to CGI script
**
** IN:
**      uint_32 ses_handle - handle to session used for reading
**      char*   buffer - user buffer to read data to
**      uint_32 length - size of buffer in bytes
**
** OUT:
**      none
**
** Return Value:
**      uint_32 - Number of bytes read
*/
uint_32 HTTPSRV_cgi_read(uint_32 ses_handle, char* buffer, uint_32 length)
{
    HTTPSRV_SESSION_STRUCT* session = (HTTPSRV_SESSION_STRUCT*) ses_handle;
    uint_32 retval;

    retval = httpsrv_read(session, buffer, length);

    if (retval > 0)
    {
        session->request.content_length -= retval;
    }
    
    return(retval);
}

/*
** Write data to client from server side include
**
** IN:
**      uint_32 ses_handle - session foe writing
**      char*   data - user data to write
**      uint_32 length - size of data in bytes
**
** OUT:
**      none
**
** Return Value:
**      uint_32 - Number of bytes written
*/
uint_32 HTTPSRV_ssi_write(uint_32 ses_handle, char* data, uint_32 length)
{
    HTTPSRV_SESSION_STRUCT* session = (HTTPSRV_SESSION_STRUCT*) ses_handle;
    uint_32 retval = 0;

    if ((session != NULL) && (data != NULL) && (length))
    {
        retval = httpsrv_write(session, data, length);
    }
    
    return(retval);
}
