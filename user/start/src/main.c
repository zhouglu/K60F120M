/**HEADER********************************************************************
* 
* Copyright (c) 2013 Holley Technology;
* All Rights Reserved                       
*
*************************************************************************** 
*
* $FileName: main.c$
* $Author  : Ganglu Zhou (ganglu.zhou@holley.cn)$
* $Version : 0.0.0.1$
* $Date    : Nov-21-2013$
*
* Comments:
*
*   This file contains the source for the usage of MQX libraries.
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h> 
#include <fio.h>
#include <shell.h>
#include <stdlib.h>

/* Task IDs */
#define MAIN_TASK 5

#if ! BSPCFG_ENABLE_IO_SUBSYSTEM
#error This application requires BSPCFG_ENABLE_IO_SUBSYSTEM defined non-zero in user_config.h. Please recompile BSP with this option.
#endif


#ifndef BSP_DEFAULT_IO_CHANNEL_DEFINED
#error This application requires BSP_DEFAULT_IO_CHANNEL to be not NULL. Please set corresponding BSPCFG_ENABLE_TTYx to non-zero in user_config.h and recompile BSP with this option.
#endif


void Shell_task(uint_32);
extern void hmi_task(uint_32 arg);
extern void dio_task(uint_32 arg);

/* function prototypes */
extern int_32 Shell_set_module(int_32, char_ptr []);
extern int_32 Shell_get_module(int_32, char_ptr []);
extern int_32 Shell_calc_energy(int_32, char_ptr []);
extern int_32 Shell_eable_dio(int_32, char_ptr []);

const TASK_TEMPLATE_STRUCT  MQX_template_list[] = 
{ 
    /* Task Index,   Function,   Stack,  Priority, Name,     Attributes,          Param, Time Slice */
    { MAIN_TASK,   Shell_task, 9000,   12,        "shell",  MQX_AUTO_START_TASK, 0,     0 },
    { MAIN_TASK + 1,   hmi_task, 1500,   9,        "hmi",  MQX_AUTO_START_TASK, 0,     0 },
    { MAIN_TASK + 1,   dio_task, 1500,   9,        "dio",  MQX_AUTO_START_TASK, 0,     0 },
   { 0 }
};

const SHELL_COMMAND_STRUCT Shell_commands[] = {
        {"module", Shell_set_module},
        {"get", Shell_get_module},
        {"calcenergy", Shell_calc_energy},
        {"enabledio", Shell_eable_dio},
        {"exit", Shell_exit},
        {"help", Shell_help},
        {NULL, NULL}
};

/*TASK*-----------------------------------------------------
*
* Task Name    : Shell_task
* Comments     :
*   This task starts shell and polls it regulary.
*
*END*-----------------------------------------------------*/

void Shell_task(uint_32 initial_data)
{
    while (1)
    {
        Shell(Shell_commands,NULL);    /* start shell */
    }
}

/* EOF */
