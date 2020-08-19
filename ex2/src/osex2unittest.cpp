#include <iostream>
//#include "Thread.h"
#include "uthreads.h"
#include "uthreads.cpp"
#include "./lib/googletest-master/googletest/include/gtest/gtest.h"
#include <string>
#include <sstream>

int main(int argc , char *argv[])
{
    testing::InitGoogleTest(&argc , argv);
    return RUN_ALL_TESTS();
}



/**
 * HashMap related tests.
 */
TEST(uthreadsTest , uthread_init)
{
//    Thread thread = Thread(0, 0, 5, -2);
    int arr[1000] = {0};
    for (int i = 0; i< 1000; ++i)
    {
        arr[i] = i+100;
    }
    uthread_init(arr, 1000);
    for (int i = 0; i < 100; ++i) {
        arr[i] = -1;
    }
    for (int i = 0; i < 1000; ++i){
        EXPECT_EQ(quantum_lengths[i], i + 100);
    }
    EXPECT_EQ(thread_array[0]->_quantumLength, 100);
    EXPECT_EQ(thread_array[0]->_priority, 0);
    EXPECT_EQ(thread_array[0]->_id, 0);
    EXPECT_EQ(thread_array[0]->_cur_state, 1);
    EXPECT_EQ(priority_amount, 1000);
}

TEST(uthreads_test, timer_test) {
    int arr[1]= {999999};
    uthread_init(arr,1);
    while (true) {

    }
}

void junk(){
//    while (printf("in func\n") > 0);
//    pause();
    for (;;) {
        printf("in junk\n");
        fflush(stdout);
        usleep(900000);
    }
}

void func(){
    while (true) {
        printf("in funk\n");
        fflush(stdout);
        usleep(900000);
    }
}

TEST(uthreads_test, scheduler) {
    int arr[6] = {250000, 500000, 750000, 250000, 999999, 999999};
    uthread_init(arr, 6);
    EXPECT_EQ(uthread_spawn(func, 0), 1);
    EXPECT_EQ(uthread_spawn(junk, 4), 2);
    while(true) {

    }
}



TEST(uthreadsTest, uthread_spawn)
{
    int arr[6] = {999999,999999,999999,999999,999999,999999};
    uthread_init(arr, 6);
    EXPECT_EQ(uthread_spawn(func, 5), 1);
//    EXPECT_EXIT(uthread_spawn(func, 7),);
    EXPECT_EQ(thread_array[1]->_quantumLength, 999999);
    EXPECT_EQ(thread_array[1]->_cur_state, READY);
    EXPECT_EQ(thread_array[1]->_priority, 5);
//    EXPECT_EQ(thread_array[1]->_env[0].__jmpbuf[JB_SP], );
//    EXPECT_EQ(thread_array[1]->_env[0].__jmpbuf[JB_PC], );

}
