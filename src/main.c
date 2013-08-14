/*
 * calcDistance.h
 *
 * Author		: Iven
 * Description	: Simple Voronoi diagram with graphical output (OpenGL)
 * Usage		: ./Voronoi coordinats.txt
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <GL/glut.h>
#include "calcDistance.h"
#include "opencl.h"
#include "openmp.h"
#include "serial.h"

#define MAX_POINTS 250 // crashes at 274 in OpenCL :(
#define HEIGHT 600
#define WIDTH 1200

// need global variables to access them in draw function
int saveColor[HEIGHT][WIDTH];
int x[MAX_POINTS];
int y[MAX_POINTS];
int pointsFromFileCounter = 0;

void draw()
{
	int i,j,colorIndex;
	unsigned char rgb[MAX_POINTS*3];

	/**
	 * generate some random RGB colors consecutively
	 * e.g. rgb[0]+rgb[1]+rgb[2] = yellow
	 * 		rgb[3]+rgb[4]+rgb[5] = blue
	 * 		and so on ...
	 */
	for(i = 0; i < (MAX_POINTS)*3; i++) {
		rgb[i] = rand() / (1. + RAND_MAX) * 256;
	}

	/**
	 * draw every pixel with its assigned color
	 */
	glClear ( GL_COLOR_BUFFER_BIT );
	for(i = 0; i < HEIGHT; i++){
		for(j = 0; j < WIDTH; j++){
			colorIndex = saveColor[i][j];
			glBegin(GL_POINTS);
			glColor3ub(rgb[colorIndex*3],rgb[colorIndex*3+1],rgb[colorIndex*3+2]);
			glVertex2i(j,i);
			glEnd();
		}
	}

	/**
	 * draw every point from file twice the size of normal pixel
	 */
	glPointSize(2.0f);
	glBegin(GL_POINTS);
	glColor3f(0.0f, 0.0f,0.0f); // black drawing color
	for(i = 0; i < pointsFromFileCounter; i++){
		glVertex2i(x[i],y[i]);
	}
	glEnd();
	glFlush();
}

/**
 *	Read and save values from file
 *	Start OpenCL/OpenMP/Serial calculation
 *	Evaluate shortest distance and save it (colorIndex)
 */
int main (int argc, char **argv) {

	if ( argc != 2 ) {
			printf( "usage: %s coordinates.txt\n", argv[0] );
	}
	else {
		/**
		 * Read file with coordinats
		 * We assume argv[1] is the filename to open
		 */
		FILE *infile;
		infile = fopen(argv[1], "r");
		int i = 0;

		/**
		 * Analyze file line per line and save x and y values
		 * Increase pointsFromFileCounter to count actual pair of values in file
		 */
		while(i < MAX_POINTS) {
			fscanf(infile,"%d %d",&x[pointsFromFileCounter],&y[pointsFromFileCounter]);
			i++;
			pointsFromFileCounter++;
		}
		fclose(infile);

		// initialize and allocate some space for the results
		int result_size = HEIGHT*WIDTH*pointsFromFileCounter;
		float * results = (float *)malloc(result_size*sizeof(float));
		double prgstart, prgende;

		// Do the serial calculation and timing
		prgstart = omp_get_wtime();

		runSerial(x, y, results, result_size, pointsFromFileCounter, HEIGHT, WIDTH);

		prgende = omp_get_wtime();
		printf("serielle Laufzeit: %.2f Sekunden\n",prgende-prgstart);

		// Do the OpenMP calculation and timing
		prgstart = omp_get_wtime();

		runOpenMP(x, y, results, result_size, pointsFromFileCounter, HEIGHT, WIDTH);

		prgende = omp_get_wtime();
		printf("OpenMP Laufzeit: %.2f Sekunden\n",prgende-prgstart);

		// Do the OpenCL calculation and timing
		prgstart = omp_get_wtime();

		runCL(x, y, results, result_size, pointsFromFileCounter, HEIGHT, WIDTH);

		prgende = omp_get_wtime();
		printf("OpenCL Laufzeit: %.2f Sekunden\n",prgende-prgstart);

		/**
		 * Evaluate shortest distance and save it as a colorIndex
		 */
		int point, j;
		int nCounter = 0;
		double tempDistance;

		for(i=0;i<HEIGHT;i++){
			for(j=0;j<WIDTH;j++){

				// reset tempDistance and point
				tempDistance = HEIGHT*WIDTH; point = 0;

				// compares next MAX_POINTS points to X,Y
				while(point < pointsFromFileCounter){
					if (results[nCounter] < tempDistance){
						tempDistance = results[nCounter];
						saveColor[i][j] = point;
					}
					point++;
					nCounter++;
				}
			}
		}

		free(results);

		/**
		 * Do all necessary OpenGL stuff
		 */
		glutInit ( &argc, argv );
		glutInitDisplayMode ( GLUT_SINGLE | GLUT_RGB );
		glutInitWindowSize ( (GLsizei)WIDTH, (GLsizei)HEIGHT );
		glutInitWindowPosition ( 100, 100 );
		glutCreateWindow("Voronoi");
		glClearColor ( 1.0, 1.0, 1.0, 0.0 ); //white background
		glMatrixMode ( GL_PROJECTION );
		glLoadIdentity ( );
		gluOrtho2D(0.0,(GLdouble)WIDTH,0.0,(GLdouble)HEIGHT);
		glutDisplayFunc ( draw );
		glutMainLoop ();

	}
    return EXIT_SUCCESS;
}


