#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#define MAX_POINTS 300
#define HEIGHT 600
#define WIDTH 1200

int x[MAX_POINTS];
int y[MAX_POINTS];
unsigned char rgb[MAX_POINTS*3];

float saveDistance[HEIGHT*WIDTH*MAX_POINTS];

int pointsFromFileCounter = 0;

int saveColor[HEIGHT][WIDTH];


GLsizei wh = HEIGHT ; // initial height of window
GLsizei ww = WIDTH ; // initial width of window


void draw()
{
	glClear ( GL_COLOR_BUFFER_BIT ); //clear pixel buffer

	int i,j,k = 0;
	for(i = 0; i < HEIGHT; i++){
		for(j = 0; j < WIDTH; j++){
			k = saveColor[i][j];
			glBegin(GL_POINTS); // render with points
			glColor3ub(rgb[k*3],rgb[k*3+1],rgb[k*3+2]); // die Farbe setzt sich aus drei aufeinanderfolgenden Werten im Array zusammen
			glVertex2i(j,i);
			glEnd();
		}
	}

	// draw points from file
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

	// argc should be 2 for correct execution
	if ( argc != 2 ) {
		printf( "usage: %s coordinates.txt (max. 500 points)\n", argv[0] );
	}
	else {
		// We assume argv[1] is a filename to open
		FILE *infile;
		infile = fopen(argv[1], "r");
		int i=0;

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

		double prgstart, prgende;
		prgstart=omp_get_wtime();

		int max = WIDTH*HEIGHT*pointsFromFileCounter;
		int point;
		int width;
		int height;


		#pragma omp parallel for private(point, width, height)
		for (i = 0; i < max; i++) {
			// reduce three loops to one
			point = i % pointsFromFileCounter;
			width = (i / pointsFromFileCounter) % WIDTH;
			height = ((i / pointsFromFileCounter / WIDTH)) % HEIGHT;

			saveDistance[i] = calculateDistance(width,height,x[point],y[point]);
		}

		int j,z = 0;
		point = 0;
		double tempDistance = HEIGHT*WIDTH;


		for(i=0;i<HEIGHT;i++){
			for(j=0;j<WIDTH;j++){
				// compares next MAX_POINTS points to X,Y
				while(point < pointsFromFileCounter){
					if (saveDistance[z] < tempDistance){
						tempDistance = saveDistance[z];
						saveColor[i][j] = point;
					}
					point++;
					z++;
				}
				// verified each point to X,Y?
				// reset tempDistance and point
				tempDistance = resetDistance(); point = 0;
			}
		}

		prgende=omp_get_wtime();
		printf("Laufzeit %.2f Sekunden\n",prgende-prgstart);


		// OpenGL stuff
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
