/**HEADER********************************************************************
* 
* Copyright (c) 2013 Holley Technology;
* All Rights Reserved                       
*
*************************************************************************** 
*
* $FileName: fft_2.c$
* $Author  : Ganglu Zhou (ganglu.zhou@holley.cn)$
* $Version : 0.0.0.1$
* $Date    : Nov-21-2013$
*
* Comments:
*
*   This file contains the source for radix-2 FFT.
*
*END************************************************************************/

#include <math.h>
#include "analog_sample.h"

static DataType triFuncTabRe[TRINUM], triFuncTabIm[TRINUM];

/*----------------------------------------------------------
* 
* Function Name : initTriTable
*
* Arguments     :
*    order     : in, exponential of radix-2
*
* return        : none
*
* Comments      :
*    Initialize triangle function table
*
*---------------------------------------------------------*/
void initTriTable(int order)
{
    int i;
	double angle, cosw, sinw;
    int num = 1 << (order - 1);
	angle = PI / num;
	cosw = cos(angle);
	sinw = -sin(angle);
	triFuncTabRe[0] = 1.0;
	triFuncTabIm[0] = 0.0;
	triFuncTabRe[1] = cosw;
	triFuncTabIm[1] = sinw;
    for (i = 2; i < num; i++) {
		triFuncTabRe[i] = cosw*triFuncTabRe[i-1] - sinw*triFuncTabIm[i-1];
		triFuncTabIm[i] = sinw*triFuncTabRe[i-1] + cosw*triFuncTabIm[i-1];
    }
}

/*----------------------------------------------------------
* 
* Function Name : fft2
*
* Arguments     :
*    dataR     : io, address of data real part
*    dataI     : io, address of data real part
*    order     : in, exponential of radix-2
*
* return        : none
*
* Comments      :
*    radix-2 FFT
*
*---------------------------------------------------------*/
void fft2(DataType dataR[], DataType dataI[], int order)
{
	int count = 1 << order;
	int i, j, k;
	int bfsize, p;
	DataType addR, addI, plusR, plusI;

	for (k = 0; k < order; k++)
	{
		bfsize = 1 << (order - k);		
		for (j = 0; j < 1 << order; j += bfsize)
		{
			for (i = 0; i < bfsize / 2; i++)
			{
				int l = i + j;
				p = i << k;

				addR = dataR[l] + dataR[l + bfsize / 2];
				addI = dataI[l] + dataI[l + bfsize / 2];
				plusR = dataR[l] - dataR[l + bfsize / 2];
				plusI = dataI[l] - dataI[l + bfsize / 2];
				dataR[l] = addR;
				dataI[l] = addI;
				dataR[l + bfsize / 2] = plusR * triFuncTabRe[p] - plusI * triFuncTabIm[p];
				dataI[l + bfsize / 2] = plusI * triFuncTabRe[p] + plusR * triFuncTabIm[p];
			}
		}
	}

	for (j = 0; j < count / 2; j++)
	{
		p = 0;
		for (i = 0; i <order ; i++)
		{
			if (j & (1 << i)) p += 1 << (order - i - 1);
		}
		
		if (j < p) {
			addR = dataR[j];
			addI = dataI[j];
			dataR[j] = dataR[p];
			dataI[j] = dataI[p];
			dataR[p] = addR;
			dataI[p] = addI;
		}
	}
} /* END FFT */
