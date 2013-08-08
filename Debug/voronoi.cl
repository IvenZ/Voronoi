__kernel void 
calculate(__global int *a,
	__global int *b,
	__global float *answer,
	unsigned int max_points,
	unsigned int size_width,
	unsigned int size_height)
{
	int gid = get_global_id(0);
	
	int point = gid % max_points;
	int width = (gid / max_points) % size_width;
	int height = ((gid / max_points / size_width)) % size_height;
	
	//printf("%f\n",sqrt(((width-a[point]) * (width-a[point])) + ((height-b[point]) * (height-b[point]))));
	
	answer[gid] = sqrt(((width-a[point]) * (width-a[point])) + ((height-b[point]) * (height-b[point])));
}