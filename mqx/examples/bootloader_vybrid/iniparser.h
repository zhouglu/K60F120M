/**HEADER********************************************************************
*
* Copyright (c) 2008 Freescale Semiconductor;
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
* $FileName: boot.c$
* $Version : 3.8.11.0$
* $Date    : Oct-3-2012$
*
* Comments:
*
*END************************************************************************/

#ifndef __iniparser_h__
#define __iniparser_h__

#include <psptypes.h>

#define INIPARSER_CHAR_IS_EOS(x) \
(x == '\0')

#define INIPARSER_CHAR_IS_COMMENT(x) \
(x == ';')

#define INIPARSER_CHAR_IS_EOL(x) \
(((x) == 0xa) || ((x) == 0xd))

#define INIPARSER_LENGTH_PROPERTY_NAME  (64)
#define INIPARSER_LENGTH_PROPERTY_VALUE (128)
#define INIPARSER_LENGTH_LABEL_NAME     (64)
#define INIPARSER_LENGTH_LINE           (256)

#if (INIPARSER_LENGTH_LINE <= INIPARSER_LENGTH_PROPERTY_NAME) || \
    (INIPARSER_LENGTH_LINE <= INIPARSER_LENGTH_PROPERTY_VALUE) || \
    (INIPARSER_LENGTH_LINE <= INIPARSER_LENGTH_LABEL_NAME)
#error "INIPARSER_LENGTH_LINE value is too short "
#endif


typedef void (*iniparser_label_fptr)(const char_ptr);
typedef void (*iniparser_property_fptr)(const char_ptr, const char_ptr);


uint_32 iniparser_load(const char_ptr, iniparser_label_fptr, iniparser_property_fptr);
uint_32 iniparser_recognize_comment(const char_ptr);
uint_32 iniparser_recognize_label_name(const char_ptr, const char_ptr);
uint_32 iniparser_recognize_label(const char_ptr, char_ptr, uint_32);
uint_32 iniparser_recognize_property(const char_ptr, char_ptr, uint_32, char_ptr, uint_32);


#endif
