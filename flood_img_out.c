#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <omp.h>

#include "list/list.h"

// Define the size of the event array
#define EVENT_SIZE 8*8+2
#define ENERGY_SIZE 8*8
#define MAP_SIZE 1280

// 64 coordinates
float xposition[] = {7, 5, 3, 1, -1, -3, -5, -7, 7, 5, 3, 1, -1, -3, -5, -7, 7, 5, 3, 1, -1, -3, -5, -7, 7, 5, 3, 1, -1, -3, -5, -7, 7, 5, 3, 1, -1, -3, -5, -7, 7, 5, 3, 1, -1, -3, -5, -7, 7, 5, 3, 1, -1, -3, -5, -7, 7, 5, 3, 1, -1, -3, -5, -7};
float yposition[] = {7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 5, 5, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -3, -3, -3, -3, -3, -3, -3, -3, -5, -5, -5, -5, -5, -5, -5, -5, -7, -7, -7, -7, -7, -7, -7, -7};


off_t get_file_size(char *file_name)
{
	int ret;
	int fd = -1;
	struct stat file_stat;

	fd = open(file_name, O_RDONLY);	// 打开文件
	if (fd == -1) {
		printf("Open file %s failed : %s\n", file_name, strerror(errno));
		return -1;
	}
	ret = fstat(fd, &file_stat);	// 获取文件状态
	if (ret == -1) {
		printf("Get file %s stat failed:%s\n", file_name, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);
	return file_stat.st_size;
}

void hadamard_product(float *a, float *b, float *c, int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
    }
}

float sum_array(float *a, int n) {
    float sum = 0;
    int i;
    #pragma omp parallel for reduction(+:sum) schedule(static, CHUNK_SIZE)
    for (i = 0; i < n - 4; i += 4) {
        sum += a[i] + a[i + 1] + a[i + 2] + a[i + 3];
    }
    for (; i < n; i++) {
        sum += a[i];
    }
    return sum;
}


int main() {
    char* path = "../data/data0423_group_2_4test.ldat";
    char* save_1 = "../data/0-63/bin/events-0-63.bin";
    char* save_2 = "../data/256-319/bin/events-256-319.bin";

    if(access(save_1, 0) == -1 || access(save_2, 0) == -1) {
        printf("Error: Event data not prepared yet\n");
        return -1;
    }
    FILE *fp_1 = fopen(save_1, "rb");
    FILE *fp_2 = fopen(save_2, "rb");

    int line;
    char buffer[66*4];
    float time, e;
    float energy[64], x_sum[64], y_sum[64];
    List x_list, y_list;
    init_list(&x_list);
    init_list(&y_list);
    while(line = fread(buffer, 66*4, 1, fp_1)) {
        time = *(float*)buffer;
        e = *(float*)(buffer + 4);
        memset(energy, 0, sizeof(energy));
        memset(x_sum, 0, sizeof(x_sum));
        memset(y_sum, 0, sizeof(y_sum));
        for(int i = 0; i < 64; i++) {
            energy[i] = *(float*)(buffer + (i+2)*4);
        }
        hadamard_product(energy, xposition, x_sum, 64);
        hadamard_product(energy, yposition, y_sum, 64);
        float x = sum_array(x_sum, 64) / e;
        float y = sum_array(y_sum, 64) / e;
        append(&x_list, x);
        append(&y_list, y);
    }
    fclose(fp_1);
    while(line = fread(buffer, 66*4, 1, fp_2)) {
        time = *(float*)buffer;
        e = *(float*)(buffer + 4);
        memset(energy, 0, sizeof(energy));
        memset(x_sum, 0, sizeof(x_sum));
        memset(y_sum, 0, sizeof(y_sum));
        for(int i = 0; i < 64; i++) {
            energy[i] = *(float*)(buffer + (i+2)*4);
        }
        hadamard_product(energy, xposition, x_sum, 64);
        hadamard_product(energy, yposition, y_sum, 64);
        float x = sum_array(x_sum, 64) / e;
        float y = sum_array(y_sum, 64) / e;
        append(&x_list, x);
        append(&y_list, y);
    }
    fclose(fp_2);

    int heatmap[MAP_SIZE][MAP_SIZE];
    memset(heatmap, 0, sizeof(heatmap));
    char *plt_path = "./plt/";
    struct stat st = {0};
    if (stat(plt_path, &st) == -1) {
        mkdir(plt_path, 0700);
    }
    char *dat_path = "./plt/heatmap.dat";
    FILE *fp_data = fopen(dat_path, "w");
    // Traverse x_list and y_list to perform Min-Max Normalization
    int max_cnt = -1;
    for(int i = 0; i < x_list.size; i++) {
        int x_ = (int)(((x_list.data[i]-x_list.minn)/(x_list.maxn-x_list.minn)) * 1000);
        int y_ = (int)(((y_list.data[i]-y_list.minn)/(y_list.maxn-y_list.minn)) * 1000);
        // int x_ = (int)(((x_list.maxn-x_list.data[i])/x_list.maxn) * MAP_SIZE);
        // int y_ = (int)(((y_list.maxn-y_list.data[i])/y_list.maxn) * MAP_SIZE);
        heatmap[x_+140][y_+140]++;
        if(heatmap[x_][y_] > max_cnt) {
            max_cnt = heatmap[x_][y_];
        }
    }
    for(int i = 0; i < MAP_SIZE; i++) {
        for(int j = 0; j < MAP_SIZE; j++) {
            fprintf(fp_data, "%d ", heatmap[i][j]);
        }
        fprintf(fp_data, "\n");
    }
    fclose(fp_data);

    return 0;
}