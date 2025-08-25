#include <stdio.h>
#include "lora_frame_sync.h"

int main(void)
{
    // Test 1: clean preamble at start
    uint32_t symbols1[] = {0,0,0,0,0,0,0,0, 3,4,5};
    size_t idx1 = lora_frame_sync_find_preamble(symbols1, 11, 8);
    if (idx1 != 8) {
        printf("Test 1 failed: idx %zu\n", idx1);
        return 1;
    }
    uint32_t aligned1[8];
    size_t len1 = lora_frame_sync_align(symbols1, 11, 8, aligned1);
    if (len1 != 3 || aligned1[0] != 3 || aligned1[1] != 4 || aligned1[2] != 5) {
        printf("Test 1 align failed\n");
        return 1;
    }

    // Test 2: preamble after some symbols
    uint32_t symbols2[] = {5,1,0,0,0,0,0,0,0,0,2,3};
    size_t idx2 = lora_frame_sync_find_preamble(symbols2, 12, 8);
    if (idx2 != 10) {
        printf("Test 2 failed: idx %zu\n", idx2);
        return 1;
    }
    uint32_t aligned2[8];
    size_t len2 = lora_frame_sync_align(symbols2, 12, 8, aligned2);
    if (len2 != 2 || aligned2[0] != 2 || aligned2[1] != 3) {
        printf("Test 2 align failed\n");
        return 1;
    }

    // Test 3: no preamble present
    uint32_t symbols3[] = {1,2,3,4,5};
    size_t idx3 = lora_frame_sync_find_preamble(symbols3, 5, 8);
    if (idx3 != 5) {
        printf("Test 3 failed: idx %zu\n", idx3);
        return 1;
    }

    printf("Frame sync test passed\n");
    return 0;
}
