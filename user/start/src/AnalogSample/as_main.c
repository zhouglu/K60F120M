/**HEADER********************************************************************
* 
* Copyright (c) 2013 Holley Technology;
* All Rights Reserved                       
*
*************************************************************************** 
*
* $FileName: as_main.c$
* $Author  : Ganglu Zhou (ganglu.zhou@holley.cn)$
* $Version : 0.0.0.1$
* $Date    : Nov-21-2013$
*
* Comments:
*
*   This file is the source of analog sample application.
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h> 
#include <fio.h>
#include <shell.h>
#include <stdlib.h>
#include <math.h>

#include "analog_sample.h"

int_32 Shell_calc_energy(int_32 argc, char_ptr argv[])
{
    boolean      print_usage, shorthelp = FALSE;
    int_32       return_code = SHELL_EXIT_SUCCESS;
	int_32 opt;
	double f = 50.0;
	int_32 sps = 8000;
	int_32 hn = 19;
	float buf[512];
	float xR[512], xI[512];
	int_32 sampleNum = sizeof(buf)/sizeof(buf[0]);
	int_32 vm = 16384;

    print_usage = Shell_check_help_request(argc, argv, &shorthelp);

    if (!print_usage)
    {
        while (1) {
            opt = Shell_getopt(argc, argv, "f:s:h:n:v:");
            if (-1 == opt) break;

            switch (opt) {
            case 'f':
                f = atof(optarg);
                break;
            case 's':
                sps = atoi(optarg);
                break;
            case 'h':
                hn = atoi(optarg);
                break;
            case 'n':
                sampleNum = atoi(optarg);
                break;
            case 'v':
                vm = atoi(optarg);
                break;
            default:
                printf("Usage: %s [-f frequency]\n", 
                    argv[0]);
                return 0;
            }
        }

        double t = 1.0 / sps;

        sampleNum = sampleNum < sizeof(buf)/sizeof(buf[0]) ? 
            sampleNum : sizeof(buf)/sizeof(buf[0]);
        printf("sample num = %d, sps = %d\n", sampleNum, sps);
        int i = 0;
        for (; i < sampleNum; i++) {
            double tmp = vm * sin(2.0 * 3.14159 * f * i * t)
                + 0.03 * vm * sin(3 * 2 * 3.14159 * f * i * t)
                + 0.02 * vm * sin(5 * 2 * 3.14159 * f * i * t)
                + 0.01 * vm * sin(7 * 2 * 3.14159 * f * i * t);
            buf[i] = (float)tmp;
        }

        float freq = GetFreq(buf, sampleNum, -vm * 0.8, sps / 50);

        printf("freq:%10.6f, exp = %.6f, p = %6.4f%%\n", freq, f, (freq - f) / f *100.0);

        int num = (int)((sps + freq / 2) / freq);
        int order = 1;
        while (num >> order) order++;
        if (0 == num - (1 << (order - 1))) order -= 1;
        
        for (i = 0; i < num; i++) {
            xR[i] = (float)buf[i];
            xI[i] = 0.0;
        }

        for (; i < (1 << order); i++) {
            xR[i] = 0.0;
            xI[i] = 0.0;
        }

        printf("num = %d, order = %d\n", num, order);
        initTriTable(order);
        fft2(xR, xI, order);

        double v = 2 * sqrt(xR[1] * xR[1] + xI[1] * xI[1]) / num;
        double ev = vm * 1.0;
        printf("1rd: %10.4f, exp = %10.4f, p = %6.4f%%\n", v, ev, (v - ev)/ev * 100.0);
        v = 2 * sqrt(xR[3] * xR[3] + xI[3] * xI[3]) / num;
        ev = 0.03 * vm;
        printf("3rd: %10.4f, exp = %10.4f, p = %6.4f%%\n", v,  ev, (v - ev)/ev * 100.0);
        v = 2 * sqrt(xR[5] * xR[5] + xI[5] * xI[5]) / num;
        ev = 0.02 * vm;
        printf("5th: %10.4f, exp = %10.4f, p = %6.4f%%\n", v,  ev, (v - ev)/ev * 100.0);
        v = 2 * sqrt(xR[7] * xR[7] + xI[7] * xI[7]) / num;
        ev = 0.01 * vm;
        printf("7th: %10.4f, exp = %10.4f, p = %6.4f%%\n", v,  ev, (v - ev)/ev * 100.0);
        
        for (i = 1; i < hn; i++) {
            v = 2 * sqrt(xR[i] * xR[i] + xI[i] * xI[i]) / num;
            printf("%2dth: %10.4f, p = %6.4f%%\n", i, v,  v / vm * 100.0);
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
            printf("Usage: %s [-f signal_frequency] [-s sample_frequency] \
                   [-h harmonic_order] [-n sample_number] [-v signal_peak]\n", 
                       argv[0]);
            return 0;
        }
    }
    
    return return_code;
}
