/*
 * serial.c
 *
 * Author: Iven
 */

#include "calcDistance.h"

/**
 * Calculate each distance between all given points and every pixel in our window
 * @param x	X values
 * @param y	Y values
 * @param results	array to save the distance
 * @param size_results	number of calculations
 * @param size_points	number of x,y value pairs
 * @param size_height	windows height (number of x pixels)
 * @param size_width	window width (number of y pixels)
 */
void runSerial(int * x, int * y, float * results, int size_results, int size_points, int size_height, int size_width)
{
	int i,width,height,point;

	for (i = 0; i < size_results; i++) {
		point = i % size_points;
		width = (i / size_points) % size_width;
		height = ((i / size_points / size_width)) % size_height;

		results[i] = calculateDistance(width,height,x[point],y[point]);
	}
}
