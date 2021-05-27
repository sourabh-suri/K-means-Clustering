#include"headerfiles.h"
#include"functionfiles.h"


// Class object from opencv: 3D Input Image
Mat input_image;

//2D Vector rolled up to fit 3D input_image | size: number of pixels * 3
vector<vector<int> > rolled_2D_img;

//2D Vector to store rgb values of final centroid centres
vector<vector<int> > centroid_centres;

//Map from centroid centres' centroid_centres to cluster pixels
//It is the representative of compressed image
map<int,vector<int> > centroid_pixel_map;


int main( int argc, char** argv )
{
    int num_of_centroids, num_of_iteration;
    num_of_centroids = atoi(argv[2]);
    num_of_iteration = atoi(argv[3]);
    // Loading input image
    cout<<"[1]loading input_image...\n";
    input_image = imread(argv[1], IMREAD_COLOR);
    if(! input_image.data )
    {
        cout <<  "Error : could not find input image" << endl ;
        return -1;
    }

    // Converting mat object input_image to 2d vector rolled_2D_img
    rolling_3D_to_2D();
    // Initialising random numbers to centroid centres within image spread (W*H) range
    random_init_centroids(num_of_centroids);
    // Mapping of a given pixel to one of the centroid centres
    openmp_kmeans_clustering(num_of_centroids, num_of_iteration);
    // Estimating compression ratio
    display_compression_ratio(num_of_centroids);
    // saving K-Means clustered image for visual distinction
    //saving_clustered_image(num_of_centroids);
    cout<<"[5]saving output image...\n";
    Mat uncompressed_image(input_image.rows,input_image.cols, CV_8UC3, Scalar(0, 0, 0));
    //8U means the 8-bit Usigned integer, C3 means 3 Channels for RGB color,
    //and Scalar(0, 0, 0) is the initial value for each pixel.
    for(int i=0;i<num_of_centroids;i++)
    {
        for(int j=0;j<centroid_pixel_map[i].size();j++)
        {
            int x=centroid_pixel_map[i][j] / input_image.cols;
            int y=centroid_pixel_map[i][j] % input_image.cols;
            Vec3b intensity = uncompressed_image.at<Vec3b>(x,y);
            intensity.val[0]=(uchar)centroid_centres[i][0];
            intensity.val[1]=(uchar)centroid_centres[i][1];
            intensity.val[2]=(uchar)centroid_centres[i][2];
            uncompressed_image.at<Vec3b>(x,y) = intensity;
        }
    }
    imwrite(remove_extension(argv[1]).append("_openmp_uncompressed.jpg"),uncompressed_image);
    cout<<"[6]end...\n";
    return 0;
}
