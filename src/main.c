#include <assert.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <GL/glut.h>

#include <CL/cl.h>

#define MAX_POINTS 270 // ab 274 kracht es im OpenCL :(
#define HEIGHT 600
#define WIDTH 1200
int saveColor[HEIGHT][WIDTH];
unsigned char rgb[MAX_POINTS*3];
int x[MAX_POINTS];
int y[MAX_POINTS];

int pointsFromFileCounter = 0;

GLsizei wh = HEIGHT ; // initial height of window
GLsizei ww = WIDTH ; // initial width of window

char * load_program_source(const char *filename)
{

	struct stat statbuf;
	FILE *fh;
	char *source;

	fh = fopen(filename, "r");
	if (fh == 0)
		return 0;

	stat(filename, &statbuf);
	source = (char *) malloc(statbuf.st_size + 1);
	fread(source, statbuf.st_size, 1, fh);
	source[statbuf.st_size] = '\0';

	return source;
}

int runCL(int * a, int * b, float * results, int n, int points, int size_height, int size_width)
{
	cl_program program[1];
	cl_kernel kernel[2];

	cl_command_queue cmd_queue;
	cl_context   context;

	cl_device_id cpu = NULL, device = NULL;
	cl_platform_id platform;
	cl_uint platforms;

	cl_int err = 0;
	size_t returned_size = 0;
	size_t buffer_size, buffer_size_a, buffer_size_b;

	cl_mem a_mem, b_mem, ans_mem;

	{
		// Fetch available platform; we only want the first one
		err = clGetPlatformIDs(1, &platform, &platforms);
		assert(err == CL_SUCCESS);
		printf("Number of available platforms = %d\n",platforms);

		// Find the CPU CL device, as a fallback
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &cpu, NULL);
		assert(err == CL_SUCCESS);

		// Find the GPU CL device, this is what we really want
		// If there is no GPU device is CL capable, fall back to CPU
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
		if (err != CL_SUCCESS) device = cpu;
		assert(device);

		// Get some information about the returned device
		cl_char vendor_name[1024] = {0};
		cl_char device_name[1024] = {0};
		err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(vendor_name),
							  vendor_name, &returned_size);
		err |= clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name),
							  device_name, &returned_size);
		assert(err == CL_SUCCESS);
		printf("Connecting to %s %s...\n", vendor_name, device_name);
	}

	{
		// Now create a context to perform our calculation with the
		// specified device
		context = clCreateContext(0, 1, &device, NULL, NULL, &err);
		assert(err == CL_SUCCESS);

		// And also a command queue for the context
		cmd_queue = clCreateCommandQueue(context, device, 0, NULL);
	}

	{
		// Load the program source from disk
		// The kernel/program is the project directory and in Xcode the executable
		// is set to launch from that directory hence we use a relative path
		const char * filename = "voronoi.cl";
		char *program_source = load_program_source(filename);
		program[0] = clCreateProgramWithSource(context, 1, (const char**)&program_source,
											   NULL, &err);
		assert(err == CL_SUCCESS);

		err = clBuildProgram(program[0], 0, NULL, NULL, NULL, NULL);
		assert(err == CL_SUCCESS);

		// Now create the kernel "objects" that we want to use in the example file
		kernel[0] = clCreateKernel(program[0], "calculate", &err);
	}

	{
		// Allocate memory on the device to hold our data and store the results into
		buffer_size = sizeof(float) * n;
		buffer_size_a = sizeof(int) * points;
		buffer_size_b = sizeof(int) * points;

		// Input array a
		a_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, buffer_size_a, NULL, NULL);
		err = clEnqueueWriteBuffer(cmd_queue, a_mem, CL_TRUE, 0, buffer_size_a,
								   (void*)a, 0, NULL, NULL);

		// Input array b
		b_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, buffer_size_b, NULL, NULL);
		err |= clEnqueueWriteBuffer(cmd_queue, b_mem, CL_TRUE, 0, buffer_size_b,
									(void*)b, 0, NULL, NULL);
		assert(err == CL_SUCCESS);

		// Results array
		ans_mem	= clCreateBuffer(context, CL_MEM_READ_WRITE, buffer_size, NULL, NULL);

		// Get all of the stuff written and allocated
		clFinish(cmd_queue);
	}

	{
		// Now setup the arguments to our kernel
		err  = clSetKernelArg(kernel[0],  0, sizeof(cl_mem), &a_mem);
		err |= clSetKernelArg(kernel[0],  1, sizeof(cl_mem), &b_mem);
		err |= clSetKernelArg(kernel[0],  2, sizeof(cl_mem), &ans_mem);
		err |= clSetKernelArg(kernel[0],  3, sizeof(int), &points);
		err |= clSetKernelArg(kernel[0],  4, sizeof(int), &size_width);
		err |= clSetKernelArg(kernel[0],  5, sizeof(int), &size_height);
		assert(err == CL_SUCCESS);
	}

	{
		// Run the calculation by enqueuing it and forcing the
		// command queue to complete the task
		size_t global_work_size = n;
		err = clEnqueueNDRangeKernel(cmd_queue, kernel[0], 1, NULL,
									 &global_work_size, NULL, 0, NULL, NULL);
		assert(err == CL_SUCCESS);
		clFinish(cmd_queue);

		// Once finished read back the results from the answer
		// array into the results array
		err = clEnqueueReadBuffer(cmd_queue, ans_mem, CL_TRUE, 0, buffer_size,
								  results, 0, NULL, NULL);
		assert(err == CL_SUCCESS);
		clFinish(cmd_queue);
	}

	{
		clReleaseMemObject(a_mem);
		clReleaseMemObject(b_mem);
		clReleaseMemObject(ans_mem);

		clReleaseCommandQueue(cmd_queue);
		clReleaseContext(context);
	}
	return CL_SUCCESS;
}

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

#define frand(x) (rand() / (1. + RAND_MAX) * x)
int main (int argc, char **argv) {

	if ( argc != 2 ) {
			printf( "usage: %s coordinates.txt\n", argv[0] );
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
		for(i = 0; i < (MAX_POINTS)*3; i++) {
			rgb[i] = frand(256);
		}

		int n = HEIGHT*WIDTH*pointsFromFileCounter;
		int point;
		int width;
		int height;
		int j;
		int nCounter;
		double tempDistance = HEIGHT*WIDTH;

		// Allocate memory and a place for the results
		float * results = (float *)malloc(n*sizeof(float));

		double prgstart, prgende;
		prgstart=omp_get_wtime();

		// Do the OpenCL calculation
		runCL(x, y, results, n, pointsFromFileCounter, HEIGHT, WIDTH);

		prgende=omp_get_wtime();
		printf("OpenCL Laufzeit: %.2f Sekunden\n",prgende-prgstart);
		prgstart=omp_get_wtime();

		// Do the OpenMP calculation
		#pragma omp parallel for private(point, width, height)
		for (i = 0; i < n; i++) {
			// reduce three loops to one
			point = i % pointsFromFileCounter;
			width = (i / pointsFromFileCounter) % WIDTH;
			height = ((i / pointsFromFileCounter / WIDTH)) % HEIGHT;

			results[i] = calculateDistance(width,height,x[point],y[point]);
		}

		prgende=omp_get_wtime();
		printf("OpenMP Laufzeit: %.2f Sekunden\n",prgende-prgstart);
		prgstart=omp_get_wtime();

		// Do the seriell calculation
		for (i = 0; i < n; i++) {
			// reduce three loops to one
			point = i % pointsFromFileCounter;
			width = (i / pointsFromFileCounter) % WIDTH;
			height = ((i / pointsFromFileCounter / WIDTH)) % HEIGHT;

			results[i] = calculateDistance(width,height,x[point],y[point]);
		}

		prgende=omp_get_wtime();
		printf("serielle Laufzeit: %.2f Sekunden\n",prgende-prgstart);


		/**
		 * Prepare color index
		 */
		point = 0;
		nCounter = 0;

		for(i=0;i<HEIGHT;i++){
			for(j=0;j<WIDTH;j++){
				// compares next MAX_POINTS points to X,Y
				while(point < pointsFromFileCounter){
					if (results[nCounter] < tempDistance){
						tempDistance = results[nCounter];
						saveColor[i][j] = point;
					}
					point++;
					nCounter++;
				}
				// verified each point to X,Y?
				// reset tempDistance and point
				tempDistance = HEIGHT*WIDTH; point = 0;
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
