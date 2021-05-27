LC_NUMERIC=C printf "\n------------------------------------------------------------------------\n" 
echo "Parallel Image Compression using CUDA and OpenCV by K-means clustering"
echo "[0]Compiling..."
export PATH=$PATH:/usr/local/cuda/bin
nvcc cuda.cu `pkg-config --cflags --libs opencv` -o cuda_out.out
echo "------------------enter the name of the file-----------------------------"
read filename
width=$(identify -format "%w" $filename)
height=$(identify -format "%h" $filename)
res1=$(date +%s.%N)
./cuda_out.out $filename 16 100
res2=$(date +%s.%N)
dt=$(echo "$res2 - $res1" | bc)
dd=$(echo "$dt/86400" | bc)
dt2=$(echo "$dt-86400*$dd" | bc)
dh=$(echo "$dt2/3600" | bc)
dt3=$(echo "$dt2-3600*$dh" | bc)
dm=$(echo "$dt3/60" | bc)
ds=$(echo "$dt3-60*$dm" | bc)
LC_NUMERIC=C printf "\nTime Taken for CUDA Kmeans Clustering: %d:%02d:%02d:%02.4f" $dd $dh $dm $ds
LC_NUMERIC=C printf "\n------------------------------------------------------------------------\n"
echo "Parallel Image Compression using OpenMP by K-means clustering"
echo "[0]Compiling..."
export OMP_NUM_THREADS=8
g++ openmp.cpp -fopenmp `pkg-config --cflags --libs opencv` -o openmp_compress.out
echo "------------------enter the name of the file-----------------------------"
read filename
res1=$(date +%s.%N)
./openmp_compress.out $filename 16 100
res2=$(date +%s.%N)
dt=$(echo "$res2 - $res1" | bc)
dd=$(echo "$dt/86400" | bc)
dt2=$(echo "$dt-86400*$dd" | bc)
dh=$(echo "$dt2/3600" | bc)
dt3=$(echo "$dt2-3600*$dh" | bc)
dm=$(echo "$dt3/60" | bc)
ds=$(echo "$dt3-60*$dm" | bc)
LC_NUMERIC=C printf "\nTime Taken for Serial Kmeans Clustering: %d:%02d:%02d:%02.4f" $dd $dh $dm $ds
LC_NUMERIC=C printf "\n------------------------------------------------------------------------\n"

echo "Serial Image Compression using OpenCV by K-means clustering"
echo "[0]Compiling..."
g++ serial.cpp `pkg-config --cflags --libs opencv` -o serial_compress.out
echo "------------------enter the name of the file-----------------------------"
read filename
res1=$(date +%s.%N)
./serial_compress.out $filename 16 100
res2=$(date +%s.%N)
dt=$(echo "$res2 - $res1" | bc)
dd=$(echo "$dt/86400" | bc)
dt2=$(echo "$dt-86400*$dd" | bc)
dh=$(echo "$dt2/3600" | bc)
dt3=$(echo "$dt2-3600*$dh" | bc)
dm=$(echo "$dt3/60" | bc)
ds=$(echo "$dt3-60*$dm" | bc)
LC_NUMERIC=C printf "\nTime Taken for serial Kmeans Clustering: %d:%02d:%02d:%02.4f" $dd $dh $dm $ds
LC_NUMERIC=C printf "\n------------------------------------------------------------------------\n"
