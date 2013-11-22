/**HEADER********************************************************************
* 
* Copyright (c) 2013 Holley Technology;
* All Rights Reserved                       
*
*************************************************************************** 
*
* $FileName: calc_freq.c$
* $Author  : Ganglu Zhou (ganglu.zhou@holley.cn)$
* $Version : 0.0.0.1$
* $Date    : Nov-21-2013$
*
* Comments:
*
*   This file impliment the fucntion calcalating the frequerency of sample data.
*
*END************************************************************************/

#include "analog_sample.h"

/*----------------------------------------------------------
* 
* Function Name : GetZeroCross
*
* Arguments     :
*    buf       : in, address of sampled data buf 
*    num       : in, number of data
*    threshold : in, can be 0.8 * max value of data
*
* return        : index of zero cross item
*
* Comments      :
*    Find the first index of zero cross item in input array
*
*---------------------------------------------------------*/
static int GetZeroCross(DataType *buf, int num, DataType threshold)
{
	int negFlag = 0;
	int i = 0;
	for (; i < num; i++) {
		if (0 == negFlag) {
			if (threshold > buf[i]) negFlag = 1;
		} else {
			if (0 <= buf[i]) return i;
		}
	}

	return -1;
}

/*----------------------------------------------------------
* 
* Function Name : GetZeroCrossT
*
* Arguments     :
*    buf       : in, address of sampled data buf 
*    num       : in, number of data
*    start     : inout, start index for searching 
*    threshold : in, can be 0.8 * max value of data
*
* return        : time of zero cross
*
* Comments      :
*    Calcuate the time of the first zero cross from *start item
*
*---------------------------------------------------------*/
static float GetZeroCrossT(DataType *buf, int num, int *start, DataType threshold)
{
	int k = GetZeroCross(&buf[*start], num - *start, threshold);
	if (0 > k) return 0.0;
	*start += k;
	k = *start;
	if (k + 4 > num) return 0.0;

	int j = k - 4;
	float v = 0;
	int i = 0;

	for (; i < 8; i++) {
		v += buf[j + i];
	}
    
	float u = 0;
	for (i = 0; i < 8; ++i)	{
		u += (2 * i - 7) * (8 * buf[j + i] - v);
	}

	float t = k - 0.5 - 84 * v / u;

	return t;
}

/*----------------------------------------------------------
* 
* Function Name : GetFreq
*
* Arguments     :
*    buf       : in, address of sampled data buf 
*    num       : in, number of data
*    threshold : in, can be 0.8 * max value of data
*    sn        : in, sample number in a cycle 
*
* return        : frequency
*
* Comments      :
*    Calcuate the frequency of sample data
*
*---------------------------------------------------------*/
float GetFreq(DataType *buf, int num, DataType threshold, int sn)
{
	float crsossTime[33];
	int loops = num / sn;
	int i = 0, zeroPoint = 0, cross = 0;
	for (; i < loops && cross < 33; ++i)
	{
		float t = GetZeroCrossT(buf, num, &zeroPoint, threshold);
		if (0.0 < t) {
			crsossTime[cross++] = t;
		} else {
			break;
		}
	}

	if (2 > cross) return 0.0;

	float period = 0.0;
	for (i = 0; i < cross - 1; ++i)
	{
		period += crsossTime[i + 1] - crsossTime[i];
	}

	period /= cross - 1;

	return (50.0 * sn / period);
}
