#include <iostream>
#include "compsort.h"

bool compress(uint32_t* data, uint32_t len, uint32_t*& out, uint32_t& out_len)
{
    for (int i=len-1; i>=1; i--)
        data[i] -= data[i-1];
    //    for (int i=0; i<len; i++)
    //        printf("send: %u\n", data[i]);
    CompSort::compress(data, len, out, out_len);
    for (int i=1; i<len; i++)
        data[i] += data[i-1];
} 

bool decompress(uint32_t* data, uint32_t len, uint32_t*& out, uint32_t& out_len)
{
    CompSort::decompress(data, len, out, out_len);
    //    for (int i=0; i<out_len; i++)
    //        printf("recd: %u\n", out[i]);
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
    printf("Match successful\n");
}

int main()
{
    uint32_t n = 1000000;
    uint32_t* data = (uint32_t*)calloc(n, sizeof(uint32_t));
    uint32_t* comp = (uint32_t*)calloc(n+4, sizeof(uint32_t));
    uint32_t* decomp = (uint32_t*)calloc(n+4, sizeof(uint32_t));
    uint32_t comp_len;
    uint32_t decomp_len;
    uint32_t min = (UINT32_MAX / 64) * 63;
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
