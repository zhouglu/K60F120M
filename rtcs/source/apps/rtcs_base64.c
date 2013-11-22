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
* $FileName: rtcs_base64.c$
* $Version : 4.0.1.0$
* $Date    : Jan-24-2013$
*
* Comments:
*
*   Base64 encode and decode functions.
*
*END************************************************************************/

#include <rtcs_base64.h>
#include <mqx.h>


static void base64_encodeblock(unsigned char in[3], unsigned char out[4], int len);
static void base64_decodeblock(char in[4], char out[3]);
/*
** base64 decoding alphabet
*/
static const char decode_base64[] = "n###odefghijklm#######0123456789:;<=>?@ABCDEFGHI######JKLMNOPQRSTUVWXYZ[\\]^_`abc";

#define DECODE_BASE64_IDX(ch)       (decode_base64[ch - 43] - '0')

/*
** Translation Table as described in RFC1113
*/
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
** base64_encodeblock
**
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
static void base64_encodeblock(unsigned char in[3], unsigned char out[4], int len)
{
    out[0] = cb64[in[0]>>2];
    out[1] = cb64[((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}


char* base64_encode(char *source, char *destination)
{
    unsigned char in[3], out[4];
    int i, n, p, len;
    i = 0;
    n = 0;
    p = 0;

    *destination = '\0';

    while(source[n] != '\0') 
    {
        len = 0;
        for(i = 0; i < 3; i++) 
        {
            in[i] = 0;
            if (source[n] != '\0')
            {
                in[i] = source[n++];
                len++;
            }
        }

        if(len) 
        {
            base64_encodeblock(in, out, len);
            for(i = 0; i < 4; i++) 
            {
                destination[p++]= out[i];
            }
        }
    }

    destination[p] = '\0';
    return destination;
}

/** Decode base64 single block (4 char string).
 * \param in < 4 char input buffer
 * \param out < 3 char output buffer
 */
static void base64_decodeblock(char in[4], char out[3])
{
    out[0] = (unsigned char)(in[0] << 2 | in[1] >> 4);
    out[1] = (unsigned char)(in[1] << 4 | in[2] >> 2);
    out[2] = (unsigned char)(((in[2] << 6) & 0xc0) | in[3]);
}


char* base64_decode(char *dst, char *src, uint_32 dst_size)
{
    char in[4];
    int i, len; 
    uint_32 free_size = dst_size;

    while (*src && free_size)
    {
        for (i = 0, len = 0; i < 4; i++)
        {
            if (*src)
            {
                if (!(*src < 43 || *src > 122) && *src != '=')
                {
                    len++;
                    in[i] = (unsigned char)DECODE_BASE64_IDX(*src);
                }
                else
                {
                    in[i] = 0;
                }
                src++;
            }
            else
            {
                in[i] = 0;
            }
        }

        if ((len > 1) && (free_size > (len - 1)))
        {
            base64_decodeblock(in, dst);
            dst += len - 1;
            *dst = '\0';
            free_size -= (len - 1);
        }
        else
        {
            break;
        }
    }
    return dst;
}