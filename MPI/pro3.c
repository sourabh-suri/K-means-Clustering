#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>
#include <string.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"


//int dis(int a, int b, int c, int x, int y, int z );

int Clusterassign(int r,int g, int b, int cluster_r[],int cluster_g[], int cluster_b[] , int K);

void centr(int centroid_r[], int centroid_g[], int  centroid_b[]);

MPI_Status stat;

 
int main(int argc, char** argv){


	int width, height, channels, size;
    char *file = argv[1];
    int K= atoi(argv[2]);
    char str[40]="out";
    sprintf(str,"%s_%d_",str,K);
    strcat(str,file);
    printf("%s\n",str);
	unsigned char *img = stbi_load(file, &width, &height, &channels, 0);
	if(img == NULL) {
          printf("Error in loading the image\n");
         exit(1);
         }
   //  printf("image width of %dpx, a height of %dpx and %d channels\n", width,height, channels); 
	unsigned char *beg= img;
	size_t img_size = width * height * channels;
	size= width * height;
	int tag=200;
	
	
	int *R[height], *G[height],*B[height];
        for(int m=0;m< height;m++)
           R[m] = (int *)malloc(width * sizeof(int));
        for(int m=0;m< height;m++)
           G[m] = (int *)malloc(width * sizeof(int));
        for(int m=0;m< height;m++)
           B[m] = (int *)malloc(width * sizeof(int));
        
       
           
        for(int i=0; i< height ;i++){
           for(int j=0; j< width ;j++){
              R[i][j] = (int) *img;
              G[i][j] = (int) *(img+1);
              B[i][j] = (int) *(img+2);
              
              img += 3; 
              }
        }
    
        img = beg;
     printf("size %d\n", size);
     MPI_Init(&argc, &argv);
     int num_proc, my_rank;
    
     MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
     MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    // printf("%d %d\n", num_proc, my_rank);
   
   // start=MPI_Wtime();
      unsigned int b= time(0);
      //printf("time %x\n",b);
      
      
      
      /*************************************Master code ***********************************************/
    if(my_rank==0)
    {
    	//int *map[height];
    	printf("Master node!\n");
    	//int assignment[size];
	//for(int m=0;m< height;m++)
         //  map[m] = (int *)malloc(width * sizeof(int));
        int map[height][width]; 
        printf("Master node!\n"); 
    	int ppnode = size/(num_proc-1);
    
    	int centroid_r[K], centroid_g[K], centroid_b[K];
    	for(int i=0;i<K;i++)
    	{
    		int random_i = rand_r(&b)%height;
    		int random_j = rand_r(&b)%width; 
    		//printf("%d %d \n", random_i,random_j);
    		centroid_r[i] = R[random_i][random_j];
    		centroid_g[i] = G[random_i][random_j];
    		centroid_b[i] = B[random_i][random_j];
    		printf("%d %d %d\n", centroid_r[i], centroid_g[i], centroid_b[i]);
    		// printf("%d  ", cluster_points[i]);
    	}
    	//centr(centroid_r, centroid_g, centroid_b);
    	printf("\n");
    	//printf("%d %d %d\n", centroid_r[0], centroid_g[0], centroid_b[0]);
    	
    	int start_index=0 , end_index=-1;
    	for(int j=1;j<num_proc;j++)
    	{
    		start_index = (j-1)*height/(num_proc-1);
    		//end_index = (j+1)*ppnode-1;
    		//int num_indices = end_index- start_index+1;
    		MPI_Send(&start_index, 1, MPI_INT, j, 11, MPI_COMM_WORLD); //starting point tag 11
    		//MPI_Send(&end_index, 1, MPI_INT, j+1, tag, MPI_COMM_WORLD);
               printf("start: %d \n",start_index);
    	}
    	int cluster_sum_r[K], cluster_sum_g[K] , cluster_sum_b[K] ;    	
    	int cluster_count[K] , count[K];
    	int sum_r[K], sum_g[K], sum_b[K];
    	
    	
    	/*******************************************looping********************************************/
    	
    	for(int iter=0;iter<50;iter++)
    	{
		for(int j=0 ; j< K; j++){
	    	cluster_sum_r[j]=0;
	    	cluster_sum_g[j]=0;
	    	cluster_sum_b[j]=0;
	    	cluster_count[j]=0;
	    	count[j]=0;
	    	sum_r[j]=0;
	    	sum_g[j]=0;
	    	sum_b[j]=0;	    	
       	}
       	       	    	

    	for (int proc = 1; proc < num_proc; proc++){
    	MPI_Send(&centroid_r[0], K, MPI_INT, proc,21, MPI_COMM_WORLD);
    	MPI_Send(&centroid_g[0], K, MPI_INT, proc,22, MPI_COMM_WORLD);
    	MPI_Send(&centroid_b[0], K, MPI_INT, proc,23, MPI_COMM_WORLD);
    	
    	}
    	
	  printf("iter: %d\n",iter);  	
  
    	   for (int proc = 1; proc < num_proc; proc++)
    	{
    		
    		MPI_Recv(&sum_r[0], K, MPI_INT, proc, 12, MPI_COMM_WORLD, &stat);
		MPI_Recv(&sum_g[0], K, MPI_INT, proc, 13, MPI_COMM_WORLD, &stat);
		MPI_Recv(&sum_b[0], K, MPI_INT, proc, 14, MPI_COMM_WORLD, &stat);
		MPI_Recv(&count[0], K, MPI_INT, proc, 15, MPI_COMM_WORLD, &stat);    		
    		
    		for(int i=0;i<K;i++)
	    	{
	    		
    		// printf(" master: %d %d %d %d\n",i,proc, sum_r[i],  count[i]);

	    	 	cluster_sum_r[i] +=sum_r[i];
	    	 	cluster_sum_g[i] +=sum_g[i];
	    	 	cluster_sum_b[i] +=sum_b[i];
	    	 	cluster_count[i] +=count[i];
	    	 	
	    	 	// printf(" rank: %d %d %d %d\n",proc,i, sum_r[i], count[i]);
		    }

        }
     //   for(int i=0;i<K;i++)
	//	printf(" fsd: %d %d %d\n",i, cluster_sum_r[i], cluster_count[i]);
		
		 for(int i=0;i<K;i++)
    	 {
    	 		centroid_r[i] = cluster_sum_r[i]/(1+cluster_count[i]);
    	 		centroid_g[i] = cluster_sum_g[i]/(1+cluster_count[i]);
    	 		centroid_b[i] = cluster_sum_b[i]/(1+cluster_count[i]);
    	 		// printf("%d : %d %d %d\n", i ,centroid_r[i], centroid_g[i], centroid_b[i]);
    	 }

        //  printf("count %d val %d\n",cluster_count[0],centroid_r[0]);
    	 if(iter==49)
    	 {
    	         int qm= size/ (num_proc-1);
           
           int qhm= height / (num_proc -1);
    	        int off=0;
    	       
    	 	int sti, endi ;
    	 	for(int i=1;i<num_proc;i++){
	    	 	MPI_Recv(&sti, 1, MPI_INT, i, 45, MPI_COMM_WORLD, &stat);
	    	 	
	    	 	MPI_Recv(&map[sti][0], ppnode, MPI_INT, i, tag, MPI_COMM_WORLD, &stat);
	    	 	
	    	 	}
	    	 	
	    	 printf("Recieved:\n");
	    	unsigned char *kx= malloc(img_size);
	    	unsigned char *kb=kx;
                int xc;
               for(int i=0; i< height ;i++){    
               for(int j=0; j< width ;j++){
                   
                   xc=map[i][j];
                   if( xc>15 || xc<0){
                   xc=0; }
                  /* R[i][j] =  centroid_r[xc];
                   G[i][j] =  centroid_g[xc];
                   B[i][j] =  centroid_b[xc];*/
                   
                   *kx     = (uint8_t)centroid_r[xc];
                   *(kx+1) = (uint8_t)centroid_g[xc];
                   *(kx+2) = (uint8_t)centroid_b[xc]; 
                         
                   kx += 3;
                  // printf("%d ", map[i][j]); 
               }
            }         
         
          kx=kb;
   
   
          stbi_write_jpg(str, width, height, channels , kx, 100);
         
         
       // free(kx);
 
    	 }



    	}
    	printf("Answer:\n");
    	for(int i=0;i<K;i++)
    	{
    		printf("%d %d %d\n", centroid_r[i], centroid_g[i], centroid_b[i]);
    	}
    	// MPI_Recv(&sum, sizeof(int *), MPI_INT, 1, 200, MPI_COMM_WORLD, &stat);
    	// printf("%d at Master\n", sum);
    }
  /***********************************************slave_code************************************************/
    else
    {
           int q= size/ (num_proc-1);
           
           int qh= height / (num_proc -1);
                      
           int map1[qh][width];
           int start;
           
           MPI_Recv(&start, 1, MPI_INT, 0, 11, MPI_COMM_WORLD, &stat);
          // printf("q=%d qh=%d \n",start,qh);
           int end=start + qh-1;
           printf("In slave: %d start: %d end; %d\n ", my_rank,start,end);
           
           int centroid_r[K], centroid_g[K], centroid_b[K];
           int slave_sum_r[K] ,slave_sum_g[K] ,slave_sum_b[K];
           
           int slave_count[K];
    		    		
    	for(int iter=0;iter<50;iter++)
    	{
	   
	    	for(int j=0 ; j< K; j++){
	    	slave_sum_r[j]=0;
	    	slave_sum_g[j]=0;
	    	slave_sum_b[j]=0;
	    	slave_count[j]=0;
       	}
       	int c=0;
	    	//for(int j=0 ; j< K; j++)
	    	 //printf(" before: %d %d  %d\n",my_rank, slave_sum_r[j], slave_count[j]);	    	
	    	MPI_Recv(&centroid_r[0], K, MPI_INT, 0, 21, MPI_COMM_WORLD ,&stat);
	    	MPI_Recv(&centroid_g[0], K, MPI_INT, 0, 22, MPI_COMM_WORLD ,&stat);
	    	MPI_Recv(&centroid_b[0], K, MPI_INT, 0, 23, MPI_COMM_WORLD ,&stat);
	    	
	    	
	    	for(int i=start ;i <= end ;i++){
	    	for(int j=0;j<width;j++){	    	
	    	
	    		// has to be changed
	    		// printf("Assigning cluster to %d %d %d...\n", r[i], g[i], b[i]);
	    		 c = Clusterassign(R[i][j],G[i][j],B[i][j], centroid_r,centroid_g,centroid_b, K);
	    		//printf(" %d ",R[i+start][j]);
	    		map1[i-start][j]= c;
	    		//printf("%d ",map1[i-start][j]);
	    		slave_sum_r[c] += (R[i][j]);
	    		slave_sum_g[c] += (G[i][j]);
	    		slave_sum_b[c] += (B[i][j]);
	    		
	    		slave_count[c]++;
	    		//printf(" c: %d %d %d %d\n",c,my_rank ,slave_sum_r[c],slave_count[c]);
	    	}
	    	
	    	
	    	}
	    	 	MPI_Send(&slave_sum_r[0], K, MPI_INT, 0, 12, MPI_COMM_WORLD);
	    	 	MPI_Send(&slave_sum_g[0], K, MPI_INT, 0, 13, MPI_COMM_WORLD);
	    	 	MPI_Send(&slave_sum_b[0], K, MPI_INT, 0, 14, MPI_COMM_WORLD);

	    	 	MPI_Send(&slave_count[0], K, MPI_INT, 0, 15, MPI_COMM_WORLD);

	    	if(iter==49)
	    	{
	    		MPI_Send(&start, 1, MPI_INT, 0, 45, MPI_COMM_WORLD);
	    		//MPI_Send(&end, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
	    		MPI_Send(&map1[0][0], q , MPI_INT, 0, tag, MPI_COMM_WORLD);
	    			    		
	    		printf("sent\n");
	    	} 
	       
          }
 		
	}
	
	
	
	MPI_Finalize();

 
 
 for(int h=0;h<height;h++){
       free(R[h]);
       free(G[h]);
       free(B[h]);}

}

 


int Clusterassign(int r,int g, int b, int cluster_r[],int cluster_g[], int cluster_b[] , int K){	

	int min=200000, min_ind;
	for(int j=0;j<K;j++){
		int dist = pow((r-cluster_r[j]),2)+pow((g-cluster_g[j]),2)+ pow((b-cluster_b[j]),2);//dis(r, g, b, cluster_points_r[j], cluster_points_g[j], cluster_points_b[j]);
		if(dist <min){
			min=dist;
			min_ind = j;
		}
	}
	return min_ind;
}

