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
#include <fio.h>
#include <shell.h>
#include <stdlib.h>
#include <math.h>

#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))

static LWGPIO_STRUCT led[4], btn[2];
static int dio_task_status = 0;
static LWEVENT_STRUCT lwevent;

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

void change_dio_task_state(pointer pstate)
{
    int state = *(int *)pstate;
    switch (state) {
    case 0:
        _lwevent_clear(&lwevent, 0x01);
        dio_task_status = 0;
        break;
    case 1:
        _lwevent_set(&lwevent, 0x01);
        dio_task_status = 1;
        break;
    default:
        if (dio_task_status == 0) {
            _lwevent_set(&lwevent, 0x01);
            dio_task_status = 1;
        } else {
             _lwevent_clear(&lwevent, 0x01);
            dio_task_status = 0;
        }
        break;
    }
}

int_32 Shell_eable_dio(int_32 argc, char_ptr argv[])
{
    boolean      print_usage, shorthelp = FALSE;
    int_32       return_code = SHELL_EXIT_SUCCESS;
	int_32       opt, enable = 0;
    LWGPIO_VALUE led_state = LWGPIO_VALUE_LOW;

    print_usage = Shell_check_help_request(argc, argv, &shorthelp);

    if (!print_usage)
    {
        while (1) {
            opt = Shell_getopt(argc, argv, "e:s:");
            if (-1 == opt) break;

            switch (opt) {
            case 'e':
                enable = atoi(optarg);
                break;
            case 's':
                led_state = (LWGPIO_VALUE)(0 == atoi(optarg) ? 
                                LWGPIO_VALUE_LOW : LWGPIO_VALUE_HIGH);
                break;
            default:
                printf("Usage: %s [-e 1/0]\n", argv[0]);
                return 0;
            }
        }
        
        _task_id dio_taskid = _task_get_id_from_name("dio");
        change_dio_task_state(&enable);
//        if (enable) {
//            // resume thread
//            pointer td_ptr = _task_get_td(dio_taskid);
//            if (!dio_task_status){
//                _task_ready(td_ptr);
//                dio_task_status = 1;
//            }
//        } else {
//            // pause thread            
//            dio_task_status = 0;
//        }
        
        for (int i = 0; i < ARRAY_SIZE(led); i++)
        {
            /* write logical 1 to the pin */
            lwgpio_set_value(&led[i], led_state); /* set pin to 1 */
        }
    }
    
    if (print_usage)
    {
        if (shorthelp)
        {
            printf("%s\n", argv[0]); /* shows in help command */
        }
        else
        {
            printf("Usage: %s [-e 1/0]\n", argv[0]);
            return 0;
        }
    }
    
    return return_code;
}

void dio_task(uint_32 arg)
{
    uint_32 i = 0;
    MQX_TICK_STRUCT cur_tick_time;
    
    cur_tick_time = _mqx_zero_tick_struct;
    
    dio_task_status = 1;
    
    init_dio();
    
    if (MQX_OK != _lwevent_create(&lwevent, LWEVENT_AUTO_CLEAR)) {
        printf("_lwevent_create error\n");
        return;
    }
    _lwevent_set(&lwevent, 0x01);
    
//    lwgpio_set_value(&led[3], LWGPIO_VALUE_LOW);
    
    while (1) {
        if (dio_task_status == 0 ) _lwevent_wait_for(&lwevent, 0x01, 0, 0);
        lwgpio_toggle_value(&led[i]);
        if (4 == ++i) i = 0;
//        lwgpio_toggle_value(&led[i]);
        _time_add_msec_to_ticks(&cur_tick_time, 250);
        _time_delay_until(&cur_tick_time);
//        _time_delay(200);
    }
}
