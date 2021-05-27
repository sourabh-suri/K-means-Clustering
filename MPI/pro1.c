#include <stdio.h>
#include <stdlib.h>
//#include <math.h>
//#include <mpi.h>
#include <time.h>



#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"
 
int main(void) {
 
 
   int width, height, channels;
      unsigned char *img = stbi_load("lbj.jpg", &width, &height, &channels, 0);
      if(img == NULL) {
          printf("Error in loading the image\n");
         exit(1);
         }
     printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels); 
 
    size_t img_size = width * height * channels;

    unsigned char *beg= img;

 
 int *R[height], *G[height],*B[height];
    for(int m=0;m< height;m++)
       R[m] = (int *)malloc(width * sizeof(int));
    for(int m=0;m< height;m++)
       G[m] = (int *)malloc(width * sizeof(int));
    for(int m=0;m< height;m++)
       B[m] = (int *)malloc(width * sizeof(int));
       
           
 for(int i=0; i< height ;i++){
    
    for(int j=0; j< width ;j++){
      
      R[i][j] = (uint8_t) *img;
      G[i][j] = (uint8_t) *(img+1);
      B[i][j] = (uint8_t) *(img+2);
      
      img += 3; 
   
   
    }}
    
   img = beg;
    
  
   
   unsigned char *k= malloc(img_size);
   unsigned char *kb=k;
   for(int i=0; i< height ;i++){
    
    for(int j=0; j< width ;j++){
      
      *k = (uint8_t) R[i][j] ;
      *(k+1) = (uint8_t) G[i][j] ;
      *(k+2) = (uint8_t) B[i][j] ;
        k += 3;   
     
    }}
   
   k=kb;
   
   
 stbi_write_jpg("lbj_x.jpg", width, height, 3 , k, 100);
 
 srand(time(0));
 int K=16;
 int centroid_r[K], centroid_g[K], centroid_b[K];
    	for(int i=0;i<K;i++)
    	{
    		int random_i = rand()%height;
    		int random_j = rand()%width; 
    		centroid_r[i] = R[random_i][random_j];
    		centroid_g[i] = G[random_i][random_j];
    		centroid_b[i] = B[random_i][random_j];
    		printf("%d %d %d\n", centroid_r[i], centroid_g[i], centroid_b[i]);
    		// printf("%d  ", cluster_points[i]);
    	}
 
 /*
  for(int i=1000; i<1015; i++){
   for(int j=150; j<165; j++){
    printf("%d  ",B[i][j]);
    }
    printf("\n");
    }*/
 
 
 for(int h=0;h<height;h++){
free(R[h]);
free(G[h]);
free(B[h]);}
 
 
 
 
 
 
 }
 
 
 
 
 
 
 
 
 
 
     
 //    stbi_write_jpg("lbj_sepia.jpg", width, height, 3 , sepia_img, 100);
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 









 
 
 
 
 
 
 
 

