#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#define MAX_POINTS 5
#define HEIGHT 600
#define WIDTH 1200

int x[MAX_POINTS];
int y[MAX_POINTS];
unsigned char rgb[MAX_POINTS*3];

//float saveColor[WIDTH*HEIGHT*MAX_POINTS*5];

float saveDistance[HEIGHT*WIDTH*MAX_POINTS];

int pointsFromFileCounter = 0;

int numOfIters = 0;

int saveColor[HEIGHT][WIDTH];


GLsizei wh = HEIGHT ; // initial height of window
GLsizei ww = WIDTH ; // initial width of window


void draw()
{
	glClear ( GL_COLOR_BUFFER_BIT ); //clear pixel buffer

//	int i,j,k = 0;
//	for(i = 0; i < HEIGHT; i++){
//		for(j = 0; j < WIDTH; j++){
//			k = saveColor[i][j];
//			glBegin(GL_POINTS); // render with points
//			glColor3ub(rgb[k*3],rgb[k*3+1],rgb[k*3+2]); // die Farbe setzt sich aus drei aufeinanderfolgenden Werten im Array zusammen
//			glVertex2i(j,i);
//			glEnd();
//		}
//	}

	int i,j,k,z = 0;
	double tempDistance = HEIGHT*WIDTH;
//////
	for(i=0;i<HEIGHT;i++){
		for(j=0;j<WIDTH;j++){
			// geht immer nur die nächsten max_point vor.
			while(k < MAX_POINTS){
				//printf("%d %d point: %f\n",j, i,saveDistance[z]);
				if (saveDistance[z] < tempDistance){
					tempDistance = saveDistance[z];
					glBegin(GL_POINTS); // render with points
					glColor3ub(rgb[k*3],rgb[k*3+1],rgb[k*3+2]); // die Farbe setzt sich aus drei aufeinanderfolgenden Werten im Array zusammen
					glVertex2i(j,i);
					glEnd();
					//printf("width: %d height: %d shortest: %f\n",j,i, saveDistance[z]);
				}
				k++;
				z++; // ist wie max

			}
			tempDistance = HEIGHT*WIDTH;
			k = 0;
		}
	}
//	for(i=0;i<HEIGHT*WIDTH*MAX_POINTS;i++){
//
//		if(i%pointsFromFileCounter == 0){
//			tempDistance = resetDistance();
//			printf("reset\n");
//			k = 0;
//		}
//
//
//		if(saveColor[i*5+4] < tempDistance){
//			tempDistance = saveColor[i*5+4];
//			printf("male %d\n",k);
//			glBegin(GL_POINTS); // render with points
//			glColor3ub(rgb[k*3],rgb[k*3+1],rgb[k*3+2]); // die Farbe setzt sich aus drei aufeinanderfolgenden Werten im Array zusammen
//			glVertex2i((int)saveColor[i*5],(int)saveColor[i*5+1]);
//			glEnd();
//
//		}
//		printf("%f %f %f %f %f\n",saveColor[i*5],saveColor[i*5+1],saveColor[i*5+2],saveColor[i*5+3],saveColor[i*5+4]);
//		k++;
//
//	}
//	for(i=0;i<numOfIters;i++){
//		//printf("%d %d %d\n",saveColor2[i],saveColor2[i*3+1],saveColor2[i*3+2]);
//		int k = saveColor[i*3+2];
//		glBegin(GL_POINTS); // render with points
//		glColor3ub(rgb[k*3],rgb[k*3+1],rgb[k*3+2]); // die Farbe setzt sich aus drei aufeinanderfolgenden Werten im Array zusammen
//		glVertex2i(saveColor[i*3],saveColor[i*3+1]);
//		glEnd();
//	}


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
		int p;
		int point;
		int width;
		int height;


		#pragma omp parallel for private(point, width, height)
		for (p = 0; p < max; p++) {
			// reduce three loops to one
			point = p % pointsFromFileCounter;
			width = (p / pointsFromFileCounter) % WIDTH;
			height = ((p / pointsFromFileCounter / WIDTH)) % HEIGHT;

			//printf("Differenz: width: %d, height: %d an Punkt: [%d %d] = %f\n",width, height, x[point],y[point],sqrt(((width-x[point]) * (width-x[point])) + ((height-y[point]) * (height-y[point]))));

			saveDistance[p] = calculateDistance(width,height,x[point],y[point]);
			//printf("width: %d height:%d punktX:%d punktY:%d - distanz:%f\n",width,height,x[point],y[point],saveDistance[p]);
//			saveColor[p*5] = height;
//			saveColor[p*5+1] = width;
//			saveColor[p*5+2] = x[point];
//			saveColor[p*5+3] = y[point];
//			saveColor[p*5+4] = saveDistance[p];

			//printf("width: %d, height: %d, point: %d\n",width,height, point);

			//printf("%f\n",);

			//calculateDistance(width,height,x[point],y[point]);

			// reset tempDistance after iterating through each point
//			if(p%pointsFromFileCounter == 0){
//				tempDistance = resetDistance();
//			}
//
//			distance = calculateDistance(width,height,x[point],y[point]);
//
//			if (distance < tempDistance){
//				tempDistance = distance;
//				//saveColor[width][height] = point;
//				saveColor2[numOfIters*3] = width;
//				saveColor2[numOfIters*3+1] = height;
//				saveColor2[numOfIters*3+2] = point;
//
//				#pragma omp critical
//				numOfIters++;
//			}
		}

//		int j,k,z = 0;
//		double tempDistance = HEIGHT*WIDTH;
//
//	//////
//		for(i=0;i<HEIGHT;i++){
//			for(j=0;j<WIDTH;j++){
//				// geht immer nur die nächsten max_point vor.
//				while(k < MAX_POINTS){
//					//printf("%d %d point: %f\n",j, i,saveDistance[z]);
//					if (saveDistance[z] < tempDistance){
//						tempDistance = saveDistance[z];
//						saveColor[i][j] = k;
//						//printf("width: %d height: %d shortest: %f\n",j,i, saveDistance[z]);
//					}
//					k++;
//					z++; // ist wie max
//
//				}
//				tempDistance = HEIGHT*WIDTH;
//				k = 0;
//			}
//		}


//		for(i=0;i<max;i++){
//				printf("%f\n",saveDistance[i]);
//		}


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
