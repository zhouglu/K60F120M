/**HEADER********************************************************************
*
* Copyright (c) 2010 Freescale Semiconductor;
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
* $FileName: krtc_ext.c$
* $Version : 3.8.2.0$
* $Date    : Oct-7-2011$
*
* Comments:
*   Kinetis RTC driver functions
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>


/*FUNCTION****************************************************************
*
* Function Name    : _rtc_inc_upcounter
* Returned Value   : none
* Comments         :
*    This function increments up-counter register by 1.
*
*END*********************************************************************/

void _rtc_inc_upcounter
(
    void
)
{
#ifdef RTC_MCLR
    /* Increment monotinic counter register by 1*/
    RTC_MCLR = 1;
#endif
}


/*FUNCTION****************************************************************
*
* Function Name    : _rtc_get_upcounter
* Returned Value   : value of up-counter register
* Comments         :
*    This function returns value of up-counter register.
*
*END*********************************************************************/

uint_32 _rtc_get_upcounter
(
    void
)
{
#ifdef RTC_MCLR
    return RTC_MCLR;
#else
    return 0;
#endif

}


/*FUNCTION**********************************************************************
*
* Function Name    : _rtc_tamper_enable
* Returned Value   : _mqx_uint
* Comments         :
*
*END*********************************************************************/

_mqx_int _rtc_tamper_enable
(
    RTC_TAMPER_TYPE enable_mask
)
{
#ifndef RTC_TER
    /* Suppress compiler warning */
    if (enable_mask);

    return MQX_ERROR;
#else
    RTC_TER |= enable_mask;
    return MQX_OK;
#endif
}


/*FUNCTION**********************************************************************
*
* Function Name    : _rtc_tamper_disable
* Returned Value   : _mqx_uint
* Comments         :
*
*END*********************************************************************/

_mqx_int _rtc_tamper_disable
(
    RTC_TAMPER_TYPE disable_mask
)
{
#ifndef RTC_TER
    /* Suppress compiler warning */
    if (disable_mask);

    return MQX_ERROR;
#else
    RTC_TER &= ~disable_mask;
    return MQX_OK;
#endif
}

/*FUNCTION**********************************************************************
*
* Function Name    : _rtc_get_tamper_timestamp
* Returned Value   :
* Comments         :
*
*
*END***************************************************************************/

void _rtc_get_tamper_timestamp
(
    /* [OUT] this parameter gets tamper time */
    RTC_TIME_STRUCT_PTR time
)
{
#ifdef RTC_TSR
    time->seconds = RTC_TSR;
#else
    time->seconds = 0;
#endif
}

/*FUNCTION**********************************************************************
*
* Function Name    : _rtc_get_tamper_status
* Returned Value   : RTC_TAMPER_TYPE
* Comments         :
*
*END*********************************************************************/


RTC_TAMPER_TYPE _rtc_get_tamper_status
(
    void
)
{
#ifdef RTC_TDR
    return (RTC_TAMPER_TYPE)RTC_TDR;
#else
    return (RTC_TAMPER_TYPE)0;
#endif
}

/* EOF */
