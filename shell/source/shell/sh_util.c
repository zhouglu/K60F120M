/**HEADER********************************************************************
*
* Copyright (c) 2008 Freescale Semiconductor;
* All Rights Reserved
*
* Copyright (c) 2004-2008 Embedded Access Inc.;
* All Rights Reserved
*
* Copyright (c) 1989-2008 ARC International;
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
* $FileName: sh_util.c$
* $Version : 3.8.17.0$
* $Date    : Jun-7-2012$
*
* Comments:
*
*   This file contains the RTCS shell.
*
*END************************************************************************/

// Here's something you've all been waiting for:  the AT&T public domain
// source for getopt(3).  It is the code which was given out at the 1985
// UNIFORUM conference in Dallas.  I obtained it by electronic mail
// directly from AT&T.  The people there assure me that it is indeed
// in the public domain.

// There is no manual page.  That is because the one they gave out at
// UNIFORUM was slightly different from the current System V Release 2
// manual page.  The difference apparently involved a note about the
// famous rules 5 and 6, recommending using white space between an option
// and its first argument, and not grouping options that have arguments.
// Getopt itself is currently lenient about both of these things White
// space is allowed, but not mandatory, and the last option in a group can
// have an argument.  That particular version of the man page evidently
// has no official existence, and my source at AT&T did not send a copy.
// The current SVR2 man page reflects the actual behavor of this getopt.
// However, I am not about to post a copy of anything licensed by AT&T.

#include <ctype.h>
#include <string.h>
#include <mqx.h>
#include <fio.h>
#include <shell.h>
#include <sh_prv.h>

#define ERR(s, c) if(opterr){\
   char errbuf[2];\
   errbuf[0] = c; errbuf[1] = '\n';\
   write(stderr, argv[0], strlen(argv[0]));\
   write(stderr, s, strlen(s));\
   write(stderr, errbuf, 2);}

extern int strcmp();
extern char *strchr();

int   opterr = 1;
int   optind = 1;
int   optopt;
char  *optarg;

#if SHELLCFG_USES_MFS
MQX_FILE_PTR Shell_default_fs = NULL;
char_ptr Shell_default_fs_name = NULL;


static void Shell_init_current_filesystem(pointer argv);
#endif //SHELLCFG_USES_MFS


int_32 Shell_getopt(int argc, char** argv, char* opts)
{
   static int sp = 1;
   register int c;
   register char *cp;

   if(sp == 1)
      if(optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
      {
         optind = 1;
         return(-1);
      }
      else if(strcmp(argv[optind], "--") == 0) {
         optind++;
         return(-1);
      }
   optopt = c = argv[optind][sp];
   if(c == ':' || (cp=strchr(opts, c)) == NULL) {
      ERR(": illegal option -- ", c);
      if(argv[optind][++sp] == '\0') {
         optind++;
         sp = 1;
      }
      return('?');
   }
   if(*++cp == ':') {
      if(argv[optind][sp+1] != '\0')
         optarg = &argv[optind++][sp+1];
      else if(++optind >= argc) {
         ERR(": option requires an argument -- ", c);
         sp = 1;
         return('?');
      } else
         optarg = argv[optind++];
      sp = 1;
   } else {
      if(argv[optind][++sp] == '\0') {
         sp = 1;
         optind++;
      }
      optarg = NULL;
   }
   return(c);
}

int_32 Shell_parse_command_line(char_ptr command_line_ptr, char_ptr argv[])
{
   char_ptr             sptr = command_line_ptr;
   uint_32              i;
   int_32               argc = 0;
   boolean              inquotes = 0;

   for (i = 0; i < SHELL_MAX_ARGS; i++)
   {
      while (*sptr && !isgraph((uchar)*sptr)) sptr++;
      if (!*sptr)
      {
         argv[i] = NULL;
      } 
      else
      {
         char qchar = 0;
         argc++;
         if ((*sptr == '\'') || (*sptr == '\"'))
         {
            inquotes = 1;
            qchar = *sptr;
            sptr++;
         }
         argv[i] = sptr;

         if (!inquotes)
         {
            while (*sptr && isgraph(*sptr)) sptr++;
         }   
         else
         {
            while (*sptr && (*sptr != qchar)) sptr++;
            inquotes = 0;
         }

         if (*sptr)
         {
            *sptr++ = '\0';
         }   
      }
   }
   return argc;
}



boolean Shell_parse_number( char_ptr arg, uint_32_ptr num_ptr)
{
   uint_32 i=0,j=0;

   if (num_ptr == NULL) return FALSE;
   while (isdigit(arg[i]))  {
      j = j*10 + (arg[i++]-'0');
   }
   if (arg[i]=='\0')
   {
      *num_ptr=j;
      return TRUE;
   }
   return FALSE;
}

boolean Shell_parse_uint_32( char_ptr arg, uint_32_ptr num_ptr)
{
   uint_32 i=0,j=0;

   if (num_ptr == NULL) return FALSE;
   while (isdigit(arg[i]))  {
      j = j*10 + (arg[i++]-'0');
   }
   if (arg[i]=='\0')
   {
      *num_ptr=j;
      return TRUE;
   }
   return FALSE;
}

boolean Shell_parse_uint_16( char_ptr arg, uint_16_ptr num_ptr)
{
   uint_32 i=0;
   uint_16 j=0;

   if (num_ptr == NULL) return FALSE;
   while (isdigit(arg[i]))  {
      j = j*10 + (arg[i++]-'0');
   }
   if (arg[i]=='\0')
   {
      *num_ptr=j;
      return TRUE;
   }
   return FALSE;
}

boolean Shell_parse_int_32( char_ptr arg, int_32_ptr num_ptr)
{
   uint_32 i=0;
   int_32 sign=1,j=0;

   if (num_ptr == NULL) return FALSE;
   if (arg[i]=='-')  {
      sign = -1;
      i++;
   }
   while (isdigit(arg[i]))  {
      j = j*10 + (arg[i++]-'0');
   }
   if (arg[i]=='\0')
   {
      *num_ptr=j * sign;
      return TRUE;
   }
   return FALSE;
}

boolean Shell_parse_hexnum( char_ptr arg, uint_32_ptr num_ptr)
{
   uint_32 i=2,j=0;

   if (num_ptr == NULL) return FALSE;

   for (i=0;i<=8;i++) {
      if (isdigit(arg[i])) {
         j = j*16 + (arg[i]-'0');
      } else if ((arg[i] >='a') && (arg[i] <='f') ) {
         j = j*16 + (arg[i]-'a'+10);
      } else if ((arg[i] >='A') && (arg[i] <='F') ) {
         j = j*16 + (arg[i]-'A'+10);
      } else if (arg[i]==0) {
         *num_ptr=j;
         return TRUE;
      } else {
         return FALSE;
      }
   }
   return FALSE;
}


boolean Shell_check_help_request(int_32 argc, char_ptr argv[], boolean _PTR_ short_ptr )
{
   if ((argc == 3) && (strcmp(argv[1], "help") == 0))  {
      *short_ptr = (strcmp(argv[2], "short")==0)?TRUE:FALSE;
      return TRUE;
   }
   return FALSE;
}


#if SHELLCFG_USES_MFS
void Shell_create_prefixed_filename( char_ptr new_ptr, char_ptr in_ptr,  pointer argv)
{
   _io_create_prefixed_filename(new_ptr,in_ptr,Shell_get_current_filesystem_name(argv));
}


MQX_FILE_PTR Shell_get_current_filesystem(pointer argv)
{
   SHELL_CONTEXT_PTR    shell_ptr = Shell_get_context( argv );

   if(shell_ptr->CURRENT_DEVICE_FP == NULL)
   {
      // shell current filesystem is not initialized
      Shell_init_current_filesystem(argv);
      return shell_ptr->CURRENT_DEVICE_FP;
   }
   else if(!_io_is_fs_valid(shell_ptr->CURRENT_DEVICE_FP))
   {
      // current filesystem was closed
      Shell_init_current_filesystem(argv);
      return shell_ptr->CURRENT_DEVICE_FP;
   }
   else
   {
      // everything is ok, return stored filesystem ptr
      return shell_ptr->CURRENT_DEVICE_FP;
   }
}

char_ptr Shell_get_current_filesystem_name(pointer argv)
{
   SHELL_CONTEXT_PTR    shell_ptr = Shell_get_context( argv );

   if(shell_ptr->CURRENT_DEVICE_FP == NULL)
   {
      // shell current filesystem is not initialized
      Shell_init_current_filesystem(argv);
      return shell_ptr->CURRENT_DEVICE_NAME;
   }
   else if(!_io_is_fs_valid(shell_ptr->CURRENT_DEVICE_FP))
   {
      // current filesystem was closed
      Shell_init_current_filesystem(argv);
      return shell_ptr->CURRENT_DEVICE_NAME;
   }
   else
   {
      // everything is ok, return stored device name
      return shell_ptr->CURRENT_DEVICE_NAME;
   }
}

int_32 Shell_set_current_filesystem(pointer argv, MQX_FILE_PTR fp)
{
   SHELL_CONTEXT_PTR    shell_ptr = Shell_get_context( argv );

   if(fp == NULL)
   {
      shell_ptr->CURRENT_DEVICE_FP       = NULL;
      shell_ptr->CURRENT_DEVICE_NAME[0]  = '\0';
      strcpy(shell_ptr->CURRENT_DIR,"\\");
   }
   else if(_io_is_fs_valid(fp))
   {
      _io_get_fs_name(fp,shell_ptr->CURRENT_DEVICE_NAME,sizeof(shell_ptr->CURRENT_DEVICE_NAME));
      shell_ptr->CURRENT_DEVICE_FP = fp;
      strcpy(shell_ptr->CURRENT_DIR,"\\");
   }
   else
   {
      return MQX_INVALID_POINTER;
   }
   return MQX_OK;
}

int_32 Shell_set_current_filesystem_by_name(pointer argv, char_ptr dev_name)
{
   MQX_FILE_PTR         temp_fs;

   temp_fs = _io_get_fs_by_name(dev_name);
   // if valid filesystem entry is returned, fill in internal variables
   if(temp_fs != NULL)
   {
      Shell_set_current_filesystem(argv,temp_fs);
   }
   else
   {
      return MQX_INVALID_POINTER;
   }
   return MQX_OK;
}


static void Shell_init_current_filesystem(pointer argv)
{
   MQX_FILE_PTR         temp_fs;

   temp_fs   = _io_get_first_valid_fs();
   // if valid filesystem entry is returned, fill in internal variables
   // otherwise set default values
   Shell_set_current_filesystem(argv,temp_fs);
}
#endif //SHELLCFG_USES_MFS
/* EOF */
