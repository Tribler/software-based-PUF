//
// Created by Ade Setyawan on 10/25/17.
//

#ifndef PUF_TOOLS_H
#define PUF_TOOLS_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "string.h"

using namespace std;

class Tools {
public:
    Tools();

    int8_t xor_int(int8_t a,int8_t b);

    void convert_uint8_to_bin_array(uint8_t input, int8_t *result);

    void convert_bin_array_to_uint8_array(int8_t *binary, uint8_t *result, int length);

    void convert_uint8_array_to_bin_array(uint8_t *input, int8_t *result, int length);

    void combine_two_bin_array_to_uint8_array(int8_t *bin1, int length_bin1, int8_t *bin2, int length_bin2, uint8_t *result);

    uint8_t ceil(uint8_t i, uint8_t div);

    void xor_binary_array(uint8_t *a, uint8_t *b, uint8_t *result, int size);

    void gen_key_per_row(uint8_t *keys, uint8_t *result);

    void convert_key_back(uint8_t *key_37, uint8_t *key_32);
};

#endif //PUF_TOOLS_H
