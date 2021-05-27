
// Class object from opencv: 3D Input Image
extern Mat input_image;

//2D Vector rolled up to fit 3D input_image | size: number of pixels * 3
extern vector<vector<int> > rolled_2D_img;

//2D Vector to store rgb values of final centroid centres
extern vector<vector<int> > centroid_centres;

//Map from centroid centres' centroid_centres to cluster pixels
//It is the representative of compressed image
extern map<int,vector<int> > centroid_pixel_map;

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

/*
#define CUDA_CALL(x) do { if((x) != cudaSuccess) { \
   printf("Error at %s:%d\n",__FILE__,__LINE__); \
   printf("%s\n",cudaGetErrorString(x)); \
   system("pause"); \
   return EXIT_FAILURE;}} while(0)
*/

// Function to initialise random numbers within image spread (W*H) to centroid centres
void random_init_centroids(int num_of_centroids)
{
  cout<<"[2]initialising centroids... \n";
  for(int i=0;i<num_of_centroids;i++)
  {
    int rand_index = rand()%rolled_2D_img.size();
    centroid_centres.push_back(rolled_2D_img[rand_index]);
 }
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

// Converting mat object input_image to 2d vector rolled_2D_img
void rolling_3D_to_2D()
{
    vector<int> temp;
    for(int i=0;i<input_image.rows;i++)
    {
      for(int j=0;j<input_image.cols;j++)
      {
        temp.clear();
        Vec3b intensity = input_image.at<Vec3b>(i,j);
        temp.push_back((int)intensity.val[0]);
        temp.push_back((int)intensity.val[1]);
        temp.push_back((int)intensity.val[2]);
        rolled_2D_img.push_back(temp);
      }

    }
}

// OpenMP compatible Function to map of a given pixel to one of the cluster centers i.e centroid centres
void openmp_kmeans_clustering(int num_of_centroids, int num_of_iteration)
{
  cout<<"[3]K-means centroid_pixel_maping...\n";
    //Flag set when centroids are not changing and iterations converged
    bool final_centroid_set=true;
    int iteration=0;

    while((iteration<num_of_iteration)  && final_centroid_set)
    {
        iteration++;
        centroid_pixel_map.clear();
        int min_rgb,min_rgb_index;

    // Mapping of seletec pixel data points to cluster centers
    #pragma omp parallel for private(min_rgb,min_rgb_index)
        for(int i=0;i<rolled_2D_img.size();i++)
        {
            min_rgb=INT_MAX;
            min_rgb_index=-1;
            for(int j=0;j<centroid_centres.size();j++)
            {
                // If the pixel selected is cluster center only => continue breaking the loop
                  if(j==i)
                  continue;
                  //  clustering based on euclidean distance of the colour intensities
                  int pixel_distance = euclidiean_distance(rolled_2D_img[i],centroid_centres[j]);
                  if(min_rgb>pixel_distance)
                  {
                    min_rgb=pixel_distance;
                    min_rgb_index=j;
                  }
            }
            #pragma omp critical
            {
            // Storing the identified pixel into the corresponding cluster i.e. centroid_pixel_map
                if(centroid_pixel_map.find(min_rgb_index)==centroid_pixel_map.end())
                // If not found => insert it as a new pair
                {    vector<int> v;
                    v.push_back(i);
                    centroid_pixel_map.insert(make_pair(min_rgb_index,v));
                }
                else
                centroid_pixel_map[min_rgb_index].push_back(i);
            }
        }
    // Finding new cluster centers as the mean of the clusters based on..
    // .. cluster labels for each pixel and individual pixel intensities
        for(int i=0;i<num_of_centroids;i++)
        {
            int blue=0,green=0,red=0,j;
            #pragma omp parallel for reduction(+:red,green,blue) private(j)
            for(j=0;j<centroid_pixel_map[i].size();j++)
            {
                // Summing all pixels from a cluster
                //int selected_pixel = centroid_pixel_map[i][j];
                blue+=rolled_2D_img[centroid_pixel_map[i][j]][0];
                green+=rolled_2D_img[centroid_pixel_map[i][j]][1];
                red+=rolled_2D_img[centroid_pixel_map[i][j]][2];
            }
        // Finding mean value
            vector<int> updated_mean_val;
            if(centroid_pixel_map[i].size())
            {
                updated_mean_val.push_back(blue/centroid_pixel_map[i].size());
                updated_mean_val.push_back(green/centroid_pixel_map[i].size());
                updated_mean_val.push_back(red/centroid_pixel_map[i].size());
            }
            else
            {
                int rand_index = rand()%rolled_2D_img.size();
                updated_mean_val = rolled_2D_img[rand_index];
            }
        // Updating the centroid centres with the new mean values
            for(int k=0;k<3;k++)
            {
                if(updated_mean_val[k]!=centroid_centres[i][k])
                {
                    centroid_centres[i][k]=updated_mean_val[k];
                    final_centroid_set=final_centroid_set && false;
                }
                // Condition to check if cnetroid centre have changed or already converged
                else
                  final_centroid_set=final_centroid_set && true;
            }
        }
        final_centroid_set=!final_centroid_set;


    }
    cout<<"\t Converged in "<<iteration<<" iterations\n";
}

// Function to map of a given pixel to one of the cluster centers i.e centroid centres
void kmeans_clustering(int num_of_centroids, int num_of_iteration)
{
  cout<<"[3]K-means centroid_pixel_maping...\n";
    //Flag set when centroids are not changing and iterations converged
    bool final_centroid_set=true;
    int iteration=0;

    while((iteration<num_of_iteration)  && final_centroid_set)
    {
        iteration++;
        centroid_pixel_map.clear();
        int min_rgb,min_rgb_index;
    // Mapping of seletec pixel data points to cluster centers
        for(int i=0;i<rolled_2D_img.size();i++)
        {
            min_rgb=INT_MAX;
            min_rgb_index=-1;
            for(int j=0;j<centroid_centres.size();j++)
            {
                // If the pixel selected is cluster center only => continue breaking the loop
                  if(j==i)
                  continue;
                  //  clustering based on euclidean distance of the colour intensities
                  int pixel_distance = euclidiean_distance(rolled_2D_img[i],centroid_centres[j]);
                  if(min_rgb>pixel_distance)
                  {
                    min_rgb=pixel_distance;
                    min_rgb_index=j;
                  }
            }

		// Storing the identified pixel into the corresponding cluster i.e. centroid_pixel_map
		if(centroid_pixel_map.find(min_rgb_index)==centroid_pixel_map.end())
		{    vector<int> v;
		    v.push_back(i);
		    centroid_pixel_map.insert(make_pair(min_rgb_index,v));
		}
		else
		centroid_pixel_map[min_rgb_index].push_back(i);
        }
    // Finding new cluster centers as the mean of the clusters based on..
    // .. cluster labels for each pixel and individual pixel intensities
        for(int i=0;i<num_of_centroids;i++)
        {
            int blue=0,green=0,red=0;
            for(int j=0;j<centroid_pixel_map[i].size();j++)
            {
                // Summing all pixels from a cluster
                int selected_pixel = centroid_pixel_map[i][j];
                blue+=rolled_2D_img[selected_pixel][0];
                green+=rolled_2D_img[selected_pixel][1];
                red+=rolled_2D_img[selected_pixel][2];
            }
        // Finding mean value
            vector<int> updated_mean_val;
            if(centroid_pixel_map[i].size())
            {
                updated_mean_val.push_back(blue/centroid_pixel_map[i].size());
                updated_mean_val.push_back(green/centroid_pixel_map[i].size());
                updated_mean_val.push_back(red/centroid_pixel_map[i].size());
            }
            else
            {
                int rand_index = rand()%rolled_2D_img.size();
                updated_mean_val = rolled_2D_img[rand_index];
            }
        // Updating the centroid centres with the new mean values
            for(int k=0;k<3;k++)
            {
                if(updated_mean_val[k]!=centroid_centres[i][k])
                {
                    centroid_centres[i][k]=updated_mean_val[k];
                    final_centroid_set=final_centroid_set && false;
                }
                // Condition to check if cnetroid centre have changed or already converged
                else
                  final_centroid_set=final_centroid_set && true;
            }
        }
        final_centroid_set=!final_centroid_set;


    }
    cout<<"\t Converged in "<<iteration<<" iterations\n";
}
/*
// Function to store K-Means clustered image for visual distinction
void saving_clustered_image(int num_of_centroids)
{
    cout<<"[5]saving input_image...\n";
    for(int i=0;i<num_of_centroids;i++)
    {
        for(int j=0;j<centroid_pixel_map[i].size();j++)
        {
            int x=centroid_pixel_map[i][j] / input_image.cols;
            int y=centroid_pixel_map[i][j] % input_image.cols;
            Vec3b intensity = uncompressed_input_image.at<Vec3b>(x,y);
            intensity.val[0]=(uchar)centroid_centres[i][0];
            intensity.val[1]=(uchar)centroid_centres[i][1];
            intensity.val[2]=(uchar)centroid_centres[i][2];
            uncompressed_input_image.at<Vec3b>(x,y) = intensity;
        }
    }
}
*/
