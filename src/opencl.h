/*
 * opencl.h
 *
 * Author: Iven
 */

#ifndef OPENCL_H_
#define OPENCL_H_

char * load_program_source(const char *filename);
int runCL(int * a, int * b, float * results, int size_results, int points, int size_height, int size_width);

#endif /* OPENCL_H_ */
