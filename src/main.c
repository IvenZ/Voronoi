#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#define MAX_POINTS 50	 //Segmentation fault @ 319 ?!
#define HEIGHT 600
#define WIDTH 1200

int x[MAX_POINTS];
int y[MAX_POINTS];
int rgb[MAX_POINTS*3];

double saveDistance[HEIGHT][WIDTH][MAX_POINTS];
int pointsFromFileCounter = 0;

GLsizei wh = HEIGHT ; // initial height of window
GLsizei ww = WIDTH ; // initial width of window


void draw()
{
	glClear ( GL_COLOR_BUFFER_BIT ); //clear pixel buffer

	int i,j,k;
	double shortestDistance = HEIGHT*WIDTH;

	for(i = 0; i < HEIGHT; i++){
		for(j = 0; j < WIDTH; j++){

			for(k = 0; k < pointsFromFileCounter; k++){

				if(saveDistance[i][j][k] < shortestDistance){
					shortestDistance = saveDistance[i][j][k];
					glBegin(GL_POINTS); // render with points
					glColor3ub(rgb[k*3],rgb[k*3+1],rgb[k*3+2]); // die Farbe setzt sich aus drei aufeinanderfolgenden Werten im Array zusammen
					glVertex2i(j,i);
					glEnd();
				}
			}
			// reset shortestDistance for each x;y
			shortestDistance = resetDistance();
		}
	}

	// draw POINTS
	glPointSize(2.0f);
	glBegin(GL_POINTS);
	glColor3f(0.0f, 0.0f,0.0f); // black drawing color
	for(i = 0; i < pointsFromFileCounter; i++){
		glVertex2i(x[i],y[i]);
	}
	glEnd();

	glFlush();
}

double calculateDistance (double x1,double y1,double x2 ,double y2) {
	return sqrt(((x1-x2) * (x1-x2)) + ((y1-y2) * (y1-y2)));
}
int resetDistance(){
	return HEIGHT*WIDTH;
}

#define frand(x) (rand() / (1. + RAND_MAX) * x)
int main(int argc, char **argv) {

	clock_t prgstart, prgende;
	prgstart=clock();

	int i=0;
	int j=0;
	int k=0;
	double distance = 0.0;
	int T[] = { 0,0,0,0 };

	// argc should be 2 for correct execution
	if ( argc != 2 ) {
		printf( "usage: %s coordinates.txt (max. 500 points)\n", argv[0] );
	}
	else {
		// We assume argv[1] is a filename to open
		FILE *infile;
		infile = fopen(argv[1], "r");

		while(i < MAX_POINTS) {
			fscanf(infile,"%d %d",&x[pointsFromFileCounter],&y[pointsFromFileCounter]);
			i++;
			pointsFromFileCounter++;
		}
		fclose(infile);

		// generate random colors
		for(i = 0; i < (pointsFromFileCounter)*3; i++) {
			rgb[i] = frand(256);
		}


		// iterate through each pixel from HEIGHT x WEIGHT and calculate the distance to our given points.

		//#pragma omp parallel for private(j,k)
		for(i = 0; i < HEIGHT; i++){
			for(j = 0; j < WIDTH; j++){
				for(k = 0; k < pointsFromFileCounter; k++){
					distance = calculateDistance(j,i,x[k],y[k]);
					saveDistance[i][j][k] = distance;
					T[omp_get_thread_num()]++;

				}
			}
		}

		printf("Thread 1: %d calculations\n", T[0]);
		printf("Thread 2: %d calculations\n", T[1]);
		printf("Thread 3: %d calculations\n", T[2]);
		printf("Thread 4: %d calculations\n", T[3]);

		prgende=clock();
		printf("Laufzeit %.2f Sekunden\n",((float)(prgende-prgstart) / CLOCKS_PER_SEC)/1);

		// OpenGL stuff
//
		glutInit ( &argc, argv );
		glutInitDisplayMode ( GLUT_SINGLE | GLUT_RGB );
		glutInitWindowSize ( ww, wh );
		glutInitWindowPosition ( 100, 100 );
		glutCreateWindow("Voronoi");
		glClearColor ( 1.0, 1.0, 1.0, 0.0 ); //white background
		glMatrixMode ( GL_PROJECTION );
		glLoadIdentity ( );
		gluOrtho2D(0.0,(GLdouble)ww,0.0,(GLdouble)wh);
		glutDisplayFunc ( draw );

		glutMainLoop ();

	}

return 0;
}
