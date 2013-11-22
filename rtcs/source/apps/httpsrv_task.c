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
* $FileName: httpsrv_task.c$
*
* Comments:
*
*   HTTPSRV tasks and session processing.
*
*END************************************************************************/

#include "httpsrv.h"
#include "httpsrv_prv.h"
#include "httpsrv_supp.h"
#include "socket.h"
#include "message.h"

static int httpsrv_readreq(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session);
static HTTPSRV_AUTH_REALM_STRUCT* httpsrv_req_realm(HTTPSRV_STRUCT *server, char* path);
static int httpsrv_check_auth(HTTPSRV_AUTH_REALM_STRUCT* realm, HTTPSRV_AUTH_USER_STRUCT* user);
static void httpsrv_ses_process(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session);
static void httpsrv_processreq(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session);
static void httpsrv_response(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session);

static HTTPSRV_SESSION_STRUCT* httpsrv_ses_alloc(HTTPSRV_STRUCT *server);
static void httpsrv_ses_free(HTTPSRV_SESSION_STRUCT *session);
static void httpsrv_ses_close(HTTPSRV_SESSION_STRUCT *session);
static void httpsrv_ses_init(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, const int sock);

static void httpsrv_session_task(pointer init_ptr, pointer creator);
static void httpsrv_process_cgi(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, char* cgi_name);

/*
** HTTPSRV main task which creates new task for each new client request
*/
void httpsrv_server_task(pointer init_ptr, pointer creator) 
{
    HTTPSRV_STRUCT   *server = (HTTPSRV_STRUCT*)init_ptr;
    _mqx_uint res;
    
    if (server == NULL)
    {
        goto ERROR;
    }
    
    server->server_tid = _task_get_id();

    res = _lwsem_create(&server->ses_cnt, server->params.max_ses);  
    if (res != MQX_OK)
    {
        goto ERROR;
    }
    
    RTCS_task_resume_creator(creator, RTCS_OK);
    
    while (server->run) 
    {
        uint_32          connsock = 0;
        uint_32          new_sock;

        /* limit number of opened sessions */
        _lwsem_wait(&server->ses_cnt);

        /* Get socket with incoming connection (IPv4 or IPv6) */
        while (!connsock && server->run)
        {
            connsock = RTCS_selectset(&(server->sock_v4), 2, 250);
        }
        
        if (server->run)
        {
            struct sockaddr  remote_addr;
            unsigned short   length;

            new_sock = accept(connsock, (sockaddr *) &remote_addr, &length);
            connsock = 0;
        }
        else
        {
            break;
        }

        if (new_sock != RTCS_SOCKET_ERROR)
        {
            HTTPSRV_SES_TASK_PARAM  ses_param;
            _mqx_uint               error;

            ses_param.server = server;
            ses_param.sock = new_sock;
            /* Try to create task for session */
            error = RTCS_task_create("httpsrv session", server->params.server_prio, HTTPSRV_SESSION_STACK_SIZE, httpsrv_session_task, &ses_param);
            if (MQX_OK != error)
            {
            	shutdown(new_sock, FLAG_ABORT_CONNECTION);
                _lwsem_post(&server->ses_cnt);
            }
        }
        else
        {
            _lwsem_post(&server->ses_cnt);
            /* We probably run out of sockets. Wait some time than try again to prevent session tasks resource starvation */
            _time_delay(150);
        }
    } 
ERROR:
    _lwsem_destroy(&server->ses_cnt);
    server->server_tid = 0;
    RTCS_task_resume_creator(creator, (uint_32)RTCS_ERROR);
}

/*
** Session task.
** This task is responsible for session creation, processing and cleanup.
*/
static void httpsrv_session_task(pointer init_ptr, pointer creator) 
{
    HTTPSRV_STRUCT* server = ((HTTPSRV_SES_TASK_PARAM*) init_ptr)->server;
    uint_32 sock = ((HTTPSRV_SES_TASK_PARAM*) init_ptr)->sock;
    HTTPSRV_SESSION_STRUCT *session;
    uint_32 i;
    boolean found = FALSE;
    _task_id tid = _task_get_id();

    /* Find empty session */
    _lwsem_wait(&server->tid_sem);

    for (i = 0; i < server->params.max_ses; i++)
    {
        if (server->session[i] == NULL)
        {
            found = TRUE;
            break;
        }
    }
    
    if (!found)
    {
        RTCS_task_resume_creator(creator, (uint_32) RTCS_ERROR);
        _lwsem_post(&server->tid_sem);
        _lwsem_post(&server->ses_cnt);
        _task_destroy(MQX_NULL_TASK_ID);
    }
  
    /* Save task ID - used for indication of running task */
    server->ses_tid[i] = tid;
    /* Access to array done. Unblock other tasks. */
    _lwsem_post(&server->tid_sem);
    
    /* Allocate session */
    session = httpsrv_ses_alloc(server);
        
    if (session) 
    {
        server->session[i] = session;

        RTCS_task_resume_creator(creator, RTCS_OK);       
        httpsrv_ses_init(server, session, sock);
    
        /* Disable keep-alive for last session so we have at least one session free (not blocked by keep-alive timeout) */
        if (i == server->params.max_ses - 1)
        {
            session->keep_alive_enabled = FALSE;
        }

        while (HTTPSRV_SESSION_VALID == session->valid) 
        {
            /* Run state machine for session */
            httpsrv_ses_process(server, session);
            _sched_yield();
        }
        /* If session is not closed close it */
        if (session->state != HTTPSRV_SES_CLOSE)
        {
            httpsrv_ses_close(session);
        }
        httpsrv_ses_free(session);
        server->session[i] = NULL;
    }
    else 
    {
        RTCS_task_resume_creator(creator, (uint_32) RTCS_ERROR);
    }
    /* Cleanup and end task */
    _lwsem_post(&server->ses_cnt);
    /* Null tid => task is no longer running */
    _lwsem_wait(&server->tid_sem);
    server->ses_tid[i] = 0;
    _lwsem_post(&server->tid_sem);
}

/*
** Task for CGI/SSI handling.
*/
void httpsrv_script_task(pointer param, pointer creator)
{
    HTTPSRV_STRUCT* server = (HTTPSRV_STRUCT*) param;

    server->script_tid = _task_get_id();
    /* Create pool */
    server->script_msg_pool = _msgpool_create(sizeof(HTTPSRV_SCRIPT_MSG), server->params.max_ses, 0, server->params.max_ses);
    if (server->script_msg_pool == 0)
    {
        goto EXIT;
    }

    /* Open queue */
    server->script_msgq = _msgq_open(MSGQ_FREE_QUEUE, 0);
    if (server->script_msgq == MSGQ_NULL_QUEUE_ID)
    {
        goto EXIT;
    }

    RTCS_task_resume_creator(creator, RTCS_OK);

    /* Read messages */
    while(1)
    {
        HTTPSRV_CALL_PARAM call_param;
        HTTPSRV_SESSION_STRUCT* session;
        HTTPSRV_SCRIPT_MSG* msg_ptr;

        msg_ptr = _msgq_receive(server->script_msgq, 0);
        if (msg_ptr == NULL)
        {
            goto EXIT;
        }
        /* NULL name and session => exit task */
        if ((msg_ptr->name == NULL) && (msg_ptr->session == NULL))
        {
            _msg_free(msg_ptr);
            break;
        }
        session = msg_ptr->session;
        /* CGI callback */
        if (msg_ptr->type == HTTPSRV_CGI_CALLBACK)
        {
            HTTPSRV_CGI_REQ_STRUCT cgi_param;
            char server_ip[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")+1];
            char remote_ip[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")+1];
            struct sockaddr l_address;
            struct sockaddr r_address;
            uint_16 len;
            _mqx_int retval = 0;

            cgi_param.ses_handle = (uint_32) session;
            cgi_param.request_method = session->request.method;
            cgi_param.content_type = (HTTPSRV_CONTENT_TYPE) session->request.content_type;
            cgi_param.content_length = session->request.content_length;
            cgi_param.server_port = server->params.port;
            
            getsockname(session->sock, &l_address, &len);
            getpeername(session->sock, &r_address, &len);
            #if RTCSCFG_ENABLE_IP4
            if (l_address.sa_family == AF_INET)
            {
                unsigned long l_addr = ((struct sockaddr_in*) &l_address)->sin_addr.s_addr;
                unsigned long r_addr = ((struct sockaddr_in*) &r_address)->sin_addr.s_addr;
                
                snprintf(server_ip, sizeof(server_ip), "%d.%d.%d.%d", IPBYTES(l_addr));
                snprintf(remote_ip, sizeof(remote_ip), "%d.%d.%d.%d", IPBYTES(r_addr));
            }
            #endif
            #if RTCSCFG_ENABLE_IP6
            if (l_address.sa_family == AF_INET6)
            {
                inet_ntop(AF_INET6, ((struct sockaddr_in6*) &l_address)->sin6_addr.s6_addr, server_ip, sizeof(server_ip));
                inet_ntop(AF_INET6, ((struct sockaddr_in6*) &r_address)->sin6_addr.s6_addr, remote_ip, sizeof(remote_ip));
            }
            #endif

            cgi_param.auth_type = HTTPSRV_AUTH_BASIC;
            cgi_param.remote_user = session->request.auth.user_id;
            cgi_param.remote_addr = remote_ip;
            cgi_param.server_name = server_ip;
            cgi_param.script_name = msg_ptr->name;
            cgi_param.server_protocol = HTTPSRV_PROTOCOL_STRING;
            cgi_param.server_software = HTTPSRV_PRODUCT_STRING;
            cgi_param.query_string = session->request.query;
            cgi_param.gateway_interface = HTTPSRV_CGI_VERSION_STRING;

            call_param.table = (HTTPSRV_FN_LINK_STRUCT*) server->params.cgi_lnk_tbl;
            call_param.param = &cgi_param;
            call_param.fn_name = msg_ptr->name;
            call_param.type = msg_ptr->type;

            retval = httpsrv_call_fn(&call_param);
            if (retval == HTTPSRV_CGI_INVALID)
            {
                session->response.status_code = 404;
            }
            else
            {
                /* Flush out buffer so all data are send to client */
                httpsrv_send_buffer(msg_ptr->session);
            }
        }
        /* Server Side Include */
        else if (msg_ptr->type == HTTPSRV_SSI_CALLBACK)
        {
            char* tmp;
            HTTPSRV_SSI_PARAM_STRUCT ssi_param;
            _mqx_int retval = 0;

            ssi_param.ses_handle = (uint_32) msg_ptr->session;
            tmp = strchr(msg_ptr->name, ':');
            if (tmp != NULL)
            {
                *tmp++ = '\0';
            }
            else
            {
                tmp = "";
            }
            ssi_param.com_param = tmp;
            call_param.param = &ssi_param;
            call_param.table = (HTTPSRV_FN_LINK_STRUCT*) server->params.ssi_lnk_tbl;
            call_param.fn_name = msg_ptr->name;
            call_param.type = msg_ptr->type;

            retval = httpsrv_call_fn(&call_param);
            if (retval != HTTPSRV_CGI_INVALID)
            {
                /* Flush out buffer so all data are send to client */
                httpsrv_send_buffer(msg_ptr->session);
            }
        }
        _task_ready(_task_get_td(msg_ptr->ses_tid));
        _msg_free(msg_ptr);
    }

EXIT:
    if (server->script_msgq != MSGQ_NULL_QUEUE_ID)
    {
       _msgq_close(server->script_msgq);
       server->script_msgq = 0;
    }
    
    if (server->script_msg_pool)
    {
        _msgpool_destroy(server->script_msg_pool);
        server->script_msg_pool = 0;
    }
    server->script_tid = 0;
}

/*
** HTTP session state machine
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer.
**      HTTPSRV_STRUCT *server - pointer to server structure (needed for session parameters).
**
** OUT:
**      none
**
** Return Value: 
**      none
*/
static void httpsrv_ses_process(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session)
{
    uint_32 time_now;
    int     res;

    if (HTTPSRV_SESSION_VALID != session->valid)
    {
        session->state = HTTPSRV_SES_CLOSE;
    }
        
    /* check session timeout */
    time_now = RTCS_time_get();

    if (RTCS_timer_get_interval(session->start_time, time_now) > session->timeout)
    {
        session->state = HTTPSRV_SES_CLOSE;
    }

    switch (session->state)
    {
        case HTTPSRV_SES_WAIT_REQ:
            res = httpsrv_readreq(server, session);
            
            if ((res < 0) && (!session->keep_alive) && (HTTPSRV_SES_CLOSE != session->state))
            {
                session->state = HTTPSRV_SES_END_REQ;
            }
            else if ((res == 0) && (session->request.process_header == 0))
            {
                session->state = HTTPSRV_SES_PROCESS_REQ;
            }
            break;
        case HTTPSRV_SES_PROCESS_REQ:
            httpsrv_processreq(server, session);
            break;

        case HTTPSRV_SES_RESP:
            httpsrv_response(server, session);
            if (session->keep_alive)
            {
                session->start_time = RTCS_time_get();
            }
            break;

        case HTTPSRV_SES_END_REQ:
            if (!session->keep_alive)
            {
                session->state = HTTPSRV_SES_CLOSE;
            }
            else
            {
                /* Re-init session */
                session->state = HTTPSRV_SES_WAIT_REQ;
                if (session->response.file)
                {
                    fclose(session->response.file);
                }
                _mem_zero(&session->response, sizeof(session->response));
                session->request.process_header = 1;
                if (session->request.auth.user_id != NULL)
                {
                    _mem_free(session->request.auth.user_id);
                }
                session->request.auth.user_id = NULL;
                session->request.auth.password = NULL;
                session->start_time = RTCS_time_get();
                session->timeout = HTTPSRVCFG_KEEPALIVE_TO;
                session->keep_alive = 1;
                break;
            }

        case HTTPSRV_SES_CLOSE:
            httpsrv_ses_close(session);
            session->valid = HTTPSRV_SESSION_INVALID;
            break;
        default:
            // invalid state
            session->valid = HTTPSRV_SESSION_INVALID;
            break;
    }
}

/*
** Function for request parsing
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer.
**      HTTPSRV_STRUCT *server - pointer to server structure (needed for session parameters).
**
** OUT:
**      none
**
** Return Value: 
**      int - zero if request is valid, negative value if invalid.
*/
static int httpsrv_readreq(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session)
{
    char* buf = session->buffer.data;
    char *query_str;
    int read;
    uint_32 req_lines = 0;    
    char* end = NULL;
    
    /* Begin */
    while (session->request.process_header)
    {
        /* Read data */
        read = recv(session->sock, buf+session->buffer.offset, HTTPSRVCFG_SES_BUFFER_SIZE-session->buffer.offset, 0);
        if ((read == RTCS_ERROR) || (read == 0))
        {
            if (RTCS_geterror(session->sock) == RTCSERR_TCP_CONN_CLOSING)
            {
                /* Client closed connection so we close it too */
                session->keep_alive = 0;
                session->state = HTTPSRV_SES_CLOSE;
            }
            break;
        }
        /* Process data */
        while (session->request.process_header && ((end = strchr(buf, '\n')) != NULL))
        {
            *end = '\0';
            if (*(end-1) == '\r')
            {
                *(end-1) = '\0';
            }
            req_lines++;
            end++;
            /* Found an empty line => end of header */
            if (!strlen(buf))
            {
                session->request.process_header = FALSE;
            }
            else
            {
                if (req_lines == 1) 
                {
                    httpsrv_process_req_method(server, session, buf);
                }
                else
                {
                    httpsrv_process_req_hdr_line(server, session, buf);
                }
            }
            buf = end;
        }
        session->buffer.offset = strlen(buf);
        if (session->buffer.offset == HTTPSRVCFG_SES_BUFFER_SIZE)
        {
            _mem_zero(session->buffer.data, HTTPSRVCFG_SES_BUFFER_SIZE);
            session->buffer.offset = 0;
        }
        /* We are not at the end of buffer and there are some data after the end of last line. */
        if (session->buffer.offset != 0)
        {
            /* Copy rest of data to begining of buffer and save offset for next reading. */
            memmove(session->buffer.data, buf, session->buffer.offset);
            buf = session->buffer.data;
        }
        /* Clear the buffer so we don't have some old data there. */
        _mem_zero(session->buffer.data+session->buffer.offset, HTTPSRVCFG_SES_BUFFER_SIZE-session->buffer.offset);
    }
    /* End */

    /* Save query string */
    query_str = strchr(session->request.path, '?');
    if (NULL != query_str)
    {
        *query_str = '\0';
        query_str++;
        session->request.query = query_str;
    }

    return (0 > read) ? read : 0;
}

/*
** Function for request processing
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer.
**      HTTPSRV_STRUCT *server - pointer to server structure (needed for session parameters).
**
** OUT:
**      none
**
** Return Value: 
**      none
*/

static void httpsrv_processreq(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session)
{
    char *cp;
    char *cp1;
    char* buffer = session->buffer.data;
    uint_32 space = HTTPSRVCFG_SES_BUFFER_SIZE - session->buffer.offset;
    HTTPSRV_ALIAS* table_row = server->params.alias_tbl;
    char* root_dir = NULL;
    boolean free_buffer = FALSE;
    uint_32 length;
    uint_32 alias_length = 0;
    
    session->state = HTTPSRV_SES_RESP;
    /* If method is not implemented return without request processing */
    if (session->request.method == HTTPSRV_REQ_UNKNOWN)
    {
        session->response.status_code = 501;
    }
    /* We need content length for post requests */
    else if ((session->request.method == HTTPSRV_REQ_POST) && (session->request.content_length == 0))
    {
        session->response.status_code = 411;
    }
    
    /* Check for aliases */
    while (table_row && (table_row->path != NULL) && (table_row->alias != NULL))
    {
        if (!strncmp(table_row->alias, session->request.path, strlen(table_row->alias)))
        {
            alias_length = strlen(table_row->alias);
            root_dir = table_row->path;
            break;
        }
        table_row++;
    }
    /* No alias found use default root */
    if (root_dir == NULL)
    {
        root_dir = server->params.root_dir;
    }

    /* Check request path */
    cp = strrchr(session->request.path, '/');
    if ((cp == NULL) && session->response.status_code == 0)
    {
        /* We have invalid request */
        session->response.status_code = 400;
    }

    /* If response status is not already set than set it to default value */
    if (session->response.status_code == 0)
    {
        session->response.status_code = 200;
    }
    else
    {
        /* We already know how to respond and response is not going to be file/CGI */
        return;
    }

    /* Check auth */
    session->response.auth_realm = httpsrv_req_realm(server, session->request.path);
    if (session->response.auth_realm != NULL)
    {
        if (!httpsrv_check_auth(session->response.auth_realm, &session->request.auth))
        {
            session->response.status_code = 401;
            if (session->request.auth.user_id != NULL)
            {
                _mem_free(session->request.auth.user_id);
                session->request.auth.user_id = NULL;
                session->request.auth.password = NULL;
            }
            return;
        }
    }

    if (session->request.path[0] == '/' && session->request.path[1] == '\0')
    {
        snprintf(session->request.path, server->params.max_uri + 1, "%s", server->params.index_page);
    }

    /* Check if requested file is CGI script */
    if ((cp1 = strrchr(cp, '.')) != 0)
    {
        if (0 == strcasecmp(cp1, ".cgi"))
        {
            *cp1 = '\0';
            httpsrv_process_cgi(server, session, cp+1);
            *cp1 = '.';

            /* if cgi call was successful skip response, end session */
            if (session->response.status_code == 200)
            {
                session->state = HTTPSRV_SES_END_REQ;
            }
            return;
        }
    }

    length = strlen(session->request.path) + strlen(root_dir) - alias_length + 1;
    /* Session buffer is too small */
    if (length > space)
    {
        buffer = _mem_alloc_zero(length);
        if (buffer == NULL)
        {
            /* Out of memory - report server error */
            session->response.status_code = 500;
            return;
        }
        free_buffer = TRUE;
        space = length;
    }
    snprintf(buffer, space, "%s%s", root_dir , session->request.path+alias_length);
    cp = buffer;
    
    /* Correct path slashes */
    while (*cp)
    {
        if ('/' == *cp)
        {
            *cp = '\\';
        }
        cp++;
    }

    session->response.file = fopen(buffer, "r");
    session->response.len = 0;

    if (!session->response.file)
    {
        session->response.status_code = 404;
    }
    if (buffer && free_buffer)
    {
        _mem_free(buffer);
    }
    else if (!free_buffer)
    {
        _mem_zero(buffer, space);
    }
}

/*
** Get realm for requested path
**
** IN:
**      char*            path - search path.
**      HTTPSRV_STRUCT*  server - pointer to server structure (needed for session parameters).
**
** OUT:
**      none
**
** Return Value: 
**      HTTPSRV_AUTH_REALM_STRUCT* - authentication realm for requested path. Null if not found.
*/
static HTTPSRV_AUTH_REALM_STRUCT* httpsrv_req_realm(HTTPSRV_STRUCT *server, char* path)
{
    HTTPSRV_AUTH_REALM_STRUCT* table = server->params.auth_table;
    
    if (table == NULL)
    {
        return(NULL);
    }

    while((table->path != NULL) && (strstr(path, table->path) == NULL))
    {
        table++;
    }

    return(table->path ? table : NULL);
}

/*
** Check user authentication credentials
**
** IN:
**      HTTPSRV_AUTH_REALM_STRUCT* realm - search realm.
**      HTTPSRV_AUTH_USER_STRUCT*  user - user to authenticate.
**
** OUT:
**      none
**
** Return Value: 
**      int - 1 if user is sucessfully authenticated, zero otherwise.
*/
static int httpsrv_check_auth(HTTPSRV_AUTH_REALM_STRUCT* realm, HTTPSRV_AUTH_USER_STRUCT* user)
{
    HTTPSRV_AUTH_USER_STRUCT* users = NULL;
    
    if ((realm == NULL) || (user == NULL))
    {
        return(0);
    }

    users = realm->users;

    while (users->user_id != NULL)
    {
        if (!strcmp(users->user_id, user->user_id) && !strcmp(users->password, user->password))
        {
            return(1);
        }
        users++;
    }
    return(0);
}
/*
** Function for CGI request processing
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer.
**      HTTPSRV_STRUCT*         server - pointer to server structure (needed for session parameters).
**      char*                   cgi_name - name of cgi function.
**
** OUT:
**      none
**
** Return Value: 
**      none
*/
static void httpsrv_process_cgi(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, char* cgi_name)
{
    HTTPSRV_SCRIPT_MSG* msg_ptr;

    msg_ptr = _msg_alloc(server->script_msg_pool);

    if (msg_ptr == NULL)
    {
        /* Out of memory - report server error */
        session->response.status_code = 500;
        return;
    }

    msg_ptr->header.TARGET_QID = server->script_msgq;
    msg_ptr->header.SOURCE_QID = server->script_msgq;
    msg_ptr->header.SIZE = sizeof(HTTPSRV_SCRIPT_MSG);
    msg_ptr->session = session;
    msg_ptr->type = HTTPSRV_CGI_CALLBACK;
    msg_ptr->name = cgi_name;
    msg_ptr->ses_tid = _task_get_id();
    _msgq_send(msg_ptr);
    /* wait until CGI is processed */
    _task_block();

    /*
    ** There is some unread content from client after CGI finished. 
    ** It must be read and discarded if we have keep-alive enabled
    ** so it does not affect next request.
    */
    if (session->request.content_length)
    {
        /* Whole content is not in buffer. Read it from session. */
        if (session->buffer.offset < session->request.content_length)
        {
            char *tmp = NULL;
            session->request.content_length -= strlen(session->buffer.data);
            _mem_zero(session->buffer.data, session->buffer.offset);
            tmp = _mem_alloc(session->request.content_length);
            if (tmp != NULL)
            {
                /* Read rest */
                httpsrv_read(session, tmp, session->request.content_length);
                _mem_free(tmp);
                session->request.content_length = 0;
            } 
            session->buffer.offset = 0;
        }
        else /* Whole content is in buffer. Just discard it. */
        {
            _mem_zero(session->buffer.data, session->request.content_length);
            session->buffer.offset = 0;
            session->request.content_length = 0;
        }
    }

    return;
}

/*
** Function for HTTP sending response
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer.
**      HTTPSRV_STRUCT *server - pointer to server structure (needed for session parameters).
**
** OUT:
**      none
**
** Return Value: 
**      none
*/

static void httpsrv_response(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session)
{   
    switch (session->response.status_code)
    {
        case 200:
            httpsrv_sendfile(server, session);
            break;
        case 401:
            session->state = HTTPSRV_SES_END_REQ;
            httpsrv_send_err_page(session, "401 Unauthorized", "Unauthorized!");
            break;
        case 403:
            session->state = HTTPSRV_SES_END_REQ;
            httpsrv_send_err_page(session, "403 Forbidden", "Forbidden!");
            break;
        case 414:
            session->keep_alive = 0;
            session->state = HTTPSRV_SES_END_REQ;
            httpsrv_send_err_page(session, "414 Uri too long", "Requested URI too long!");
            break;
        case 400:
        case 411:
        case 500:
        case 501:
            session->keep_alive = 0;
            session->state = HTTPSRV_SES_END_REQ;
            httpsrv_sendhdr(session, 0, 0);
            break;
        default:
            session->state = HTTPSRV_SES_END_REQ;
            httpsrv_send_err_page(session, "404 Not Found", "Requested URL not found!");
            break;
    }
}

/*
** Function for session allocation
**
** IN:
**      HTTPSRV_STRUCT *server - pointer to server structure (needed for session parameters).
**
** OUT:
**      none
**
** Return Value: 
**      HTTPSRV_SESSION_STRUCT* - pointer to allocated session. Non-zero if allocation was OK, NULL otherwise
*/
static HTTPSRV_SESSION_STRUCT* httpsrv_ses_alloc(HTTPSRV_STRUCT *server)
{
    HTTPSRV_SESSION_STRUCT *session = NULL;

    if (server)
    {
        session = _mem_alloc_zero(sizeof(HTTPSRV_SESSION_STRUCT));
        if (session)
        {
            _mem_set_type(session, MEM_TYPE_HTTPSRV_SESSION_STRUCT);
            /* Alloc URI */
            session->request.path = _mem_alloc_zero(server->params.max_uri + 1);
            if (NULL == session->request.path)
            {
                goto ERROR;
            } 
            _mem_set_type(session->request.path, MEM_TYPE_HTTPSRV_URI);
            /* Alloc session buffer */
            session->buffer.data = _mem_alloc_zero(sizeof(char)*HTTPSRVCFG_SES_BUFFER_SIZE);
            if (NULL == session->buffer.data)
            {
                goto ERROR;
            }
        }
    }

    return session;

ERROR:
    if (session->request.path)
    {
        _mem_free(session->request.path);
        session->request.path = NULL;
    }
    if (session->buffer.data)
    {
        _mem_free(session->buffer.data);
        session->buffer.data = NULL;
    }
    _mem_free(session);
    return NULL;
}

/*
** Function used to free session structure
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer
**
** OUT:
**      none
**
** Return Value: 
**      none
*/
static void httpsrv_ses_free(HTTPSRV_SESSION_STRUCT *session)
{
    if (session)
    {
        if (session->request.path)
        {
            _mem_free(session->request.path);
            session->request.path = NULL;
        }
        if(session->request.auth.user_id)
        {
            _mem_free(session->request.auth.user_id);
            session->request.auth.user_id = NULL;
            session->request.auth.password = NULL;
        }
        if(session->buffer.data)
        {
            _mem_free(session->buffer.data);
            session->buffer.data = NULL;
        }
        _mem_free(session);
    }
}

/*
** Function used to init session structure
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer
**      HTTPSRV_STRUCT *server - pointer to server structure (needed for session parameters)
**      const int sock - socket handle used for communication with client
**
** OUT:
**      none
**
** Return Value: 
**      none
*/
static void httpsrv_ses_init(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, const int sock)
{
    if (server && session)
    {
        // init session
        session->state = HTTPSRV_SES_WAIT_REQ;
        session->sock = sock;
        session->valid = HTTPSRV_SESSION_VALID;
        session->request.process_header = 1;
        session->timeout = HTTPSRVCFG_SES_TO;
        session->keep_alive_enabled = HTTPSRVCFG_KEEPALIVE_ENABLED;
        session->keep_alive = session->keep_alive_enabled;
        session->start_time = RTCS_time_get();
    }
}

/*
** Function used to close session
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer
**
** OUT:
**      none
**
** Return Value: 
**      none
*/
static void httpsrv_ses_close(HTTPSRV_SESSION_STRUCT *session)
{
    if (session != NULL)
    {
        if (session->response.file)
        {
            fclose(session->response.file);
            session->response.file = NULL;
        }
        shutdown(session->sock, FLAG_CLOSE_TX);
    }
}
/* EOF */
