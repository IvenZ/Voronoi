__kernel void 
calculate(__global int *x,
	__global int *y,
	__global float *result,
	unsigned int max_points,
	unsigned int size_width,
	unsigned int size_height)
{
	int gid = get_global_id(0);
	
	int point = gid % max_points;
	int width = (gid / max_points) % size_width;
	int height = ((gid / max_points / size_width)) % size_height;
		
	result[gid] = sqrt(((width-x[point]) * (width-x[point])) + ((height-y[point]) * (height-y[point])));
}