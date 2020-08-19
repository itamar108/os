#include "VirtualMemory.h"

#include <cstdio>
#include <cassert>
#include <vector>
#include <unordered_map>

using page_t = std::vector<word_t>;
extern std::vector<page_t> RAM;
extern std::unordered_map<uint64_t, page_t> swapFile;


int main(int argc, char **argv) {
    VMinitialize();
    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        printf("writing to %llu\n", (long long int) i);
        VMwrite(5 * i * PAGE_SIZE, i);
    }

    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        word_t value;
        VMread(5 * i * PAGE_SIZE, &value);
        printf("reading from %llu %d\n", (long long int) i, value);
        assert(uint64_t(value) == i);
    }
    printf("success\n");

    return 0;
}

//int main(int argc, char **argv) {
//    VMinitialize();
//    int RANGE = VIRTUAL_MEMORY_SIZE;
//    for (uint64_t i = 0; i < RANGE; ++i) {
//        uint64_t address = VIRTUAL_MEMORY_SIZE - 1 -i;
//        printf("writing to %llu\n", (long long int) address);
//        assert(VMwrite(address, i));
//    }
//
//    for (uint64_t i = 0; i < RANGE; ++i) {
//        word_t value = 0;
//        uint64_t address = VIRTUAL_MEMORY_SIZE - 1 -i;
//        VMread(address, &value);
//        printf("reading from %llu %d\n", (long long int) address, value);
//        assert(uint64_t(value) == i);
//    }
//    VMwrite(1, 1);
//    word_t value;
//    VMread(1, &value);
////    VMprint();
//    assert(value == 1);
//    printf("success\n");
//
//    return 0;
//}

