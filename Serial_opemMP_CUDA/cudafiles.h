
// Class object from opencv: 3D Input Image
extern Mat input_image;

// Function to display and sstimate compression ratio
void display_compression_ratio(int num_of_centroids)
{
    cout<<"[4]estimating compression...\n";
    float Original_size = (8*3*sizeof(uchar)*input_image.rows*input_image.cols);
    float Compressed_size = ((log(num_of_centroids)*input_image.rows*input_image.cols)+num_of_centroids*8*3*sizeof(uchar));
    cout<<"\tOriginal input_image size:"<<Original_size<<endl;
    cout<<"\tCompressed input_image size:"<<Compressed_size<<endl;
    cout<<"\tCompression Ratio:"<<Original_size/Compressed_size<<endl;
}

// Function to remove_extension of input file to use similar file name further
std::string remove_extension(const std::string& filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot);
}


// Function to calculate the euclidean distance of colour intensities
int euclidiean_distance(vector<int> a,vector<int> b)
{
    int red_selected_pixel = a[0];
    int red_cluster_pixel = b[0];
    int blue_selected_pixel = a[1];
    int blue_cluster_pixel = b[1];
    int green_selected_pixel = a[2];
    int green_cluster_pixel = b[2];
    return sqrt(pow(red_selected_pixel-red_cluster_pixel,2)+pow(blue_selected_pixel-blue_cluster_pixel,2)+pow(green_selected_pixel-green_cluster_pixel,2));
}


void initialise_centroids(int num_of_centroids, int* cpu_red_centroid, int* cpu_green_centroid, int*  cpu_blue_centroid, int r[], int g[], int b[], int size)
{
  printf("[2]initialising centroids... \n");
  for(int i=0;i<num_of_centroids;++i)
  {
   int index = rand()%size;
   cpu_red_centroid[i] = r[index];
   cpu_blue_centroid[i] = b[index];
   cpu_green_centroid[i] = g[index];
  }
}

#define CUDA_CALL(x) do { if((x) != cudaSuccess) { \
   printf("Error at %s:%d\n",__FILE__,__LINE__); \
   printf("%s\n",cudaGetErrorString(x)); \
   system("pause"); \
   return EXIT_FAILURE;}} while(0)
