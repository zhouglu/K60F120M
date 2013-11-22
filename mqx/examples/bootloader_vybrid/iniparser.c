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

#include <mqx.h>
#include <fio.h>
#include "iniparser.h"

uint_32 iniparser_load(
    /* [IN] '\0' terminated line */
    const char_ptr filename,
    /* [IN], 'label' expression callback */
    iniparser_label_fptr label_call,
    /* [IN], 'property' expression callback */
    iniparser_property_fptr property_call
)
{
    MQX_FILE_PTR file_handle = NULL;
    char line[INIPARSER_LENGTH_LINE];
    char label_name[INIPARSER_LENGTH_LABEL_NAME];
    char property_value[INIPARSER_LENGTH_PROPERTY_VALUE];
    char property_name[INIPARSER_LENGTH_PROPERTY_NAME];
    uint_32 recognized_count;
    
    file_handle = fopen (filename, "r+");
    if (NULL == file_handle)
    { 
        return ~(MQX_OK);
    }
    
    /* read file by lines */
    while (NULL != fgets(line, INIPARSER_LENGTH_LINE, file_handle))
    {
        /* recognize comment */
        recognized_count = iniparser_recognize_comment(line);
        if (recognized_count)
        {
            //if (NULL != comment_call) (*comment_call)();
            continue;
        }
        
        /* recognize label */
        recognized_count = iniparser_recognize_label(
            line, label_name, INIPARSER_LENGTH_LABEL_NAME
        );
        if (recognized_count)
        {
            if (NULL != label_call)
            {
                (*label_call)(label_name);
            }
            continue;
        }
        
        /* recognize property */
        recognized_count = iniparser_recognize_property(
            line, 
            property_name, INIPARSER_LENGTH_PROPERTY_NAME, 
            property_value, INIPARSER_LENGTH_PROPERTY_VALUE
        );
        if (recognized_count)
        {
            if (NULL != property_call)
            {
                (*property_call)(property_name, property_value);
            }
            continue;
        }
    }
    fclose(file_handle);
    return MQX_OK;
}


uint_32 iniparser_recognize_comment
(
    /* [IN] parsed line */ 
    const char_ptr line
)
{
    uint_32 i = 0;
    
    /* check leading comment */
    if (!(INIPARSER_CHAR_IS_COMMENT(line[i]))) return 0;
    
    /* skip rest of line */
    for (i = 1; !(INIPARSER_CHAR_IS_EOS(line[i])); i++);
    return i;
}


uint_32 iniparser_recognize_label
(
    /* [IN] parsed line */ 
    const char_ptr line,
    /* [OUT] label name */ 
    char_ptr label,
    /* [IN] label length*/
    uint_32 label_length
)
{
    uint_32 i = 0, j = 0;
    
    /* check leading '[' */
    if (line[i] != '[') return 0;
    
    /* compare label name unil 'label' reach EOS */
    for (i = 1, j = 0; line[i] != ']'; i++, j++)
    {
        /* end of parsed line */
        if (INIPARSER_CHAR_IS_EOS(line[i])) return 0;
        
        /* 'value' length too small to fit parsed value */
        if ((j + 1) >= label_length) return 0;
        
        /* copy char */
        label[j] = line[i];
    }
    
    /* terminate label */
    label[j] = '\0';
    
    /* check trailing ']' */
    if (line[i] != ']') return 0;
    
    /* all passed, update readed number */
    return (i + 1);
}


// return number of recognized chars
uint_32 iniparser_recognize_property
(
    /* [IN] parsed line */
    const char_ptr line,
    /* [OUT] parsed line */
    char_ptr name,
    /* [IN] value length */
    uint_32 name_length,
    /* [OUT] value length */
    char_ptr value,
    /* [IN] value length */
    uint_32 value_length
)
{
    uint_32 i = 0, j = 0;
    
    /* take property name */
    for (i = 0, j = 0; line[i] != '='; i++, j++)
    {
        /* line ends */
        if (INIPARSER_CHAR_IS_EOS(line[i])) return 0;
        /* not enought space to store name */
        if ((j + 1) >= name_length) return 0;
        /* copy char */
        name[j] = line[i];
    }
    
    /* terminate name */
    name[j] = '\0';
    i += 1;
    
    /* take value (rest of the line after property) */
    for (j = 0; ((j + 1) < value_length); i++, j++)
    {
        /* value end by EOS or EOL, break loop */
        if (INIPARSER_CHAR_IS_EOS(line[i]) || INIPARSER_CHAR_IS_EOL(line[i])) break;
        /* copy char */
        value[j] = line[i];
    }
    
    /* terminate value */
    value[j] = '\0';
    return i;
}
