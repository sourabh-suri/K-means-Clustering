#include"headerfiles.h"
#include"cudafiles.h"
// Number of threads
#define BLOCK_SIZE 16
#define GRID_SIZE 256
// Class object from opencv: 3D Input Image
Mat input_image;

// num_of_centroids and size on gpu
__constant__ int gpu_num_of_centroids;
__constant__ int gpu_size;

//R,G,B Centroid's triple on gpu
__constant__ int gpu_red_centroid[20];
__constant__ int gpu_green_centroid[20];
__constant__ int gpu_blue_centroid[20];


int size_for_centroids = 0; // num_of_centroids * sizeof(int)
int size_image = 0;  // width * height * sizeof(int)


__global__ void sum_pixels_in_cluster(int *gpu_red,int *gpu_green,int *gpu_blue,int *gpu_sum_red,int *gpu_sum_green,int *gpu_sum_blue,int *gpu_cluster_array,int * gpu_points_in_cluster)
 {
 	int threadID = (threadIdx.x + blockIdx.x * blockDim.x) + (threadIdx.y + blockIdx.y * blockDim.y) * blockDim.x * gridDim.x;


 	if(threadID < gpu_size) {
 		int selected_cluster_array = gpu_cluster_array[threadID];
 		int selected_red_val = gpu_red[threadID];
 		int selected_green_val = gpu_green[threadID];
 		int selected_blue_val = gpu_blue[threadID];
 		atomicAdd(&gpu_sum_red[selected_cluster_array], selected_red_val);
 		atomicAdd(&gpu_sum_green[selected_cluster_array], selected_green_val);
 		atomicAdd(&gpu_sum_blue[selected_cluster_array], selected_blue_val);
 		atomicAdd(& gpu_points_in_cluster[selected_cluster_array], 1);
 	}

__global__ void update_centroids(int *gpu_update_red_centroid, int *gpu_update_green_centroid, int *gpu_update_blue_centroid,int* gpu_sum_red, int *gpu_sum_green,int *gpu_sum_blue, int*  gpu_points_in_cluster,int *gpu_flag)
{

 	int threadID = threadIdx.x + threadIdx.y * blockDim.x;
 	if(threadID < gpu_num_of_centroids)
  {
 		int points_in_selected_cluster =  gpu_points_in_cluster[threadID];
 		int sum_red = gpu_sum_red[threadID];
 		int sum_green = gpu_sum_green[threadID];
 		int sum_blue = gpu_sum_blue[threadID];

	gpu_update_red_centroid[threadID] = (int)(sum_red/points_in_selected_cluster);

 		gpu_update_green_centroid[threadID] = (int)(sum_green/points_in_selected_cluster);
 		gpu_update_blue_centroid[threadID] = (int)(sum_blue/points_in_selected_cluster);

 		if(gpu_update_green_centroid[threadID]!=gpu_green_centroid[threadID] || gpu_update_red_centroid[threadID]!=gpu_red_centroid[threadID] || gpu_update_blue_centroid[threadID]!=gpu_blue_centroid[threadID])
 		*gpu_flag=1;
 	}
}
__global__ void pop_gpu_arrays(int *gpu_sum_red,int *gpu_sum_green,int *gpu_sum_blue, int*  gpu_points_in_cluster, int* gpu_update_red_centroid, int* gpu_update_green_centroid, int* gpu_update_blue_centroid )
{

 	int threadID = threadIdx.x + threadIdx.y * blockDim.x;
 	if(threadID < gpu_num_of_centroids)
  {
 		// num_of_centroids long
 		gpu_sum_red[threadID] = 0;
 		gpu_sum_green[threadID] = 0;
 		gpu_sum_blue[threadID] = 0;
 		gpu_update_red_centroid[threadID] = 0;
 		gpu_update_green_centroid[threadID] = 0;
 		gpu_update_blue_centroid[threadID] = 0;
    gpu_points_in_cluster[threadID] = 0;
 	}
}
__global__ void pop_gpu_cluster_array(int *gpu_cluster_array)
{

	int threadID = (threadIdx.x + blockIdx.x * blockDim.x) + (threadIdx.y + blockIdx.y * blockDim.y) * blockDim.x * gridDim.x;

	if(threadID < gpu_size) {
		gpu_cluster_array[threadID] = 0;
	}
}



__global__ void get_cluster_points(int *gpu_red,int *gpu_green,int *gpu_blue,int *gpu_cluster_array)
{

 	int threadID = (threadIdx.x + blockIdx.x * blockDim.x) + (threadIdx.y + blockIdx.y * blockDim.y) * blockDim.x * gridDim.x;

 	//default min value of distance
 	float min = 1000.0, value;
 	int index = 0;
 	if(threadID < gpu_size) {

 		for(int i = 0; i < gpu_num_of_centroids; i++) {

 			value = sqrtf(powf((gpu_red[threadID]-gpu_red_centroid[i]),2.0) + powf((gpu_green[threadID]-gpu_green_centroid[i]),2.0) + powf((gpu_blue[threadID]-gpu_blue_centroid[i]),2.0));
 			if(value < min){
 				// saving new nearest centroid
 				min = value;
 				// Updating his index
 				index = i;
 			}
 		}
 		gpu_cluster_array[threadID] = index;
 	}
}





int main(int argc, char *argv[])
{
		cudaSetDevice(0);
		int *r, *g, *b, *cpu_red_centroid, *cpu_green_centroid, *cpu_blue_centroid;
		int *gpu_red, *gpu_green, *gpu_blue, *gpu_update_red_centroid, *gpu_update_green_centroid, *gpu_update_blue_centroid;
		int *cpu_cluster_Array, *gpu_cluster_array;
    int *gpu_flag, cpu_flag;
		int *sum_red, *sum_green, *sum_blue;
		int *gpu_sum_red, *gpu_sum_green, *gpu_sum_blue;
    int width, height, num_of_centroids, num_of_iteration,size;
    int *cpu_points_in_cluster, *gpu_points_in_cluster;

  // Reading the command line arguments
  printf("[1]loading image...\n");
  input_image = imread(argv[1], IMREAD_COLOR);
  if(! input_image.data )
  {
  cout <<  "Could not open or find the image" << std::endl ;
  return -1;
  }
	num_of_centroids = atoi(argv[2]);
	num_of_iteration = atoi(argv[3]);

	// Allocating memory on CPU
  width = input_image.cols;
  height = input_image.rows;
	size_image = width * height * sizeof(int);
	size_for_centroids = num_of_centroids * sizeof(int);
	size = width * height;
  // 1D pointer arrays
	r = (int*)(malloc(size_image));
	g = (int*)(malloc(size_image));
	b = (int*)(malloc(size_image));
	cpu_red_centroid = (int*)(malloc(size_for_centroids));
	cpu_green_centroid = (int*)(malloc(size_for_centroids));
	cpu_blue_centroid = (int*)(malloc(size_for_centroids));
	cpu_cluster_Array = (int*)(malloc(size_image)); //stores the cluster number for each pixel
	sum_red = (int*)(malloc(size_for_centroids));
	sum_green = (int*)(malloc(size_for_centroids));
	sum_blue = (int*)(malloc(size_for_centroids));
	cpu_points_in_cluster = (int*)(malloc(size_for_centroids));


  // 3D Image spread to three 1D pointer arrays
  for(int i=0;i<input_image.rows;i++)
   {
     for(int j=0;j<input_image.cols;j++)
     {
       Vec3b intensity = input_image.at<Vec3b>(i,j);
       *(b + i*input_image.cols + j) = (int)intensity.val[0];
       *(g + i*input_image.cols + j) = (int)intensity.val[1];
       *(r + i*input_image.cols + j) = (int)intensity.val[2];
     }
  }

	// Setting initial centroids
	initialise_centroids(num_of_centroids, cpu_red_centroid, cpu_green_centroid, cpu_blue_centroid,r,g,b,size);

  // Allocating memory on GPU
  cudaMalloc((void**) &gpu_sum_red, size_for_centroids);
  cudaMalloc((void**) &gpu_sum_green, size_for_centroids);
  cudaMalloc((void**) &gpu_sum_blue, size_for_centroids);
  cudaMalloc((void**) &gpu_update_red_centroid, size_for_centroids);
  cudaMalloc((void**) &gpu_update_green_centroid, size_for_centroids);
  cudaMalloc((void**) &gpu_update_blue_centroid, size_for_centroids);
  cudaMalloc((void**) &gpu_cluster_array, size_image);
  cudaMalloc((void**) &gpu_red, size_image);
  cudaMalloc((void**) &gpu_green, size_image);
  cudaMalloc((void**) &gpu_blue, size_image);
  cudaMalloc((void**) & gpu_points_in_cluster, size_for_centroids);
  cudaMalloc((void**) &gpu_flag, sizeof(int));

  	// copy CPU memory to GPU
  cudaMemcpyToSymbol(gpu_red_centroid, cpu_red_centroid, size_for_centroids);
  cudaMemcpyToSymbol(gpu_green_centroid, cpu_green_centroid, size_for_centroids);
  cudaMemcpyToSymbol(gpu_blue_centroid, cpu_blue_centroid, size_for_centroids);
  cudaMemcpy(gpu_update_red_centroid, cpu_red_centroid,size_for_centroids,cudaMemcpyHostToDevice);
  cudaMemcpy(gpu_update_green_centroid, cpu_green_centroid,size_for_centroids,cudaMemcpyHostToDevice);
  cudaMemcpy(gpu_update_blue_centroid, cpu_blue_centroid,size_for_centroids,cudaMemcpyHostToDevice);
  cudaMemcpy(gpu_cluster_array, cpu_cluster_Array, size_image, cudaMemcpyHostToDevice);
  cudaMemcpy(gpu_flag,&cpu_flag,sizeof(int),cudaMemcpyHostToDevice);
  cudaMemcpy( gpu_points_in_cluster, cpu_points_in_cluster, size_for_centroids, cudaMemcpyHostToDevice);
  cudaMemcpy(gpu_red, r, size_image, cudaMemcpyHostToDevice);
  cudaMemcpy(gpu_green, g, size_image, cudaMemcpyHostToDevice);
  cudaMemcpy(gpu_blue, b, size_image, cudaMemcpyHostToDevice);
  cudaMemcpyToSymbol(gpu_num_of_centroids,&num_of_centroids, sizeof(int));
  cudaMemcpyToSymbol(gpu_size, &size, sizeof(int));
  	// Clearing centroids on gpu
	for(int i = 0; i < num_of_centroids; i++)
  {
			cpu_red_centroid[i] = 0;
			cpu_green_centroid[i] = 0;
			cpu_blue_centroid[i] = 0;
		}

		// Defining grid size
		int BLOCK_X, BLOCK_Y;
		BLOCK_X = ceil(width/BLOCK_SIZE);
		BLOCK_Y = ceil(height/BLOCK_SIZE);
		if(BLOCK_X > GRID_SIZE)
			BLOCK_X = GRID_SIZE;
		if(BLOCK_Y > GRID_SIZE)
			BLOCK_Y = GRID_SIZE;

	 	dim3 dimGRID(BLOCK_X,BLOCK_Y);
		dim3 dimBLOCK(BLOCK_SIZE,BLOCK_SIZE);

		printf("[3]launching K-Means Kernels..	\n");
		//Iteration of kmeans algorithm
		int num_iterations;
		for(int i = 0; i < num_of_iteration; i++)
  {
			num_iterations = i;
			cpu_flag=0;
			cudaMemcpy(gpu_flag,&cpu_flag,sizeof(int),cudaMemcpyHostToDevice);
			pop_gpu_arrays<<<1, dimBLOCK>>>(gpu_sum_red, gpu_sum_green, gpu_sum_blue,  gpu_points_in_cluster, gpu_update_red_centroid, gpu_update_green_centroid, gpu_update_blue_centroid);
			pop_gpu_cluster_array<<<dimGRID, dimBLOCK>>>(gpu_cluster_array);
			get_cluster_points<<< dimGRID, dimBLOCK >>> (gpu_red, gpu_green, gpu_blue,gpu_cluster_array);
			sum_pixels_in_cluster<<<dimGRID, dimBLOCK>>> (gpu_red, gpu_green, gpu_blue, gpu_sum_red, gpu_sum_green, gpu_sum_blue, gpu_cluster_array, gpu_points_in_cluster);
			update_centroids<<<1,dimBLOCK >>>(gpu_update_red_centroid, gpu_update_green_centroid, gpu_update_blue_centroid, gpu_sum_red, gpu_sum_green, gpu_sum_blue,  gpu_points_in_cluster,gpu_flag);
      cudaMemcpy(cpu_red_centroid, gpu_update_red_centroid, size_for_centroids,cudaMemcpyDeviceToHost);
      cudaMemcpy(cpu_green_centroid, gpu_update_green_centroid, size_for_centroids,cudaMemcpyDeviceToHost);
      cudaMemcpy(cpu_blue_centroid, gpu_update_blue_centroid, size_for_centroids,cudaMemcpyDeviceToHost);
      cudaMemcpy(&cpu_flag, gpu_flag,sizeof(int),cudaMemcpyDeviceToHost);
      cudaMemcpyToSymbol(gpu_red_centroid, cpu_red_centroid, size_for_centroids);
      cudaMemcpyToSymbol(gpu_green_centroid, cpu_green_centroid, size_for_centroids);
      cudaMemcpyToSymbol(gpu_blue_centroid, cpu_blue_centroid, size_for_centroids);
			if(cpu_flag==0)
				break;
		}

	 cudaMemcpy(cpu_cluster_Array, gpu_cluster_array, size_image, cudaMemcpyDeviceToHost);
	 cudaMemcpy(cpu_points_in_cluster,  gpu_points_in_cluster, size_for_centroids, cudaMemcpyDeviceToHost);

	printf("\tConverged in %d iterations.\n",num_iterations);
  // Estimating compression ratio
  display_compression_ratio(num_of_centroids);
  Mat uncompressed_image(input_image.rows,input_image.cols, CV_8UC3, Scalar(0, 0, 0));
   //8U means the 8-bit Usigned integer, C3 means 3 Channels for RGB color,
   //and Scalar(0, 0, 0) is the initial value for each pixel.
 	for (int i = 0; i < size; i++)
   {
     int x = i / input_image.cols;
     int y = i % input_image.cols;
     Vec3b intensity = uncompressed_image.at<Vec3b>(x,y);
     intensity.val[0]=cpu_blue_centroid[cpu_cluster_Array[i]];
     intensity.val[1]=cpu_green_centroid[cpu_cluster_Array[i]];
     intensity.val[2]=cpu_red_centroid[cpu_cluster_Array[i]];
     uncompressed_image.at<Vec3b>(x,y) = intensity;
   }
 	printf("[5]saving image...	\n");
  imwrite(remove_extension(argv[1]).append("cuda_uncompressed.jpg"),uncompressed_image);

	free(r);
	free(g);
	free(b);
	free(cpu_red_centroid);
	free(cpu_green_centroid);
	free(cpu_blue_centroid);
	free(cpu_cluster_Array);
	free(sum_red);
	free(sum_green);
	free(sum_blue);
	free(cpu_points_in_cluster);

	 cudaFree(gpu_red);
	 cudaFree(gpu_green);
	 cudaFree(gpu_blue);
	 cudaFree(gpu_update_red_centroid);
	 cudaFree(gpu_update_green_centroid);
	 cudaFree(gpu_update_blue_centroid);
	 cudaFree(gpu_cluster_array);
	 cudaFree(gpu_sum_red);
	 cudaFree(gpu_sum_green);
	 cudaFree(gpu_sum_blue);
	 cudaFree( gpu_points_in_cluster);

	printf("[6]end...\n");
	return 0;
}
