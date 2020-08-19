//
// Created by mattan on 21/04/2020.
//

#include "uthreads.h"
//#include "Thread.h"
#include <cstring>
#include <signal.h>
#include <setjmp.h>
#include <deque>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
#include <algorithm>

#define MAX_THREADS_EXCEEDED -1
#define MAIN_THREAD 0
#define READY  0
#define RUNNING 1
#define BLOCKED 2
#define FAILURE -1
#define SEC 1000000

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr) {
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#endif



struct Thread {
    int _priority;
    int _id;
    int _quantumLength;
    int _cur_state;
    int _qunatums = 0;
    char _stack[STACK_SIZE];
    sigjmp_buf _env;

    Thread(int priority, int id, int quantumLength, int curState)
            : _priority(priority), _id(id), _quantumLength(quantumLength), _cur_state(curState) {
    }

};


static int total_quantums = 0;
static int *quantum_lengths = nullptr;
static int priority_amount;
static int current_running_tid;
static std::deque<int> ready_queue;
static Thread *thread_array[MAX_THREAD_NUM] = {nullptr};
struct itimerval timer;
struct sigaction saction{};  //TODO make global (?)
void erase_thread_from_queue(int tid);

void jump_to_next_thread();

sigset_t sigalrm;


int get_quantum_length(int priority) {
    return quantum_lengths[priority];
}

int getNewThreadId() {
    for (int i = 0; i < MAX_THREAD_NUM; ++i) {
        if (thread_array[i] == NULL) {
            return i;
        }
    }
    return MAX_THREADS_EXCEEDED;
}
void destructor() {
    sigprocmask(SIG_BLOCK, &sigalrm, NULL);
    for (int i = 0; i < MAX_THREAD_NUM; i++) {
        if (thread_array[i] != nullptr) {
            delete thread_array[i];
            thread_array[i] = nullptr;
        }
    }
    delete[] quantum_lengths;
    quantum_lengths = nullptr;
    sigprocmask(SIG_UNBLOCK, &sigalrm, NULL);
}




int system_error(const char *errorMessage) {
    std::cerr << "system error: " << errorMessage << std::endl;
    destructor();
    exit(EXIT_FAILURE);
}

void sig_block() {
    if (sigprocmask(SIG_BLOCK, &sigalrm, NULL) != 0) {
        system_error("Failed signal block");
    }
}

void sig_unblock() {
    if (sigprocmask(SIG_UNBLOCK, &sigalrm, NULL) != 0) {
        system_error("Failed signal unblock");
    }
}

int library_error(const char *error_message) {
    std::cerr << "thread library error: " << error_message << std::endl;
    sig_unblock();
    return FAILURE;
}

int set_timer(int tv_usec) {

    timer.it_value.tv_sec = tv_usec/SEC;
    timer.it_value.tv_usec = tv_usec%SEC;        // first time interval, microseconds part
    // Start a virtual timer. It counts down whenever this process is executing.
    timer.it_interval.tv_sec = tv_usec/SEC;    // following time intervals, seconds part
    timer.it_interval.tv_usec = tv_usec%SEC;    // following time intervals, microseconds part
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL)) {
        system_error("timer initialization failure");
    }
    return 0;
}


void scheduler(int sig) {
    assert(sig == SIGVTALRM);
    thread_array[current_running_tid]->_cur_state = READY;
    ready_queue.push_back(current_running_tid);
    int return_value = sigsetjmp(thread_array[current_running_tid]->_env, 1);
    if (return_value == 5) {
        return;
    }
    jump_to_next_thread();
}

/***
 * change the thread to the next one in the queue
 */
void jump_to_next_thread() {
    int tid = current_running_tid;
//    assert(!ready_queue.empty());
    if (!ready_queue.empty()) {
        tid = ready_queue.front();   //TODO
    }
//    } else {  //TODO delete
//        ready_queue.push_front(0);
//    }
    thread_array[tid]->_cur_state = RUNNING;
    current_running_tid = tid;
    set_timer(thread_array[current_running_tid]->_quantumLength);
    ready_queue.pop_front();
    total_quantums++;
    thread_array[current_running_tid]->_qunatums++;
    sig_unblock();
    siglongjmp(thread_array[current_running_tid]->_env, 5);
}

void jump_from_blocked() {
    sig_block();
    int return_value = sigsetjmp(thread_array[current_running_tid]->_env, 1);
    if (return_value == 5) {
        return;
    }
//    int tid = current_running_tid;
    if (!ready_queue.empty()) {
        current_running_tid = ready_queue.front();   //TODO
        ready_queue.pop_front();
//        current_running_tid = tid;
    } else {
        assert(current_running_tid == 0);
    }
    thread_array[current_running_tid]->_cur_state = RUNNING;
    set_timer(thread_array[current_running_tid]->_quantumLength);
    total_quantums++;
    thread_array[current_running_tid]->_qunatums++;
    sig_unblock();
    siglongjmp(thread_array[current_running_tid]->_env, 5);
}

void jump_from_terminate() {
    sig_block();

    int tid = current_running_tid;
    if (!ready_queue.empty()) {
        tid = ready_queue.front();   //TODO
        ready_queue.pop_front();
        current_running_tid = tid;
    } else {
        assert(current_running_tid == 0);
    }
    thread_array[tid]->_cur_state = RUNNING;
    set_timer(thread_array[current_running_tid]->_quantumLength);
    total_quantums++;
    thread_array[current_running_tid]->_qunatums++;
    sig_unblock();
    siglongjmp(thread_array[current_running_tid]->_env, 5);
}

void jump_from_alarm(int sig) {
    assert(sig == SIGVTALRM);
    thread_array[current_running_tid]->_cur_state = READY;
    ready_queue.push_back(current_running_tid);
    int return_value = sigsetjmp(thread_array[current_running_tid]->_env, 1);
    if (return_value == 5) {
        return;
    }
    int tid = current_running_tid;
    if (!ready_queue.empty()) {
        tid = ready_queue.front();   //TODO
        ready_queue.pop_front();
        current_running_tid = tid;
    } else {
        assert(current_running_tid == 0);
    }
    thread_array[tid]->_cur_state = RUNNING;
    set_timer(thread_array[current_running_tid]->_quantumLength);
    total_quantums++;
    thread_array[current_running_tid]->_qunatums++;
    siglongjmp(thread_array[current_running_tid]->_env, 5);
}

int uthread_init(int *quantum_usecs, int size) {
    destructor();
    if (size <= 0) {
        return library_error("illegal priority array size");
    }
    quantum_lengths = new int[size]();
    priority_amount = size;
    for (int i = 0; i < size; i++) {
        if (quantum_usecs[i] <= 0) {
            const char *errorMessage = "invalid quantum length";
            return library_error(errorMessage);
        }
        quantum_lengths[i] = quantum_usecs[i];
    }
    thread_array[0] = new Thread(0, 0, quantum_lengths[0], RUNNING);
    thread_array[0]->_qunatums = 1;
    current_running_tid = 0;
    if (set_timer(thread_array[0]->_quantumLength) < 0) {
        system_error("couldn't set timer");
    }
    sigsetjmp(thread_array[0]->_env, 1);


    sigaddset(&saction.sa_mask, SIGVTALRM);
    saction.sa_handler = &jump_from_alarm;

    // setting the sig action to be our one
    if (sigaction(SIGVTALRM, &saction, NULL) < 0) {
        system_error("Could not modify signal saction");
    }
    // add alarm to set
    sigaddset(&sigalrm, SIGVTALRM);
    total_quantums = 1;
    return 0;
}


int uthread_spawn(void (*f)(void), int priority) {
    sig_block();
    int newId = getNewThreadId();
    if (newId == MAX_THREADS_EXCEEDED) {
        return library_error("Maximum thread count exceeded");
    }
    if (priority > priority_amount - 1 || priority < 0) {
        return library_error("Invalid Priority");
    }
    int new_quantums = get_quantum_length(priority);
    int newState = READY;
    thread_array[newId] = new Thread(priority, newId, new_quantums, newState);
    //TODO modify destructor
    ready_queue.push_back(newId);
    address_t sp, pc;
    sp = (address_t) thread_array[newId]->_stack + STACK_SIZE - sizeof(address_t);
    pc = (address_t) f;
    sigsetjmp(thread_array[newId]->_env, 1);
    (thread_array[newId]->_env->__jmpbuf)[JB_SP] = translate_address(sp);
    (thread_array[newId]->_env->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&thread_array[newId]->_env->__saved_mask);
    sig_unblock();
    return newId;
}

int uthread_change_priority(int tid, int priority) {
    sig_block();
    if (tid >= 100 ||
        tid < 0 ||
        priority >= priority_amount ||
        priority < 0 ||
        thread_array[tid] == nullptr) {
        return library_error("invalid priority or ID"); //TODO check if critical code
    }
    thread_array[tid]->_priority = priority;
    thread_array[tid]->_quantumLength = quantum_lengths[priority];
    sig_unblock();
    return 0;
}

int uthread_terminate(int tid) {

    sig_block();
    if (tid >= 100 ||
        tid < 0 ||
        thread_array[tid] == nullptr) {
        return library_error("invalid ID");
    }

    //main thread
    if (tid == MAIN_THREAD) {
        destructor();
        exit(EXIT_SUCCESS);
    }
    //erase from ready queue
    int current_state = thread_array[tid]->_cur_state;
    if (current_state == READY) {
        erase_thread_from_queue(tid);
    }
    delete thread_array[tid];
    thread_array[tid] = nullptr;
    if (current_state == RUNNING) {
        jump_from_terminate();
    }
    sig_unblock();
    return 0;
}

/***
 * erase the thread with given tid from the ready queue
 * @param tid -  id of thread to delete
 */
void erase_thread_from_queue(int tid)
{
    auto threadAtindex = std::find(ready_queue.begin(), ready_queue.end(), tid);
    assert(threadAtindex != ready_queue.end());
    ready_queue.erase(threadAtindex);
}

int uthread_block(int tid) {

    sig_block();
    if (tid >= 100 ||
        tid < 0 ||
        thread_array[tid] == nullptr) {
        return library_error("invalid ID");
    }

    if (tid==MAIN_THREAD)
    {
        return library_error("couldn't block main thread");
    }

    if (thread_array[tid]->_cur_state == READY)
    {
        erase_thread_from_queue(tid);
    }
    thread_array[tid]->_cur_state = BLOCKED;

    if (tid==current_running_tid)
    {
        jump_from_blocked();
    }
    sig_unblock();
    return 0;
}

int uthread_resume(int tid) {
    sig_block();

    if (tid >= 100 ||
        tid < 0 ||
        thread_array[tid] == nullptr) {
        return library_error("invalid ID");
    }

    if (thread_array[tid]->_cur_state==BLOCKED)
    {
        ready_queue.push_back(tid);
        thread_array[tid]->_cur_state = READY;

    }
    sig_unblock();

    return 0;
}


int uthread_get_tid() {
    return current_running_tid;
}

int uthread_get_total_quantums() {
    return total_quantums;
}

int uthread_get_quantums(int tid) {
    sig_block();

    if (tid >= 100 ||
        tid < 0 ||
        thread_array[tid] == nullptr) {
        return library_error("invalid ID");
    }
    sig_unblock();

    return thread_array[current_running_tid]->_qunatums;
}
