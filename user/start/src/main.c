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


#if ! BSPCFG_ENABLE_IO_SUBSYSTEM
#error This application requires BSPCFG_ENABLE_IO_SUBSYSTEM defined non-zero in user_config.h. Please recompile BSP with this option.
#endif


#ifndef BSP_DEFAULT_IO_CHANNEL_DEFINED
#error This application requires BSP_DEFAULT_IO_CHANNEL to be not NULL. Please set corresponding BSPCFG_ENABLE_TTYx to non-zero in user_config.h and recompile BSP with this option.
#endif


/* Task IDs */
#define MAIN_TASK 5

extern void main_task(uint_32);
extern void dio_task(uint_32 arg);
extern int_32 CalcEnergy(int_32 argc, char_ptr argv[]);

const TASK_TEMPLATE_STRUCT  MQX_template_list[] = 
{ 
    /* Task Index,   Function,   Stack,  Priority, Name,     Attributes,          Param, Time Slice */
    { MAIN_TASK,   main_task, 8000,   8,        "main",  MQX_AUTO_START_TASK, 0,     0 },
    { MAIN_TASK + 1,   dio_task, 1000,   7,        "dio",  MQX_AUTO_START_TASK, 0,     0 },
    { 0 }
};

/*TASK*-----------------------------------------------------
* 
* Task Name    : main_task
* Comments     :
*    This task prints " Hello World "
*
*END*-----------------------------------------------------*/
void main_task
    (
        uint_32 initial_data
    )
{
    int i = 0;
    printf("Hello World\n"); 
    while (1) {
        Shell(Shell_commands, NULL);
        printf("\r\ncall delay %d times", i++);
        _time_delay(1000);
    }
//    _task_block();
}

const SHELL_COMMAND_STRUCT Shell_commands[] = {
    { "calcenergy", CalcEnergy },
    { NULL, NULL }
};

/* EOF */
