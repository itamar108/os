#include "osm.h"
#include <sys/time.h>
#include <iostream>


//#ifndef _OSM_H
//#define _OSM_H
#define UNROLLING_FACTOR 10


#define FAILURE -1

unsigned int roundUp(unsigned int iterations);

/* Time measurement function for a simple arithmetic operation.
   returns time in nano-seconds upon success,
   and -1 upon failure.
   */
double osm_operation_time(unsigned int iterations) {
    unsigned int roundedIterations = roundUp(iterations);
    struct timeval start{}, end{};
    int k = 0, k1 = 0, k2 = 0, k3 = 0, k4 = 0, k5 = 0, k6 = 0, k7 =0, k8 = 0, k9 = 0;
    if (gettimeofday(&start, nullptr) == FAILURE || iterations == 0) {
        return FAILURE;
    }
    for (unsigned int i = 0; i < roundedIterations / UNROLLING_FACTOR; i++) {
        k += 4;
        k1 += 4;
        k2 += 4;
        k3 += 4;
        k4 += 4;
        k5 += 4;
        k6 += 4;
        k7 += 4;
        k8 += 4;
        k9 += 4;
    }
    if (gettimeofday(&end, nullptr) == FAILURE) {
        return FAILURE;
    }
    return (double) ((1e9 * (double) (end.tv_sec - start.tv_sec) + 1000*(double)(end.tv_usec - start.tv_usec))) / roundedIterations;

}

unsigned int roundUp(unsigned int iterations) { return ((iterations + UNROLLING_FACTOR - 1) / UNROLLING_FACTOR) * UNROLLING_FACTOR; }

void dummy_function(){}

/* Time measurement function for an empty function call.
   returns time in nano-seconds upon success,
   and -1 upon failure.
   */
double osm_function_time(unsigned int iterations){
    unsigned int roundedIterations = roundUp(iterations);
    struct timeval start{}, end{};
    if (gettimeofday(&start, nullptr) == FAILURE || iterations == 0) {
        return FAILURE;
    }
    for (unsigned int i = 0; i < roundedIterations / UNROLLING_FACTOR; i++) {
        dummy_function();
        dummy_function();
        dummy_function();
        dummy_function();
        dummy_function();
        dummy_function();
        dummy_function();
        dummy_function();
        dummy_function();
        dummy_function();
    }
    if (gettimeofday(&end, nullptr) == FAILURE) {
        return FAILURE;
    }
    return (double) ((1e9 * (double) (end.tv_sec - start.tv_sec) + 1000*(double)(end.tv_usec - start.tv_usec))) / roundedIterations;

}


/* Time measurement function for an empty trap into the operating system.
   returns time in nano-seconds upon success,
   and -1 upon failure.
   */
double osm_syscall_time(unsigned int iterations) {
    unsigned int roundedIterations = roundUp(iterations);
    struct timeval start{}, end{};
    if (gettimeofday(&start, nullptr) == FAILURE) {
        return FAILURE;
    }
    for (unsigned int i = 0; i < roundedIterations / UNROLLING_FACTOR; i++) {
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
    }
    if (gettimeofday(&end, nullptr) == FAILURE) {
        return FAILURE;
    }
    return (double) ((1e9 * (double) (end.tv_sec - start.tv_sec) + 1000*(double)(end.tv_usec - start.tv_usec))) / roundedIterations;

}



