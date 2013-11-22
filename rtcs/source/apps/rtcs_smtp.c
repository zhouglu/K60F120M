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
* $FileName: rtcs_smtp.c$
* $Version : 4.0.1.0$
* $Date    : Jan-24-2013$
*
* Comments:
*
*   This file contains an implementation of a SMTP client.
*
*END************************************************************************/

#include <rtcs_smtp.h>
#include <fio.h>
#include <string.h>
#include <stdlib.h>
#include <rtcs_base64.h>

static uint_32 SMTP_send_command (int socket, char_ptr command, char_ptr response, uint_32 max_size);
static uint_32 SMTP_send_string(int socket, char_ptr s);
static uint_32 SMTP_get_response_code(char_ptr response);
static uint_32 SMTP_connect (struct sockaddr* server);
static void SMTP_cleanup(int socket, pointer a, ...);
static char_ptr SMTP_findline(char_ptr s, char_ptr* line_start, uint_32_ptr line_length);

/*
** Function for sending email
** IN:
**      SMTP_PARAM_STRUCT_PTR params - Pointer to structure with all required params set up
**                                      (email envelope, email text etc).
**
** OUT:
**      char_ptr err_string - Pointer to string in which error string should be saved (can be NULL -
**                            error string is thrown away in that case).
**
** Return value:
**      _mqx_int - Error code or RTCS_SMTP_OK.
*/
_mqx_int SMTP_send_email (SMTP_PARAM_STRUCT_PTR params, char_ptr err_string, uint_32 err_string_size)
{
    char_ptr response = NULL;
    char_ptr command = NULL;
    char_ptr location = NULL;
    int_32 retval = 0;
    uint_32 code = 0;
    int socket = 0;

    /* Check params and envelope content for NULL */
    if ((params == NULL) || (params->envelope.from == NULL) || (params->envelope.to == NULL))
    {
        return(SMTP_ERR_BAD_PARAM);
    }

    /* Allocate buffers */
    response = (char_ptr) _mem_alloc_system(sizeof(char)*SMTP_RESPONSE_BUFFER_SIZE);
    if (response == NULL)
    {
        return(MQX_OUT_OF_MEMORY);
    }
    command = (char_ptr) _mem_alloc_system(sizeof(char)*SMTP_COMMAND_BUFFER_SIZE);
    if (command == NULL)
    {
        SMTP_cleanup(socket, response, NULL);
        return(MQX_OUT_OF_MEMORY);
    }
    
    /* Connect to server */
    socket = SMTP_connect(&params->server);
    if (socket == 0)
    {
        SMTP_cleanup(socket, response, command, NULL);
        return(SMTP_ERR_CONN_FAILED);
    }
    /* Read greeting message */
    retval = recv(socket, response, SMTP_RESPONSE_BUFFER_SIZE, 0);
    /* Get response code */
    code = SMTP_get_response_code(response);
    if (code > 299)
    {
        SET_ERR_STR(err_string, response, err_string_size);
        SMTP_cleanup(socket, response, command, NULL);
        return(SMTP_WRONG_RESPONSE);
    }
    /* Get server extensions */
    sprintf(command, "EHLO FreescaleTower");
    code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
    /* If server does not support EHLO, try HELO */
    if (code > 399)
    {
        sprintf(command, "HELO FreescaleTower");
        code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
        if (code != 399)
        {
            SET_ERR_STR(err_string, response, err_string_size);
            SMTP_cleanup(socket, response, command, NULL);
            return(SMTP_WRONG_RESPONSE);
        }
    }
    /* Try to determine if authentication is supported, authenticate if needed */

    location = strstr(response, "AUTH");
    if ((location != NULL) && strstr(location, "LOGIN") && (params->login != NULL))
    {
        char_ptr b64_data = NULL;
        uint_32 b64_length = 0;
        /* Send AUTH command */
        sprintf(command, "AUTH LOGIN");
        code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
        if ((code > 399) || (code == 0))
        {
            SET_ERR_STR(err_string, response, err_string_size);
            SMTP_cleanup(socket, response, command, NULL);
            return(SMTP_WRONG_RESPONSE);
        }
        /* Send base64 encoded username */
        b64_length = (strlen(params->login) / 3) * 4 + ((strlen(params->login) % 3) ? (1) : (0)) + 1;
        b64_data = (char_ptr) _mem_alloc_system(sizeof(char)*b64_length);
        if (b64_data == NULL)
        {
            SMTP_cleanup(socket, response, command, NULL);
            return(MQX_OUT_OF_MEMORY);
        }
        sprintf(command, "%s", base64_encode(params->login, b64_data));
        _mem_free(b64_data);
        code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
        if ((code > 399) || (code == 0))
        {
            SET_ERR_STR(err_string, response, err_string_size);
            SMTP_cleanup(socket, response, command, NULL);
            return(SMTP_WRONG_RESPONSE);
        }
        /* Send base64 encoded password */
        b64_length = (strlen(params->login) / 3) * 4 + ((strlen(params->pass) % 3) ? (1) : (0)) + 1;
        b64_data = (char_ptr) _mem_alloc_system(sizeof(char)*b64_length);
        if (b64_data == NULL)
        {
            SMTP_cleanup(socket, response, command, NULL);
            return(MQX_OUT_OF_MEMORY);
        }
        sprintf(command, "%s", base64_encode(params->pass, b64_data));
        _mem_free(b64_data);
        code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
        if ((code > 299) || (code == 0))
        {
            SET_ERR_STR(err_string, response, err_string_size);
            SMTP_cleanup(socket, response, command, NULL);
            return(SMTP_WRONG_RESPONSE);
        }
    }
    /* Send Email */
    sprintf(command, "MAIL FROM:<%s>", params->envelope.from);
    code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
    if ((code > 299) || (code == 0))
    {
        SET_ERR_STR(err_string, response, err_string_size);
        SMTP_cleanup(socket, response, command, NULL);
        return(SMTP_WRONG_RESPONSE);
    }
    sprintf(command, "RCPT TO:<%s>", params->envelope.to);
    code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
    /* Mail receiver not OK nor server will forward */
    if ((code > 299) || (code == 0))
    {
        SET_ERR_STR(err_string, response, err_string_size);
        SMTP_cleanup(socket, response, command, NULL);
        return(SMTP_WRONG_RESPONSE);
    }
    /* Send message data */
    sprintf(command, "DATA");
    code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
    if ((code > 399) || (code == 0))
    {
        SET_ERR_STR(err_string, response, err_string_size);
        SMTP_cleanup(socket, response, command, NULL);
        return(SMTP_WRONG_RESPONSE);
    }
    /* Send email text */
    code = SMTP_send_string(socket, params->text);
    /* Send terminating sequence for DATA command */
    code = SMTP_send_command(socket, "\r\n.", response, SMTP_RESPONSE_BUFFER_SIZE);
    if ((code > 299) || (code == 0))
    {
        SET_ERR_STR(err_string, response, err_string_size);
        SMTP_cleanup(socket, response, command, NULL);
        return(SMTP_WRONG_RESPONSE);
    }
    /* Write response to user buffer */
    SET_ERR_STR(err_string, response, err_string_size);
    /* Disconnect from server */
    sprintf(command, "QUIT");
    code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
    /* Cleanup */
    SMTP_cleanup(socket, response, command, NULL);
    return(SMTP_OK);
}

/*
** Function for reading numeric server response to command
** IN:
**      char* response - response string from server. 
**
** OUT:
**      none
**
** Return value:
**      uint32 - numeric response code if valid, zero otherwise
*/
static uint_32 SMTP_get_response_code(char_ptr response)
{
    char code_str[] = "000";
    if (response != NULL)
    {
        strncpy(code_str, response, strlen(code_str));
    }
    return (strtol(code_str, NULL, 10));
}

/*
** Function for sending string to SMTP server
** IN:
**      int socket - socket used for communication with server.
**      char* s- string to send. 
**
** OUT:
**      none
**
** Return value:
**      uint32 - number of bytes send
*/
static uint_32 SMTP_send_string(int socket, char_ptr s)
{
    uint_32 send_total = 0;
    char_ptr line = NULL;
    uint_32 line_length = 0;
    char_ptr last_loc = s;
    uint_32  last_length = 0;
    uint_32 i;
    int dot = '.';
    
    if (s == NULL) return(0);
      
    /* Send all lines of text */
    while (SMTP_findline(s, &line, &line_length))
    {
        /* If first character is dot, send another dot to ensure email transparency */
        /* See RFC 5321 section 4.5.2 for details why this must be done */
        if (line[0] == '.')
        {
            send(socket, &dot, 1, 0);
        }
        send_total += send(socket, line, line_length, 0);
        last_loc = line;
        last_length = line_length;
    }
    /* Send rest which might not end with \n\r sequence */
    if (send_total < strlen(s))
    {
         send_total += send(socket, last_loc + last_length, strlen(s) - send_total, 0);
    }
    return(send_total);
}

/*
** Function for sending single command to SMTP server
** IN:
**      int socket - socket used for communication with server.
**      char* command - command to send. 
**      char* response - response string from server
**      uint_32 max_size - size of response buffer
**
** OUT:
**      char_ptr* - pointer to string in which full server response will be saved (can be NULL).
**
** Return value:
**      uint32 - numeric response value
*/
static uint_32 SMTP_send_command (int socket, char_ptr command, char_ptr response, uint_32 max_size)
{
    char_ptr out_string;
    uint_32 rec_len = 0;

    if ((response == NULL) || (command == NULL))
    {
        return(0);
    }
    /* Allocate buffer for output text */
    out_string = (char_ptr) _mem_alloc_system_zero(strlen(command)+3);
    if (out_string == NULL)
    {
        return(0);
    }
    /* Add terminating sequence and send command to server */
    sprintf(out_string, "%s\r\n", command);
    send(socket, out_string, strlen(out_string), 0);
    /* Read response */
    rec_len = recv(socket, response, max_size, 0);
    response[rec_len] = '\0';
    /* Cleanup and return */
    _mem_free(out_string);
    return(SMTP_get_response_code(response));
}

/*
** Function for connecting to to SMTP server.
** IN:
**      char_ptr server - server to connect to. 
**
** OUT:
**      none
**
** Return value:
**      uint32 - socket created and connected to server on port 25 or zero.
*/

static uint_32 SMTP_connect (struct sockaddr* server)
{
    int_32 retval = 0;
    uint_32 sfd = 0;
    uint_32 option;

    /* Create socket */
    sfd = socket(server->sa_family, SOCK_STREAM, 0);
    if (sfd == RTCS_SOCKET_ERROR)
    {
        return(0);
    }
    /* Set port for connection */
    switch(server->sa_family)
    {
        case AF_INET:
        ((struct sockaddr_in*) server)->sin_port = RTCS_SMTP_PORT;
        break;
        case AF_INET6:
        ((struct sockaddr_in6*) server)->sin6_port = RTCS_SMTP_PORT;
        break;
    }
    /* Connect socket */
    retval = connect(sfd, server, sizeof(*server));
    if (retval != RTCS_OK)
    {
        fprintf(stderr, "SMTPClient - Connection failed. Error: 0x%X\n", retval);
        shutdown(sfd, FLAG_ABORT_CONNECTION);
        return(0);
    }
    return(sfd);
}

/*
** Function for SMTP cleanup - free memory, close sockets etc.
** IN:
**      int socket - Socket to shutdown.
**      pointer a ... - Pointers to unallocate.
**
** OUT:
**      none
**
** Return value:
**      None
*/

static void SMTP_cleanup(int socket, pointer a, ...)
{
    va_list ap;
    /* Close socket */
    if (socket != 0)
    {
        shutdown(socket, FLAG_CLOSE_TX);
    }
    
    /* Free pointers */
    va_start(ap, a);
    while(a != NULL)
    {
        _mem_free(a);
        a = va_arg(ap, pointer);
    }
    va_end(ap);
}

/*
** Function for line searching lines in strings. After each call pointer to next line start
** is returned. When no next line can be found NULL is returned.
**
** IN:
**      char_ptr s - email text to search. 
**
** OUT:
**      char_ptr line_start - pointer to start of line 
**      uint_32_ptr line_length - pointer to variable i which length of line should be saved
**
** Return value:
**      char_ptr - pointer to start of line
*/

static char_ptr SMTP_findline(char_ptr s, char_ptr* line_start, uint_32_ptr line_length)
{
    static char_ptr last_start;
    static char_ptr last_end;
    static uint_32 first;
    char_ptr line_end;
    /* Check parameters */
    if (line_length == NULL)
    {
        return(NULL);
    }
    /* First run on string */
    if (!first)
    {
        first = TRUE;
        last_start = s;
        last_end = s;
        *line_start = s;
    }
    else
    {
        *line_start = last_end;
    }
    /* Find line end */
    line_end = strstr(*line_start, "\n\r");
    /* If end of string is reached */
    if (line_end == NULL) 
    {
        *line_start = NULL;
        *line_length = 0;
        first = FALSE;
    }
    else
    {
        line_end += 2;
        *line_length = line_end - *line_start;
    }
    /* Update line ending position */
    last_end = line_end;
    return(*line_start);
}
