/*
 * Copyright (c) 2017 Massachusetts Institute of Technology (MIT)
 * All rights reserved.
 *
 * Distributed under the terms of the BSD 3-clause license.
 *
 * The full license is in the LICENSE file, distributed with this software.
 */
/*
 * Benchmark write speed. digital rf 2.0
 */
#include "digital_rf.h"
#include <stdio.h>
#include <time.h>

void digital_rf_randomize_int16(int16_t *data, int len) {
    int i;
    for (i = 0; i < len; i++) {
        data[i] = (i % 32768) * (i + 8192) * (i % 13);
    }
}

void digital_rf_randomize_float32(float *data, int len) {
    int i;
    for (i = 0; i < len; i++) {
        data[i] = ((float)((int16_t)(i % 32768) * (i + 8192) * (i % 13))) / 32767.0;
    }
}

// length of random number buffer
#define NUM_SUBCHANNELS         4
#define RANDOM_BLOCK_SIZE       4194304 * NUM_SUBCHANNELS
#define N_SAMPLES               1048576
#define WRITE_BLOCK_SIZE        1000000
#define START_TIMESTAMP         1394368230   // March 9, 2014
#define SAMPLE_RATE_NUMERATOR   1000000
#define SAMPLE_RATE_DENOMINATOR 1
#define SUBDIR_CADENCE          10
#define MILLISECS_PER_FILE      1000
#define JUNK_FOLDER             "./junk0"
#define CHECKSUM                0
#define COMPRESSION_LEVEL       0
#define IS_COMPLEX              1
#define IS_CONTINUOUS           1
#define PRINT_DOTS              0

int main(int argc, char *argv[]) {
    // 16-bit integer data
    int16_t *data_int16;
    uint64_t i, status;
    uint64_t vector_length;
    int      n_writes;
    clock_t  begin, end;
    double   time_spent;

    data_int16    = (int16_t *)malloc(RANDOM_BLOCK_SIZE * sizeof(int16_t));
    vector_length = WRITE_BLOCK_SIZE;
    n_writes      = (int)1e8 / WRITE_BLOCK_SIZE;

    digital_rf_randomize_int16(data_int16, RANDOM_BLOCK_SIZE);

    // Local variables
    Digital_rf_write_object *data_object               = NULL;
    uint64_t                 vector_leading_edge_index = 0;
    uint64_t                 global_start_sample =
        (uint64_t)(START_TIMESTAMP * ((long double)SAMPLE_RATE_NUMERATOR) / SAMPLE_RATE_DENOMINATOR);

    status = system("rm -rf " JUNK_FOLDER " ; mkdir " JUNK_FOLDER);
    if (status != 0) {
        perror("system()");
        return -1;
    }

    vector_leading_edge_index = 0;
    data_object = digital_rf_create_write_hdf5(JUNK_FOLDER, H5T_NATIVE_SHORT, SUBDIR_CADENCE, MILLISECS_PER_FILE,
                                               global_start_sample, SAMPLE_RATE_NUMERATOR, SAMPLE_RATE_DENOMINATOR,
                                               "FAKE_UUID_0", CHECKSUM, COMPRESSION_LEVEL, IS_COMPLEX, NUM_SUBCHANNELS,
                                               IS_CONTINUOUS, PRINT_DOTS);
    if (!data_object) {
        exit(-1);
    }

    begin = clock();
    for (i = 0; i < n_writes; i++) {
        status = digital_rf_write_hdf5(data_object, vector_leading_edge_index, data_int16, vector_length);
        vector_leading_edge_index += vector_length;

        if (status != 0) {
            break;
        }
    }
    end = clock();

    if (status == 0) {
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("Done - %1.2f MB/s\n", ((double)n_writes * 4 * NUM_SUBCHANNELS * vector_length) / time_spent / 1e6);
    }
    else {
        perror("digital_rf_write_hdf5()");
    }

    // Close file.
    digital_rf_close_write_hdf5(data_object);

    status = system("rm -rf " JUNK_FOLDER);
    free(data_int16);

    return status;
}
