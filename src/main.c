#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_POINTS 500
#define HEIGHT 500
#define WIDTH 500

int x[MAX_POINTS];
int y[MAX_POINTS];
unsigned char rgb[MAX_POINTS*3];

int height[HEIGHT];
int width[WIDTH];

int saveColor[HEIGHT][WIDTH];

int pointsFromFile = -1;


GLsizei wh = HEIGHT ; // initial height of window
GLsizei ww = WIDTH ; // initial width of window


void draw()
{
	glClear ( GL_COLOR_BUFFER_BIT ); //clear pixel buffer

	int i,j,k;

	for(i = 0; i < HEIGHT; i++){
		for(j = 0; j < WIDTH; j++){

			for(k = 0; k < pointsFromFile; k++){
				if(saveColor[i][j] == k){
					glBegin(GL_POINTS); // render with points
					glColor3ub(rgb[k*3],rgb[k*3+1],rgb[k*3+2]); // die Farbe setzt sich aus drei aufeinanderfolgenden Werten im Array zusammen
					glVertex2i(width[j],height[i]);
					glEnd();
				}
			}
		}
	}

	// draw POINTS
	glPointSize(2.0f);
	glBegin(GL_POINTS);
	glColor3f(0.0f, 0.0f,0.0f); // black drawing color
	for(i = 0; i < pointsFromFile; i++){
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

		while(!feof(infile)) {
			pointsFromFile++;
			fscanf(infile,"%d %d",&x[pointsFromFile],&y[pointsFromFile]);
		}
		fclose(infile);



		int i,j,k;
		double distance;
		double tempDistance = resetDistance();

		// fill width and height. (its only possible, cause height and width is the same!)
		for(i = 0; i < WIDTH; i++) {
			width[i] = i;
			height[i] = i;
		}

		// generate random colors
		for(i = 0; i < (pointsFromFile)*3; i++) {
			rgb[i] = frand(256);
		}

		// iterate through each pixel from HEIGHT x WEIGHT and calculate the distance to our given points.
		for(i = 0; i < HEIGHT; i++){
			for(k = 0; k < WIDTH; k++){
				for(j = 0; j < pointsFromFile; j++){
					distance = calculateDistance(width[k],height[i],x[j],y[j]);
					if (distance < tempDistance){
						tempDistance = distance;
						saveColor[i][k] = j;
					}
				}
				tempDistance = resetDistance();
			}
		}


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
