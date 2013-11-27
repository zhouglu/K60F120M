/**HEADER********************************************************************
* 
* Copyright (c) 2013 Holley Technology;
* All Rights Reserved                       
*
*************************************************************************** 
*
* $FileName: dio_app.c$
* $Author  : Ganglu Zhou (ganglu.zhou@holley.cn)$
* $Version : 0.0.0.1$
* $Date    : Nov-22-2013$
*
* Comments:
*
*   This file is the source that use gpios.
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>

#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))

static LWGPIO_STRUCT led[4], btn[2];

boolean init_dio()
{
    const LWGPIO_PIN_ID ledID[] = { BSP_LED1, BSP_LED2, BSP_LED3, BSP_LED4 };
    const uint_32 ledMUX[] = { BSP_LED1_MUX_GPIO, BSP_LED2_MUX_GPIO, 
                                    BSP_LED3_MUX_GPIO, BSP_LED4_MUX_GPIO };
    const LWGPIO_PIN_ID btnID[] = { BSP_BUTTON1, BSP_BUTTON1 };
    const uint_32 btnMUX[] = { BSP_BUTTON1_MUX_GPIO, BSP_BUTTON2_MUX_GPIO };
    
    
    /* initialize lwgpio handle (led) for BSP_LED1 pin (defined in mqx/source/bsp/<bsp_name>/<bsp_name>.h file) */
    for (int i = 0; i < ARRAY_SIZE(ledID); i++)
    {
        if (!lwgpio_init(&led[i], ledID[i], LWGPIO_DIR_OUTPUT, LWGPIO_VALUE_NOCHANGE))
        {
            printf("Initializing LED1 GPIO as output failed.\n");
            return FALSE;
        }
        
        /* swich pin functionality (MUX) to GPIO mode */
        lwgpio_set_functionality(&led[i], ledMUX[i]);
        
        /* write logical 1 to the pin */
        lwgpio_set_value(&led[i], LWGPIO_VALUE_HIGH); /* set pin to 1 */
    }

    /* initialize lwgpio handle (led) for BSP_LED1 pin (defined in mqx/source/bsp/<bsp_name>/<bsp_name>.h file) */
    for (int i = 0; i < ARRAY_SIZE(btnID); i++)
    {
        /* opening pins for input */
        if (!lwgpio_init(&btn[i], btnID[i], LWGPIO_DIR_INPUT, LWGPIO_VALUE_NOCHANGE))
        {
            printf("Initializing LED1 GPIO as output failed.\n");
            return FALSE;
        }
        
        /* swich pin functionality (MUX) to GPIO mode */
        lwgpio_set_functionality(&btn[i], btnMUX[i]);
        lwgpio_set_attribute(&btn[i], LWGPIO_ATTR_PULL_UP, LWGPIO_AVAL_ENABLE);
    }
    
    return TRUE;
}

void dio_task(uint_32 arg)
{
    uint_32 i = 0;
    MQX_TICK_STRUCT cur_tick_time;
    
    init_dio();
    _time_get_ticks(&cur_tick_time);
    
//    lwgpio_set_value(&led[3], LWGPIO_VALUE_LOW);
    
    while (1) {
        lwgpio_toggle_value(&led[i]);
        if (4 == ++i) i = 0;
//        lwgpio_toggle_value(&led[i]);
//        _time_add_msec_to_ticks(&cur_tick_time, 50);
//        _time_delay_for(&cur_tick_time);
        _time_delay(200);
        
    }
}
