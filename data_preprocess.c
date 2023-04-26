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

#include <matio.h>

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


void fetch_lut(char* lut_file, double* lut) {
    mat_t *matfp = NULL;
    matvar_t *matvar = NULL;
    matfp = Mat_Open(lut_file, MAT_ACC_RDONLY);
    if(matfp == NULL) {
        printf("Failed to open LUT file\n");
        return ;
    }
    matvar = Mat_VarRead(matfp, "LUT");
    if(matvar == NULL) {
        printf("Failed to read LUT data\n");
        Mat_Close(matfp);
        return ;
    }
    memcpy(lut, matvar->data, matvar->nbytes);
    Mat_VarFree(matvar);
    Mat_Close(matfp);
}

int main() {
    // Load the LUT data into memory
    char* lut_file = "../data/LUT.mat";
    double lut[64];
    fetch_lut(lut_file, lut);

    char* path = "../data/data0423_group_2_4test.ldat";
    char* save_1 = "../data/0-63/bin";
    char* save_2 = "../data/256-319/bin";

    // Create the save directories if they don't exist
    struct stat st = {0};
    if (stat(save_1, &st) == -1) {
        mkdir(save_1, 0700);
    }
    if (stat(save_2, &st) == -1) {
        mkdir(save_2, 0700);
    }
    char save_path_1[100], save_path_2[100];
    sprintf(save_path_1, "%s/%s", save_1, "events-0-63.bin");
    sprintf(save_path_2, "%s/%s", save_2, "events-256-319.bin");
    FILE* fp_save_1 = fopen(save_path_1, "wb");
    FILE* fp_save_2 = fopen(save_path_2, "wb");

    int cnt_1 = 0;
    int cnt_2 = 0;
    int cnt = 0;
    int line_size = 18, file_num = get_file_size(path)/line_size;
    int heatmap[MAP_SIZE][MAP_SIZE];
    memset(heatmap, 0, sizeof(heatmap));
    FILE* fp_data = fopen(path, "rb");
    if (fp_data == NULL) {
        printf("Failed to open data file\n");
        return -1;
    }
    while (true) {
        // Initialize the event array
        float event[EVENT_SIZE], energies[ENERGY_SIZE];
        memset(event, 0, sizeof(event));
        float t = 0., e = 0.;

        char buffer[18];
        int num_read = fread(buffer, line_size, 1, fp_data);
        if (num_read != 1) {
            break;
        }
        uint8_t num = buffer[0];
        uint8_t idx = buffer[1];
        long long timestamp = *(int64_t*)(buffer + 2);
        float energy = *(float*)(buffer + 10);
        int sipm = *(int32_t*)(buffer + 14);
        int arr = (int)lut[sipm % 64];
        energies[arr] = energy;
        event[arr+2] = energy;
        t += timestamp;
        e += energy;
        cnt++;

        // Read additional line tuples if there are more than one
        while (idx < num-1) {
            num_read = fread(buffer, line_size, 1, fp_data);
            if (num_read != 1) {
                break;
            }
            num = buffer[0];
            idx = buffer[1];
            timestamp = *(int64_t*)(buffer + 2);
            energy = *(float*)(buffer + 10);
            sipm = *(int32_t*)(buffer + 14);
            arr = (int)lut[sipm % 64];
            energies[arr] = energy;
            event[arr+2] = energy;
            t += timestamp;
            e += energy;
            cnt++;
        }
        event[0] = t / num; // avg time window
        event[1] = e; // total energy

        // Determine which save directory to use based on the first element of the event array
        int save_dir = (sipm >= 256.0) ? 1 : 0;

        if (save_dir == 0) {
            fwrite(event, sizeof(float), EVENT_SIZE, fp_save_1);
            cnt_1++;
        } else {
            fwrite(event, sizeof(float), EVENT_SIZE, fp_save_2);
            cnt_2++;
        }

        printf("\rProcessed %d/%d lines of data", cnt, file_num);
    }
    fclose(fp_save_1);
    fclose(fp_save_2);
    fclose(fp_data);
    printf("\n%d events total\n", cnt_1+cnt_2);

    return 0;
}