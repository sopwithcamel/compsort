#include <iostream>

#define __STDC_LIMIT_MACROS

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;
using namespace google::protobuf::io;

bool varint_compress(uint32_t* data, uint32_t len)
{
    string str;
    StringOutputStream* raw_out = new StringOutputStream(&str);
    CodedOutputStream* coded_out = new CodedOutputStream(raw_out);
    for (int i=0; i<len; i++)
        coded_out->WriteVarint32(data[i]);
    printf("Size of data: %lu\n", sizeof(uint32_t) * len);
    printf("Size of compressed: %lu\n", str.size());    
}

inline uint32_t get_least_sig_k_bitmask(uint32_t k)
{
    return ((1<<k) - 1);
}

/* Each 32 bit unsigned integer is stored as follows:
 * + the k LSB are stored first
 * + if the number doesn't fit in k bits, the k+1 th bit is 1, else it is 0
 * + the next k LSB are stored next and so on
 */
bool my_compress(uint32_t* data, uint32_t len, uint32_t*& out, uint32_t& out_len)
{
    // k is the number of bits allocated for each value
    uint32_t* last_word = out;
    uint32_t a = 32; // denotes the number of bits remaining in last word
    uint32_t k = floor(log2((data[len-1] - data[0]) / len)) + 1;
    printf("K: %u\n", k);
    *last_word = k; last_word++; // store k as first element since we need it later
    last_word++; // leave another space for storing offset in the final word
    *last_word = 0;

    for (int i=0; i<len; i++) {
        uint32_t n = data[i];
        int32_t m = floor(log2(n)) + 1; // number of bits reqd.
        while (m > 0) {
            uint32_t bm = get_least_sig_k_bitmask(k);
            uint32_t x = n & bm;
            n >>= k;
            m -= k;

            // set overflow bit
            x <<= 1;
            if (m > 0) x |= 1;

            // check if x fits in last_word
            if (k+1 <= a) {
                x <<= a - (k+1);
                *last_word |= x;
                a -= (k+1);
            } else { // the entire fragment doesn't fit in last word
                // extract a MSB from x
                uint32_t y = x >> (k+1-a);
                *last_word |= y;

                // increment last_word
                last_word++;
                *last_word = 0;
                                
                // get remaining bits
                y = x & get_least_sig_k_bitmask(k+1-a);
                y <<= (32 - (k+1-a));
                *last_word |= y;
                a = 32 - (k+1-a);
            }
        }
    }
    out_len = (last_word - out) + (a == 32? 0 : 1);
    out[1] = 32-a; // store final offset in second word
    printf("Size of data: %lu\n", sizeof(uint32_t) * len);
    printf("Size of compressed: %lu\n", out_len * sizeof(uint32_t));    
}


bool my_decompress(uint32_t* data, uint32_t len, uint32_t*& out, uint32_t& out_len)
{
    out_len = 0;
    uint32_t k = data[0];
    uint32_t finoff = data[1];
    uint32_t cur = 0;
    uint32_t b = 0; // number of bits in cur so far
    uint32_t rem = 0; // number of bits reqd from next word to make up k+1
    uint32_t x; // stores fragments
    for (uint32_t i=2; i<len; i++) {
        uint32_t n = data[i];
        uint32_t a = 32; // number of bits remaining in current word
        bool of;
        while (a >= k+1) {
            uint32_t num_bits_read;
            if (i == len-1 && a == 32-finoff)
                break;
            if (rem > 0) { // we have a partial fragment from before
                // read rem bits
                num_bits_read = rem;
                uint32_t rembm = get_least_sig_k_bitmask(rem);
                uint32_t y = (n >> (a-rem)) & rembm;
                x |= y;
                rem = 0;
            } else {
                // extract next k+1 bits from n
                num_bits_read = k+1;
                uint32_t bm = get_least_sig_k_bitmask(k+1);
                x = (n >> a-(k+1)) & bm;
            }
            of = ((x & 1) == 1);
            x >>= 1; // get rid of overflow bit
            x <<= b;
            cur |= x;
            b += k;
            a -= num_bits_read;
            if (!of) {
                out[out_len++] = cur;
                cur = 0;
                b = 0;
            }
        }
        // get LS a bits
        uint32_t bm = get_least_sig_k_bitmask(a);
        x = n & bm;
        rem = (k+1-a);
        x <<= rem; // make space for remaining bits
        assert(rem > 0);
    }
    printf("Size of data: %lu\n", sizeof(uint32_t) * len);
    printf("Size of decompressed: %lu\n", out_len*sizeof(uint32_t));    
}

bool compress(uint32_t* data, uint32_t len, uint32_t*& out, uint32_t& out_len)
{
    for (int i=len-1; i>=1; i--)
        data[i] -= data[i-1];
    for (int i=0; i<len; i++)
        printf("send: %u\n", data[i]);
    my_compress(data, len, out, out_len);
    for (int i=1; i<len; i++)
        data[i] += data[i-1];
} 

bool decompress(uint32_t* data, uint32_t len, uint32_t*& out, uint32_t& out_len)
{
    my_decompress(data, len, out, out_len);
    for (int i=0; i<out_len; i++)
        printf("recd: %u\n", out[i]);
    for (int i=1; i<out_len; i++)
        out[i] += out[i-1];
} 


void quicksort(uint32_t* arr, uint32_t uleft, uint32_t uright)
{
    int32_t i, j, stack_pointer = -1;
    int32_t left = uleft;
    int32_t right = uright;
    int32_t* rstack = new int32_t[128];
    uint32_t swap, temp;
    while (true) {
        if (right - left <= 7) {
            for (j = left + 1; j <= right; j++) {
                swap = arr[j];
                i = j - 1;
                if (i < 0) {
                    fprintf(stderr, "Noo");
                    assert(false);
                }
                while (i >= left && (arr[i] > swap)) {
                    arr[i + 1] = arr[i];
                    i--;
                }
                arr[i + 1] = swap;
            }
            if (stack_pointer == -1) {
                break;
            }
            right = rstack[stack_pointer--];
            left = rstack[stack_pointer--];
        } else {
            int median = (left + right) >> 1;
            i = left + 1;
            j = right;
            swap = arr[median]; arr[median] = arr[i]; arr[i] = swap;
            if (arr[left] > arr[right]) {
                swap = arr[left]; arr[left] = arr[right]; arr[right] = swap;
            }
            if (arr[i] > arr[right]) {
                swap = arr[i]; arr[i] = arr[right]; arr[right] = swap;
            }
            if (arr[left] > arr[i]) {
                swap = arr[left]; arr[left] = arr[i]; arr[i] = swap;
            }
            temp = arr[i];
            while (true) {
                while (arr[++i] < temp);
                while (arr[--j] > temp);
                if (j < i) {
                    break;
                }
                swap = arr[i]; arr[i] = arr[j]; arr[j] = swap;
            }
            arr[left + 1] = arr[j];
            arr[j] = temp;
            if (right - i + 1 >= j - left) {
                rstack[++stack_pointer] = i;
                rstack[++stack_pointer] = right;
                right = j - 1;
            } else {
                rstack[++stack_pointer] = left;
                rstack[++stack_pointer] = j - 1;
                left = i;
            }
        }
    } 
    delete[] rstack;
}

void check_match(uint32_t* a, uint32_t alen, uint32_t* b, uint32_t blen)
{
    assert(alen == blen);
    for (uint32_t i=0; i<alen; i++) {
        if (a[i] != b[i]) {
            printf("%d, %u, %u\n", i, a[i], b[i]);
            assert(false);
        }
    }
}

int main()
{
    uint32_t n = 10;//240;
    uint32_t* data = (uint32_t*)calloc(n, sizeof(uint32_t));
    uint32_t* comp = (uint32_t*)calloc(n+4, sizeof(uint32_t));
    uint32_t* decomp = (uint32_t*)calloc(n+4, sizeof(uint32_t));
    uint32_t comp_len;
    uint32_t decomp_len;
    uint32_t min = (UINT32_MAX / 64) * 63;
    printf("Min %u\n", min);
    for (uint32_t i=0; i<n; i++) {
        data[i] = min + (rand() % (UINT32_MAX - min - 1));
    }
    quicksort(data, 0, n-1); 
    printf("First: %u, Last: %u\n", data[0], data[n-1]);
    compress(data, n, comp, comp_len);
    decompress(comp, comp_len, decomp, decomp_len);
    check_match(data, n, decomp, decomp_len);
    
    free(data);
    free(comp);
    free(decomp);
}
