/**HEADER********************************************************************
* 
* Copyright (c) 2008-2011 Freescale Semiconductor;
* All Rights Reserved                       
*
* Copyright (c) 2004-2011 Embedded Access Inc.;
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
* $FileName: qpbridge.c$
* $Version : 3.8.1.0$
* $Date    : Oct-5-2011$
*
* Comments:
*
*   This file contains the source functions for functions required to
*   control the PBRIDGE.
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>


/*FUNCTION****************************************************************
*
* Function Name    : _qpbridge_set_reg
* Returned Value   : TRUE if successful, FALSE otherwise
* Comments         :
*    This function sets the appropriate access control for the specified
*    register number. 
*
*END*********************************************************************/

static boolean _qpbridge_set_reg(vuint_32_ptr reg_base, uint_32 reg_num, uint_32 access)
{
    uint_32 index = (reg_num>>3)&0xff;
    uint_32 reg_offset = 7 - (reg_num&0x7); 
    uint_32 shift = reg_offset*4;
    uint_32 value;
    
    value = reg_base[index] & ~(0xf<<shift);
    reg_base[index] = value | (access<<shift);
    return TRUE;
}


/*FUNCTION****************************************************************
*
* Function Name    : _qpbridge_enable_access
* Returned Value   : TRUE if successful, FALSE otherwise
* Comments         :
*    This function sets read/write access for the specified peripheral ID
*
*END*********************************************************************/

boolean _qpbridge_enable_access( uint_32 device)
{
    VQPBRIDGE_REG_STRUCT_PTR pbridge_ptr = _bsp_get_qpbridge_base_address();
    uint_32 core_mask = 1<< _psp_core_num();

    switch  (device & QPBRIDGE_DEVICE_TYPE_MASK) {
        case QPBRIDGE_MPROT_FLAG:
            return _qpbridge_set_reg(&pbridge_ptr->MPROT[0],device,QPBRIDGE_MPROT_MPL | QPBRIDGE_MPROT_MTW | QPBRIDGE_MPROT_MTR);
            break;
            
        case QPBRIDGE_PACR_FLAG:
            return _qpbridge_set_reg(&pbridge_ptr->PACR[0],device,QPBRIDGE_PACR_SP|QPBRIDGE_PACR_TP);
            break;

        case QPBRIDGE_OPACR_FLAG:
            return _qpbridge_set_reg(&pbridge_ptr->OPACR[0],device,QPBRIDGE_PACR_SP|QPBRIDGE_PACR_TP);
            break;
            
        default:
            return FALSE;
    }
}

/*FUNCTION****************************************************************
*
* Function Name    : _qpbridge_init
* Returned Value   : none
* Comments         :
*    This function initializes the PBRIDGE to a default state. Masters
*    are enabled per config list, all peripherals are read-only enabled.
*
*END*********************************************************************/

void _qpbridge_init( const uint_32 config[], uint_32 size)
{
    VQPBRIDGE_REG_STRUCT_PTR pbridge_ptr = _bsp_get_qpbridge_base_address();
    uint_32 core_mask = 1<< _psp_core_num();
    uint_32 i, access;
    
    for (i=0;i<size;i++) {
        if (config[i] & core_mask) {
            access =  QPBRIDGE_MPROT_MPL | QPBRIDGE_MPROT_MTW | QPBRIDGE_MPROT_MTR;
        } else {
            access = QPBRIDGE_MPROT_MTR;
        }
        _qpbridge_set_reg(&pbridge_ptr->MPROT[0],i,access);
    }

    // lock down all peripherals. 
    for (i=1;i<QPBRIDGE_NUM_PACR*8;i++) {
        _qpbridge_set_reg(&pbridge_ptr->PACR[0],i,QPBRIDGE_PACR_SP|QPBRIDGE_PACR_WP|QPBRIDGE_PACR_TP);
    }
    
    for (i=0;i<QPBRIDGE_NUM_OPACR*8;i++) {
        _qpbridge_set_reg(&pbridge_ptr->OPACR[0],i,QPBRIDGE_PACR_SP|QPBRIDGE_PACR_WP|QPBRIDGE_PACR_TP);
    }
}

