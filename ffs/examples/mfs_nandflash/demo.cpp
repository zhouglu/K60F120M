/*HEADER**********************************************************************
 *
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * Freescale Confidential and Proprietary - use of this software is
 * governed by the Freescale MQX RTOS License distributed with this
 * material. See the MQX_RTOS_LICENSE file distributed for more
 * details.
 *
 *****************************************************************************
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
 *****************************************************************************
 *
 * $FileName: demo.c$
 * $Version : 3.7.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments: Provide MFS file system on internal NAND Flash memory.
 *
 *END************************************************************************/


#include <mqx.h>
#include <bsp.h>
#include <fio.h>
#include <mfs.h>
#include <shell.h>
#include <part_mgr.h>
#include "nandflash_wl_ffs.h"

#if ! SHELLCFG_USES_MFS
#error This application requires SHELLCFG_USES_MFS defined non-zero in user_config.h. Please recompile libraries with this option.
#endif

extern const NANDFLASH_WL_INIT_STRUCT _bsp_nandflash_wl_init;

void shell_task(uint_32);

int_32 shell_fs_open(int_32 argc, char_ptr argv[] );
int_32 shell_fs_close(int_32 argc, char_ptr argv[] );
int_32 shell_nand_erase(int_32 argc, char_ptr argv[] );
int_32 shell_nand_erase_chip(int_32 argc, char_ptr argv[] );
int_32 shell_nand_repair(int_32 argc, char_ptr argv[] );
int_32 mfs_nandflash_wl_open(void);
int_32 mfs_nandflash_wl_close(void);

void nandflash_erase(void);
void nandflash_erase_chip(void);
void nandflash_repair(void);

TASK_TEMPLATE_STRUCT  MQX_template_list[] =
{
    /* Task Index,  Function,       Stack,  Priority,   Name,       Attributes,             Param,  Time Slice */
    { 1,            shell_task,     10000,  12,         "Shell",    MQX_AUTO_START_TASK,    0,      0 },
    { 0 }
};

const SHELL_COMMAND_STRUCT Shell_commands[] = {
    { "cd",             Shell_cd },
    { "copy",           Shell_copy },
    { "create",         Shell_create },
    { "del",            Shell_del },
    { "disect",         Shell_disect},
    { "dir",            Shell_dir },
    { "di",             Shell_di   },
    { "exit",           Shell_exit },
    { "format",         Shell_format },
    { "help",           Shell_help },
    { "mkdir",          Shell_mkdir },
    { "pwd",            Shell_pwd },
    { "read",           Shell_read },
    { "ren",            Shell_rename },
    { "rmdir",          Shell_rmdir },
    { "sh",             Shell_sh },
    { "type",           Shell_type },
    { "write",          Shell_write },
    { "fsopen",         shell_fs_open },
    { "fsclose",        shell_fs_close },
    { "nanderase",      shell_nand_erase },
    { "nandrepair",     shell_nand_repair },
    { "nanderasechip",  shell_nand_erase_chip },
    { "?",              Shell_command_list },
    { NULL,             NULL }
};

boolean inserted = TRUE, readonly = FALSE, last = FALSE;
_mqx_int error_code;
_mqx_uint param;
MQX_FILE_PTR com_handle, nandflash_handle, filesystem_handle, partman_handle;
char filesystem_name[] = "a:";
char partman_name[] = "pm:";

#define FFS_DEVICE "nandflash_wl:"

/*TASK*-----------------------------------------------------------------
*
* Function Name  : nandflash_erase
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/
void nandflash_erase()
{
    if (nandflash_handle == NULL)
    {
        nandflash_handle = fopen(FFS_DEVICE, NANDWL_OPEN_FORCE_REPAIR_WHEN_FAILED);
    }

    printf("\nErasing nand drive ... \n");

    if (MQX_OK != ioctl(nandflash_handle, NANDFLASH_IOCTL_ERASE, (void*)0))
    {
        printf("\nSome errors occured. Please close the device and try again.\n");
        return;
    }
    else
    {
        printf("\nNand drive was erased completely. Please run format command.\n");
        return;
    }
}


/*TASK*-----------------------------------------------------------------
*
* Function Name  : nandflash_erase_chip
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/
void nandflash_erase_chip()
{
    int i;

    if (nandflash_handle != NULL)
    {
        printf("\n Filesystem must be closed before erasing entire chip \n");
        return;
    }

    nandflash_handle = fopen(FFS_DEVICE, NANDWL_OPEN_IGNORE_INIT_MEDIA);

    printf("\nErasing entire chip ... \n");
    for (i = NANDFLASH_1ST_DATA_DRIVE_START_BLOCK; i < NANDFLASH_1ST_DATA_DRIVE_START_BLOCK + NANDFLASH_1ST_DATA_DRIVE_SIZE_IN_BLOCK; i++)
    {
        if (MQX_OK != ioctl(nandflash_handle, NANDFLASH_IOCTL_ERASE_BLOCK_FORCE, (void*) i))
        {
            printf("\nerase block %d FAILED", i);
        }
    }

    fclose(nandflash_handle);
    nandflash_handle = NULL;
    printf("\nPlease use fsopen command to re-open NAND flash device.\n");

    return;
}

/*TASK*-----------------------------------------------------------------
*
* Function Name  : nandflash_repair
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/
void nandflash_repair()
{
    if (nandflash_handle != NULL)
    {
        fclose(nandflash_handle);
        nandflash_handle = NULL;
    }
    printf("\nRepairing...\n");

    /* Open nand flash wl in force_repair mode */
    nandflash_handle = fopen(FFS_DEVICE, NANDWL_OPEN_FORCE_REPAIR_WHEN_FAILED);

    if (nandflash_handle != NULL)
    {
        printf("\nRepair successfully\n");
    } else {
        printf("\nYou must re-open\n");
    }
}

/*TASK*-----------------------------------------------------------------
*
* Function Name  : mfs_nandflash_wl_open
* Returned Value : int_32
* Comments       :
*
*END------------------------------------------------------------------*/
int_32 mfs_nandflash_wl_open()
{
    if (nandflash_handle == NULL)
    {
        /* Open nand flash wl in normal mode */
        nandflash_handle = fopen(FFS_DEVICE, NANDWL_OPEN_NORMAL);
        if ( nandflash_handle == NULL )
        {
            printf("Unable to open nandflash device.\n");
            printf("\nYou must run Repair command\n");
            return 0;
        }

        /* Install MFS over nand_wl driver */
        error_code = _io_mfs_install(nandflash_handle, filesystem_name, (_file_size)0);
        if (error_code != MFS_NO_ERROR)
        {
            printf("Error initializing MFS: %s\n", MFS_Error_text((uint_32)error_code));
            _task_block();
        }

        if (filesystem_handle == NULL)
        {
            /* Open file system */
            filesystem_handle = fopen(filesystem_name, NULL);
            error_code = ferror (filesystem_handle);
            if ((error_code != MFS_NO_ERROR) && (error_code != MFS_NOT_A_DOS_DISK))
            {
                printf("Error opening filesystem: %s\n", MFS_Error_text((uint_32)error_code));
                _task_block();
            }

            if ( error_code == MFS_NOT_A_DOS_DISK )
            {
                printf("NOT A DOS DISK! You must format to continue.\n");
                return 0;

            }

            printf ("NAND flash installed to %s\n", filesystem_name);
        }
        else
        {
            printf("\nNAND flash was installed.\n");
        }
    }
    else
    {
        printf("\nNAND flash device was opened.\n");
    }

    return 0;
}

/*TASK*-----------------------------------------------------------------
*
* Function Name  : mfs_nandflash_wl_close
* Returned Value : int_32
* Comments       :
*
*END------------------------------------------------------------------*/
int_32 mfs_nandflash_wl_close()
{
    if (filesystem_handle != NULL)
    {
        /* Close the filesystem */
        if (MQX_OK != fclose (filesystem_handle))
        {
            printf("Error closing filesystem.\n");
            _task_block();
        }
        filesystem_handle = NULL;
    }

    if (nandflash_handle != NULL)
    {
        /* Uninstall MFS  */
        error_code = _io_dev_uninstall(filesystem_name);
        if (error_code != MFS_NO_ERROR)
        {
            printf("Error uninstalling filesystem.\n");
            _task_block();
        }

        /* Close the nand flash device */
        if (MQX_OK != fclose (nandflash_handle))
        {
            printf("Unable to close nandflash device.\n");
            _task_block();
        }
        nandflash_handle = NULL;

        printf ("nandflash uninstalled.\n");
    }

    return 0;
}

/*TASK*-----------------------------------------------------------------
*
* Function Name  : shell_task
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/
void shell_task(uint_32 temp)
{
    /* Run the shell on the serial port */
    printf("This is the Flash File System example, type help for list of available commands\n");
    printf("If this is the first time you use the demo, you should run \"nanderasechip\" command first.\n");

    if (_io_nandflash_wl_install(&_bsp_nandflash_wl_init, FFS_DEVICE) != MQX_OK)
    {
        printf("Can't install FFS!\n");
        _task_block();
    }

    for(;;)
    {
        Shell(Shell_commands, NULL);
        printf("Shell exited, restarting...\n");
    }
}

/*TASK*-----------------------------------------------------------------
*
* Function Name  : shell_fs_open
* Returned Value : int_32
* Comments       :
*
*END------------------------------------------------------------------*/
int_32  shell_fs_open(int_32 argc, char_ptr argv[] )
{
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;

    print_usage = Shell_check_help_request(argc, argv, &shorthelp );

    if (!print_usage)  {
        if (argc != 1)
        {
            printf("Error, invalid number of parameters\n");
            return_code = SHELL_EXIT_ERROR;
            print_usage=TRUE;
        }
        else
        {
            mfs_nandflash_wl_open();
        }
    }

    if (print_usage)  {
        if (shorthelp)  {
            printf("%s \n", argv[0]);
        } else  {
            printf("Usage: %s\n", argv[0]);
            printf ("   Open the Flash file system\n");
        }
    }
    return return_code;
}

/*TASK*-----------------------------------------------------------------
*
* Function Name  : shell_fs_close
* Returned Value : int_32
* Comments       :
*
*END------------------------------------------------------------------*/
int_32  shell_fs_close(int_32 argc, char_ptr argv[] )
{
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;

    print_usage = Shell_check_help_request(argc, argv, &shorthelp );

    if (!print_usage)  {
        if (argc != 1)
        {
            printf("Error, invalid number of parameters\n");
            return_code = SHELL_EXIT_ERROR;
            print_usage=TRUE;
        }
        else
        {
            mfs_nandflash_wl_close();
        }
    }

    if (print_usage)  {
        if (shorthelp)  {
            printf("%s \n", argv[0]);
        } else  {
            printf("Usage: %s\n", argv[0]);
            printf ("   Close the Flash file system\n");
        }
    }
    return return_code;
}

/*TASK*-----------------------------------------------------------------
*
* Function Name  : shell_nand_erase
* Returned Value : int_32
* Comments       :
*
*END------------------------------------------------------------------*/
int_32  shell_nand_erase(int_32 argc, char_ptr argv[] )
{
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;

    print_usage = Shell_check_help_request(argc, argv, &shorthelp );

    if (!print_usage)  {
        if (argc != 1)
        {
            printf("Error, invalid number of parameters\n");
            return_code = SHELL_EXIT_ERROR;
            print_usage=TRUE;
        }
        else
        {
            nandflash_erase();
        }
    }

    if (print_usage)  {
        if (shorthelp)  {
            printf("%s \n", argv[0]);
        } else  {
            printf("Usage: %s\n", argv[0]);
            printf ("   Erase the Flash file system\n");
        }
    }
    return return_code;

}

/*TASK*-----------------------------------------------------------------
*
* Function Name  : shell_nand_erase_chip
* Returned Value : int_32
* Comments       :
*
*END------------------------------------------------------------------*/
int_32  shell_nand_erase_chip(int_32 argc, char_ptr argv[] )
{
    boolean     print_usage, shorthelp = FALSE;
    int_32      return_code = SHELL_EXIT_SUCCESS;

    print_usage = Shell_check_help_request(argc, argv, &shorthelp );

    if (!print_usage)  {
        if (argc != 1)
        {
            printf("Error, invalid number of parameters\n");
            return_code = SHELL_EXIT_ERROR;
            print_usage=TRUE;
        }
        else
        {
            nandflash_erase_chip();
        }
    }

    if (print_usage)  {
        if (shorthelp)  {
            printf("%s \n", argv[0]);
        } else  {
            printf("Usage: %s\n", argv[0]);
            printf("   Erase NAND chip completely\n");
        }
    }
    return return_code;

}

/*TASK*-----------------------------------------------------------------
*
* Function Name  : shell_nand_repair
* Returned Value : int_32
* Comments       :
*
*END------------------------------------------------------------------*/
int_32  shell_nand_repair(int_32 argc, char_ptr argv[] )
{
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;

    print_usage = Shell_check_help_request(argc, argv, &shorthelp );

    if (!print_usage)  {
        if (argc != 1)
        {
            printf("Error, invalid number of parameters\n");
            return_code = SHELL_EXIT_ERROR;
            print_usage=TRUE;
        }
        else
        {
            nandflash_repair();
        }
    }

    if (print_usage)  {
        if (shorthelp)  {
            printf("%s \n", argv[0]);
        } else  {
            printf("Usage: %s\n", argv[0]);
            printf("    Repair the Flash file system\n");
        }
    }
    return return_code;
}

/* EOF */
