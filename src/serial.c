/*
 * serial.c
 *
 *  Created on: Aug 12, 2013
 *      Author: Iven
 */

#include "calcDistance.h"

void runSerial(int * x, int * y, float * results, int size_results, int pointsFromFileCounter, int size_height, int size_width)
{
	int i,width,height,point = 0;

	for (i = 0; i < size_results; i++) {
		point = i % pointsFromFileCounter;
		width = (i / pointsFromFileCounter) % size_width;
		height = ((i / pointsFromFileCounter / size_width)) % size_height;

		results[i] = calculateDistance(width,height,x[point],y[point]);
	}
}
