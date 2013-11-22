/**HEADER********************************************************************
* 
* Copyright (c) 2013 Freescale Semiconductor;
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
* $FileName: sh_smtp.c$
* $Version : 4.0.1.0$
* $Date    : Jan-24-2013$
*
* Comments:
*
*   This file contains the RTCS shell SMTP command.
*
*END************************************************************************/

#include <string.h>
#include <mqx.h>
#include "shell.h"
#if SHELLCFG_USES_RTCS
#include <rtcs.h>
#include <rtcs_smtp.h>
#include <stdlib.h>

#define DATE_LENGTH 150
#define ERR_MSG_BUFF_SIZE 515

const char_ptr wday[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const char_ptr months[] =  
{
   "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" 
};

void print_usage(FILE_PTR outstream, uint_32 short_help);

int_32 Shell_smtp (int_32 argc, char_ptr argv[])
{
    struct addrinfo           hints;
    struct addrinfo           *result, *rp;
    int_32                    retval = 0;
    uint_32                   sfd = 0;
    int_32                    err_code = 0;
    boolean                   print_help;
    boolean                   shorthelp = FALSE;
    SMTP_PARAM_STRUCT         params;
    char_ptr                  errstr = NULL;
    char_ptr                  server = NULL;
    char_ptr                  optstring = ":f:t:s:u:p:m:h";
    int_32                    next_option;
    char_ptr                  email_text = NULL;
    uint_32                   email_size = 0;
    MQX_TICK_STRUCT           ticks;
    MQX_XDATE_STRUCT          xdate;
    char                      date_str[DATE_LENGTH];
    
    params.login = NULL;
    params.pass = NULL;

    print_help = Shell_check_help_request(argc, argv, &shorthelp);
    
    if (print_help)
    {
        if (!shorthelp)
        {
            fprintf(stdout,"Usage:"); 
        }
        fprintf(stdout, "%s", argv[0]);
        print_usage(stdout, shorthelp);
        err_code = SHELL_EXIT_SUCCESS;
        return(err_code);
    }
    
    /* Parse command line options */
    do
    {
        next_option = Shell_getopt(argc, argv, optstring);
        switch (next_option)
        {
            case 'f':
                params.envelope.from = optarg;
                break;
            case 't':
                params.envelope.to = optarg;
                break;
            case 'm':
                params.text = optarg;
                break;
            case 's':
                server = optarg;
                break;
            case 'u':
                params.login = optarg;
                break;
            case 'p':
                params.pass = optarg;
                break;
            case 'h':
                print_usage (stdout, FALSE);
                return(SHELL_EXIT_SUCCESS);
            case '?': /* The user specified an invalid option. */
                print_usage(stderr, TRUE);
                return(SHELL_EXIT_ERROR);
            case ':': /* Option has a missing parameter. */
                printf("Option -%c requires a parameter.\n", next_option);
                return(SHELL_EXIT_ERROR);
            case -1: /* Done with options. */
                break;
            default:
                break;
        }
    }while(next_option != -1);
    
    
    _time_get_ticks(&ticks);
    _time_ticks_to_xdate(&ticks,&xdate);    

    snprintf(date_str, DATE_LENGTH, "%s, %d %s %d %02d:%02d:%02d -0000",
        wday[xdate.WDAY],xdate.MDAY, months[xdate.MONTH-1],xdate.YEAR, xdate.HOUR, xdate.MIN, xdate.SEC);
    /* Evaluate email size */
    email_size = strlen(params.envelope.from) + 
                 strlen(params.envelope.to) +
                 strlen(params.text) +
                 strlen(date_str) +
                 strlen("From: <>\r\n") +
                 strlen("To: <>\r\n") + 
                 strlen("Subject: Freescale Tower Email\r\n") +
                 strlen("Date: \r\n\r\n") +   
                 strlen("\r\n") + 1;
    /* Allocate space */
    email_text = (char_ptr) _mem_alloc_zero(email_size);
    if (email_text == NULL)
    {
        fprintf(stderr, "  Unable to allocate memory for email message.\n");
        err_code = SHELL_EXIT_ERROR;
        return(err_code);
    }
    /* Prepare email message */
    snprintf(email_text, email_size, "From: <%s>\r\n"
                                     "To: <%s>\r\n"
                                     "Subject: Freescale Tower Email\r\n"
                                     "Date: %s\r\n\r\n"
                                     "%s\r\n",
                                     params.envelope.from,
                                     params.envelope.to,
                                     date_str,
                                     params.text);
    params.text = email_text;
    
    _mem_zero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; /* TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* For wildcard IP address */
    hints.ai_protocol = 0;           /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    retval = getaddrinfo(server, NULL, &hints, &result);
    if (retval != 0)
    {
        fprintf(stderr, "  getaddrinfo failed. Error: 0x%X\n", retval);
        err_code = SHELL_EXIT_ERROR;
        return(err_code);
    }
    /* Allocate buffer for error message */
    errstr = (char_ptr) _mem_alloc_system_zero(ERR_MSG_BUFF_SIZE);
    /* Try to send email using one of addresses. If it fails try another one. */
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        _mem_copy(rp->ai_addr, &params.server, sizeof(params.server));
        /* Try to send email */
        retval = SMTP_send_email(&params, errstr, ERR_MSG_BUFF_SIZE);
        /* If connection failed try another address */
        if (retval != SMTP_ERR_CONN_FAILED)
        {
            break;
        }
    }
    /* No address succeeded */
    if (rp == NULL)
    {
        fprintf(stderr, "  Unable to connect to %s.\n", server);
        err_code = SHELL_EXIT_ERROR;
    }

    if (retval != SMTP_OK)
    {
        printf("  Email sending failed%s %s\n", (strlen(errstr) > 0) ? ":":".", errstr);
        err_code = SHELL_EXIT_ERROR;
    }
    else
    {
        printf("  Email send. Server response: %s", errstr);
    }
    /* Cleanup */
    freeaddrinfo(result);
    _mem_free(errstr);
    _mem_free(email_text);
    return(err_code);
} 

void print_usage(FILE_PTR outstream, uint_32 short_help)
{
    fprintf (outstream, " -f <sender@email.com> -t <recipient@email.com> -s www.mail.server.com [-u Username] [-p Password] -m \"text of email message\"\n");
    if (!short_help)
    {
        fprintf (outstream,
        " -f - Sender email address.\n"
        " -t - Recipient email address.\n"
        " -s - Email server that should be used for sending of email.\n"
        " -u - Username for authentication.\n"
        " -p - Password for authentication.\n"
        " -m - Email text.\n"
        " -h - Display this usage information.\n");
    }
    return;
}

#endif
