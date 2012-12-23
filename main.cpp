#include <assert.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <algorithm>

#include "compsort.h"
#include "ittnotify.h"

using namespace std;

int main() {
    uint32_t num = 100000000;
#ifdef VTUNE
    __itt_pause();
#endif

    // allocate arrays
    uint32_t* data = new uint32_t[num];
    uint32_t* data_copy = new uint32_t[num];

    // random elements for input
    for (uint32_t i = 0; i < num; ++i)
        data[i] = rand();

    // sort data
    sort(data, data + num);
    for (uint32_t i = 0; i < num; ++i)
        data_copy[i] = data[i];

    // allocate space for compressed and decompressed elements
    uint32_t* comp_data = new uint32_t[num];
    uint32_t comp_len;

    uint32_t* decomp_data = new uint32_t[num];
    uint32_t decomp_len;


#ifdef VTUNE
    __itt_resume();
#endif

    assert(compsort::compress(data, num, comp_data, comp_len));
    assert(compsort::decompress(comp_data, comp_len, decomp_data, decomp_len));

#ifdef VTUNE
    __itt_pause();
#endif

    assert(decomp_len == num);
    for (uint32_t i = 0; i < num; ++i) {
        if (decomp_data[i] != data_copy[i]) {
            fprintf(stderr, "%d\n", i);
            assert(false);
        }
    }
    fprintf(stderr, "Original length: %u\n", num);
    fprintf(stderr, "Compressed length: %u\n", comp_len);

    // Clean up
    delete[] decomp_data;
    delete[] comp_data;
    delete[] data;
    delete[] data_copy;
}
