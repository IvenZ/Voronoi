/*
 * calcDistance.c
 *
 * Author: Iven
 */

/**
 * Calculate distance with theorem of Pythagoras
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @return Distance
 * @author Iven
 */
double calculateDistance (double x1,double y1,double x2 ,double y2) {
	return sqrt(((x1-x2) * (x1-x2)) + ((y1-y2) * (y1-y2)));
}
