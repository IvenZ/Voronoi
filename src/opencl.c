/*
 * opencl.c
 *
 * Author: Iven
 */

#include <assert.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <CL/cl.h>

/**
 * Load kernel file
 * @param filename - name of kernel file
 * @return kernel file
 */
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

int runCL(int * x, int * y, float * results, int size_results, int size_points, int size_height, int size_width)
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
	size_t buffer_size, buffer_size_x, buffer_size_y;

	cl_mem x_mem, y_mem, ans_mem;

	{
		// Fetch available platform; we only want the first one
		err = clGetPlatformIDs(1, &platform, &platforms);
		assert(err == CL_SUCCESS);
		//printf("Number of available platforms = %d\n",platforms);

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
		//printf("Connecting to %s %s...\n", vendor_name, device_name);
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
		buffer_size = sizeof(float) * size_results;
		buffer_size_x = sizeof(int) * size_points;
		buffer_size_y = sizeof(int) * size_points;

		// Input array a
		x_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, buffer_size_x, NULL, NULL);
		err = clEnqueueWriteBuffer(cmd_queue, x_mem, CL_TRUE, 0, buffer_size_x,
								   (void*)x, 0, NULL, NULL);

		// Input array b
		y_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, buffer_size_y, NULL, NULL);
		err |= clEnqueueWriteBuffer(cmd_queue, y_mem, CL_TRUE, 0, buffer_size_y,
									(void*)y, 0, NULL, NULL);
		assert(err == CL_SUCCESS);

		// Results array
		ans_mem	= clCreateBuffer(context, CL_MEM_READ_WRITE, buffer_size, NULL, NULL);

		// Get all of the stuff written and allocated
		clFinish(cmd_queue);
	}

	{
		// Now setup the arguments to our kernel
		err  = clSetKernelArg(kernel[0],  0, sizeof(cl_mem), &x_mem);
		err |= clSetKernelArg(kernel[0],  1, sizeof(cl_mem), &y_mem);
		err |= clSetKernelArg(kernel[0],  2, sizeof(cl_mem), &ans_mem);
		err |= clSetKernelArg(kernel[0],  3, sizeof(int), &size_points);
		err |= clSetKernelArg(kernel[0],  4, sizeof(int), &size_width);
		err |= clSetKernelArg(kernel[0],  5, sizeof(int), &size_height);
		assert(err == CL_SUCCESS);
	}

	{
		// Run the calculation by enqueuing it and forcing the
		// command queue to complete the task
		size_t global_work_size = size_results;
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
		clReleaseMemObject(x_mem);
		clReleaseMemObject(x_mem);
		clReleaseMemObject(ans_mem);

		clReleaseCommandQueue(cmd_queue);
		clReleaseContext(context);
	}
	return CL_SUCCESS;
}

