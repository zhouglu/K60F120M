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
* $FileName: httpsrv.c$
*
* Comments:
*
*   This file contains the HTTPSRV implementation.
*
*END************************************************************************/

#include "httpsrv.h"
#include "httpsrv_supp.h"
#include "httpsrv_prv.h"
#include "rtcs_base64.h"
#include <string.h>
#include <stdlib.h>

static uint_32 httpsrv_init_socket (HTTPSRV_STRUCT *server, uint_16 family);
static uint_32 httpsrv_set_params  (HTTPSRV_STRUCT *server, HTTPSRV_PARAM_STRUCT* params);
/*
** Function for starting the HTTP server 
**
** IN:
**      HTTPSRV_PARAM_STRUCT*   params - server parameters (port, ip, index page etc.)
**
** OUT:
**      none
**
** Return Value: 
**      uint_32      server handle if successful, NULL otherwise
*/
uint_32 HTTPSRV_init(HTTPSRV_PARAM_STRUCT *params)
{
    HTTPSRV_STRUCT *server = NULL;
    uint_32 error;
    uint_32 error4 = HTTPSRV_OK;
    uint_32 error6 = HTTPSRV_OK;

    
    if ((server = _mem_alloc_zero(sizeof(HTTPSRV_STRUCT))) == NULL)
    {
        return((uint_32) NULL);
    }
    _mem_set_type(server, MEM_TYPE_HTTPSRV_STRUCT);

    error = _lwsem_create(&server->tid_sem, 1);
    if (error != MQX_OK)
    {
        HTTPSRV_release((uint_32) server);
        return((uint_32) NULL);
    }

    error = httpsrv_set_params(server, params);
    if (error != HTTPSRV_OK)
    {
        HTTPSRV_release((uint_32) server);
        return((uint_32) NULL);
    }
    
    /* Allocate space for session pointers */
    server->session = _mem_alloc_zero(sizeof(HTTPSRV_SESSION_STRUCT*) * server->params.max_ses);
    if (server->session == NULL)
    {
        HTTPSRV_release((uint_32) server);
        return((uint_32) NULL);
    }

    /* Allocate space for session task IDs */
    server->ses_tid = _mem_alloc_zero(sizeof(_rtcs_taskid) * server->params.max_ses);
    if (server->ses_tid == NULL)
    {
        HTTPSRV_release((uint_32) server);
        return((uint_32) NULL);
    }

    /* Init sockets. */
    if (params->af & AF_INET)
    {
        /* Setup IPv4 server socket */
        error4 = httpsrv_init_socket(server, AF_INET);
    }
    if (params->af & AF_INET6)
    {
        /* Setup IPv6 server socket */
        error6 = httpsrv_init_socket(server, AF_INET6);
    }

    if ((error4 != HTTPSRV_OK) || (error6 != HTTPSRV_OK))
    {
        HTTPSRV_release((uint_32) server);
        return((uint_32) NULL);
    }

    /* Create CGI handler if required */
    if ((server->params.cgi_lnk_tbl != NULL) || (server->params.ssi_lnk_tbl != NULL))
    {
        error = RTCS_task_create("httpsrv callback handler", server->params.script_prio, server->params.script_stack, httpsrv_script_task, server);
        if (error != RTCS_OK)
        {
            HTTPSRV_release((uint_32) server);
            return((uint_32) NULL);
        }
    }
    
    server->run = 1;
    /* Run server */
    if (RTCS_task_create("httpsrv server", server->params.server_prio, HTTPSRV_SERVER_STACK_SIZE, httpsrv_server_task, server) != RTCS_OK)
    {
        HTTPSRV_release((uint_32) server);
        return((uint_32) NULL);
    }

    return((uint_32) server);
}

/*
** Function for releasing/stopping HTTP server 
**
** IN:
**      uint_32       server_h - server handle
**
** OUT:
**      none
**
** Return Value: 
**      uint_32      error code. HTTPSRV_OK if everything went right, positive number otherwise
*/
uint_32 HTTPSRV_release(uint_32 server_h)
{
    uint_32 i;
    uint_32 n = 0;
    HTTPSRV_STRUCT* server = (pointer) server_h;
    boolean wait = FALSE;

    /* Shutdown server listen sockets, stop server task */
    if (server != NULL)
    {
        if (server->sock_v4)
        {
            shutdown(server->sock_v4, FLAG_ABORT_CONNECTION);
        }
            
        if (server->sock_v6)
        {
            shutdown(server->sock_v6, FLAG_ABORT_CONNECTION);
        }
    }
    else
    {
        return(RTCS_ERROR);
    }

    /* Invalidate sessions (this is signal for session tasks to end them) */
    while(n < server->params.max_ses)
    {
        if (server->session[n])
        {
            server->session[n]->valid = HTTPSRV_SESSION_INVALID;
        }
        n++;
    }
    /* Wait until all session tasks end */
    do
    {
        wait = FALSE;
        for (n = 0; n < server->params.max_ses; n++)
        {
            if (server->ses_tid[n])
            {
                wait = TRUE;
            }
        }
        _sched_yield();
    }while(wait);

    /* Shutdown script handler if there is any */
    if (server->script_tid)
    {
        HTTPSRV_SCRIPT_MSG* msg_ptr;
        msg_ptr = _msg_alloc(server->script_msg_pool);
        if (msg_ptr != NULL)
        {
            msg_ptr->header.TARGET_QID = server->script_msgq;
            msg_ptr->header.SOURCE_QID = server->script_msgq;
            msg_ptr->header.SIZE = sizeof(HTTPSRV_SCRIPT_MSG);
            msg_ptr->session = NULL;
            msg_ptr->name = NULL;
            _msgq_send(msg_ptr);
        }
    }
    while(server->script_tid)
    {
        _sched_yield();
    }
    
    /* Shutdown server task */
    server->run = 0;
    
    while(server->server_tid)
    {
        _sched_yield();
    }

    _lwsem_destroy(&server->tid_sem);
    
    /* Free memory */
    _mem_free(server->ses_tid);
    server->ses_tid = NULL;
    _mem_free(server->session);
    server->session = NULL;
    _mem_free(server);
    return(RTCS_OK);
}

/*
** Internal function for server parameters initialization
**
** IN:
**      HTTPSRV_STRUCT* server - server structure pointer
**
**      HTTPSRV_PARAM_STRUCT* params - pointer to user parameters if there are any
** OUT:
**      none
**
** Return Value: 
**      uint_32      error code. HTTPSRV_OK if everything went right, positive number otherwise
*/
static uint_32 httpsrv_set_params(HTTPSRV_STRUCT *server, HTTPSRV_PARAM_STRUCT *params)
{
    /* Stack size */
    uint_32 stack;

    server->params.port = HTTPSRVCFG_DEF_PORT;
    #if RTCSCFG_ENABLE_IP4
    server->params.ipv4_address.s_addr = HTTPSRVCFG_DEF_ADDR;
    #endif
    #if RTCSCFG_ENABLE_IP6  
    server->params.ipv6_address = in6addr_any;
    server->params.ipv6_scope_id = 0;
    #endif
    server->params.af = HTTPSRVCFG_AF;
    server->params.max_uri = HTTPSRVCFG_DEF_URL_LEN;
    server->params.max_ses = HTTPSRVCFG_DEF_SES_CNT;
    server->params.root_dir = "tfs:";
    server->params.index_page = HTTPSRVCFG_DEF_INDEX_PAGE;
    server->params.cgi_lnk_tbl = NULL;
    server->params.ssi_lnk_tbl = NULL;
    server->params.server_prio = HTTPSRVCFG_DEF_SERVER_PRIO;
    server->params.script_prio = HTTPSRVCFG_DEF_SERVER_PRIO;
    server->params.script_stack = HTTPSRV_SESSION_STACK_SIZE;
    server->params.auth_table = NULL;
    server->params.use_nagle = 0;
    server->params.alias_tbl = NULL;

    /* If there is parameters structure copy nonzero values to server */
    if (params != NULL)
    {
        if (params->port)
            server->params.port = params->port;
        #if RTCSCFG_ENABLE_IP4
        if (params->ipv4_address.s_addr != 0)
            server->params.ipv4_address = params->ipv4_address;
        #endif
        #if RTCSCFG_ENABLE_IP6
        if (params->ipv6_address.s6_addr != NULL)
            server->params.ipv6_address = params->ipv6_address;
        if (params->ipv6_scope_id)
            server->params.ipv6_scope_id = params->ipv6_scope_id;
        #endif
        if (params->af)
            server->params.af = params->af;
        if (params->max_uri)
            server->params.max_uri = params->max_uri;
        if (params->max_ses)
            server->params.max_ses = params->max_ses;
        if (params->root_dir)
            server->params.root_dir = params->root_dir;
        if (params->index_page)
            server->params.index_page = params->index_page;
        if (params->cgi_lnk_tbl)
            server->params.cgi_lnk_tbl = params->cgi_lnk_tbl;
        if (params->ssi_lnk_tbl)
            server->params.ssi_lnk_tbl = params->ssi_lnk_tbl;
        if (params->server_prio)
            server->params.server_prio = params->server_prio;
        if (params->script_prio)
            server->params.script_prio = params->script_prio;
        if (params->script_stack)
            server->params.script_stack = params->script_stack;
        if (params->auth_table)
            server->params.auth_table = params->auth_table;
        if (params->use_nagle)
            server->params.use_nagle = params->use_nagle;
        if (params->alias_tbl)
            server->params.alias_tbl = params->alias_tbl;
    }

    return(HTTPSRV_OK);
}

/*
** Function for socket initialization (both IPv4 and IPv6)
**
** IN:
**      HTTPSRV_STRUCT* server - server structure pointer
**
**      uint_16      family - IP protocol family
** OUT:
**      none
**
** Return Value: 
**      uint_32      error code. HTTPSRV_OK if everything went right, positive number otherwise
*/
static uint_32 httpsrv_init_socket(HTTPSRV_STRUCT *server, uint_16 family)
{
    uint_32 option;
    uint_32 error;
    sockaddr sin_sock;
    uint_32 sock = 0;

    _mem_zero(&sin_sock, sizeof(sockaddr));

    if (family == AF_INET) /* IPv4 */
    {
        #if RTCSCFG_ENABLE_IP4
        if ((server->sock_v4 = socket(AF_INET, SOCK_STREAM, 0)) == RTCS_ERROR)
        {
            return(HTTPSRV_CREATE_FAIL);
        }
        ((sockaddr_in *)&sin_sock)->sin_port   = server->params.port;
        ((sockaddr_in *)&sin_sock)->sin_addr   = server->params.ipv4_address;
        ((sockaddr_in *)&sin_sock)->sin_family = AF_INET;
        sock = server->sock_v4;
        #else
        return(HTTPSRV_BAD_FAMILY);
        #endif
    }
    else if (family == AF_INET6) /* IPv6 */
    {
        #if RTCSCFG_ENABLE_IP6
        if ((server->sock_v6 = socket(AF_INET6, SOCK_STREAM, 0)) == RTCS_ERROR)
        {
            return(HTTPSRV_CREATE_FAIL);
        }
        ((sockaddr_in6 *)&sin_sock)->sin6_port      = server->params.port;
        ((sockaddr_in6 *)&sin_sock)->sin6_family    = AF_INET6;
        ((sockaddr_in6 *)&sin_sock)->sin6_scope_id  = server->params.ipv6_scope_id;
        ((sockaddr_in6 *)&sin_sock)->sin6_addr      = server->params.ipv6_address;
        sock = server->sock_v6;
        #else
        return(HTTPSRV_BAD_FAMILY);
        #endif
    }
    else
    {
        return(HTTPSRV_BAD_FAMILY);
    }
    /* Set socket options */
    option = HTTPSRVCFG_SEND_TIMEOUT;
    error = setsockopt(sock, SOL_TCP, OPT_SEND_TIMEOUT, &option, sizeof(option));
    option = TRUE;
    error = setsockopt(sock, SOL_TCP, OPT_SEND_NOWAIT, &option, sizeof(option));
    option = HTTPSRVCFG_CONNECT_TIMEOUT;
    error = setsockopt(sock, SOL_TCP, OPT_CONNECT_TIMEOUT, &option, sizeof(option));
    option = HTTPSRVCFG_TIMEWAIT_TIMEOUT;
    error = setsockopt(sock, SOL_TCP, OPT_TIMEWAIT_TIMEOUT, &option, sizeof(option));
    option = HTTPSRVCFG_RECEIVE_TIMEOUT;     
    error = setsockopt(sock, SOL_TCP, OPT_RECEIVE_TIMEOUT, &option, sizeof(option));
    option = FALSE; 
    error = setsockopt(sock, SOL_TCP, OPT_RECEIVE_NOWAIT, &option, sizeof(option));
    option = TRUE;  
    error = setsockopt(sock, SOL_TCP, OPT_RECEIVE_PUSH, &option, sizeof(option));
    option = !server->params.use_nagle;
    error = setsockopt(sock, SOL_TCP, OPT_NO_NAGLE_ALGORITHM, &option, sizeof(option));
    option = HTTPSRVCFG_TX_BUFFER_SIZE;
    error = setsockopt(sock, SOL_TCP, OPT_TBSIZE, &option, sizeof(option));
    option = HTTPSRVCFG_RX_BUFFER_SIZE;
    error = setsockopt(sock, SOL_TCP, OPT_RBSIZE, &option, sizeof(option));

    if (error != RTCS_OK)
    {
        return(HTTPSRV_SOCKOPT_FAIL);
    }

    /* Bind socket */
    error = bind(sock, &sin_sock, sizeof(sin_sock));
    if(error != RTCS_OK)
    {
        return(HTTPSRV_BIND_FAIL);
    }

    /* Listen */
    error = listen(sock, 0);
    if (error != RTCS_OK)
    {
        return(HTTPSRV_LISTEN_FAIL);
    }
    return(HTTPSRV_OK);
}
/* EOF */