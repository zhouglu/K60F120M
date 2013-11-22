/**HEADER********************************************************************
* 
* Copyright (c) 2013 Holley Technology;
* All Rights Reserved                       
*
*************************************************************************** 
*
* $FileName: analog_sample.h$
* $Author  : Ganglu Zhou (ganglu.zhou@holley.cn)$
* $Version : 0.0.0.1$
* $Date    : Nov-21-2013$
*
* Comments:
*
*   This file contains prototypes of analog sample functions, some 
*   type define and constants.
*
*END************************************************************************/

#ifndef _ANALOG_SAMPLE_
#define _ANALOG_SAMPLE_

#define PI 3.14159265359
#define TRINUM  64

typedef float DataType;
//#define DataType float

float GetFreq(DataType *buf, int num, DataType threshold, int sn);
void initTriTable(int order);
void fft2(DataType dataR[], DataType dataI[], int order);

#endif
