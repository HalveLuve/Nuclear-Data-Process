gcc -o data_preprocess data_preprocess.c -lmatio -lm -fopenmp
./data_preprocess

gcc -o flood_img_out flood_img_out.c -fopenmp
./flood_img_out

gnuplot -persist <<EOF
load "heatmap.plt"
exit