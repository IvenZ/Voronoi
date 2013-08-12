/*
 * calcDistance.c
 *
 *  Created on: Aug 12, 2013
 *      Author: iven
 */

double calculateDistance (double x1,double y1,double x2 ,double y2) {
	return sqrt(((x1-x2) * (x1-x2)) + ((y1-y2) * (y1-y2)));
}
