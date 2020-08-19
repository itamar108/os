//
// Created by Itamar Hadad on 17/05/2020.
//

#include "MapReduceFramework.h"
//#include <pthread.h>
#include <iostream>
#include <atomic>
#include <algorithm>
#include "Barrier.h"

#define TO_PERCENTAGE 100
#define EXTRACT_STAGE 0b1100000000000000000000000000000000000000000000000000000000000000
#define STAGE 0b0100000000000000000000000000000000000000000000000000000000000000
#define INITIALIZE_COUNTER_MAP_STATE 0b0100000000000000000000000000000000000000000000000000000000000000
#define EXTRACT_TOTAL 0b0011111111111111111111111111111110000000000000000000000000000000
#define EXTRACT_PROCCESSED 0b0000000000000000000000000000000001111111111111111111111111111111

static const char *const mutexLockError = "system error: pthread_mutex_lock";
static const char *const mutexUnlockError = "system error: pthread_mutex_unlock";
static const char *const pthreadJoinErrorMessage = "system error: pthread_join";
static const char *const mutexInitError = "system error: pthread_mutex_init";
static const char *const mutexDestroyError = "system error: pthread_mutex_destroy";
static const char *const pthreadCreateError = "system error: pthread_create";


void error(const std::string &msg) {
    std::cerr << msg << std::endl;
    exit(1);
}

/*all the things that threads should share between each other*/
struct MapReduceContext {
    std::atomic<uint64_t> *_atomicCounter;
    const InputVec *_inputVec;
    OutputVec *_outputVec;
    const MapReduceClient *_client;
    pthread_mutex_t *_reduceLock;
    Barrier *_barrier;
    IntermediateMap *_intermediateMap;
    std::atomic<uint64_t> *_totalEmittedK2;
    std::vector<K2 *> *_reduceMapKeyVector;
    pthread_mutex_t *_mapLock;
    std::vector<IntermediatePair> _threadsMapOutvec;

    MapReduceContext() {

    }

    MapReduceContext(std::atomic<uint64_t> *_atomicCounter, const InputVec *_inputVec, OutputVec *_outputVec,
                     const MapReduceClient *_client, pthread_mutex_t *reduceVectorLock, Barrier *barrier,
                     IntermediateMap *intermediateMap, std::atomic<uint64_t> *totalEmittedK2,
                     std::vector<K2 *> *reduceMapKeyVector, pthread_mutex_t *lock)
            : _atomicCounter(_atomicCounter), _inputVec(_inputVec), _outputVec(_outputVec),
              _client(_client), _reduceLock(reduceVectorLock), _barrier(barrier),
              _intermediateMap(intermediateMap), _totalEmittedK2(totalEmittedK2),
              _reduceMapKeyVector(reduceMapKeyVector) {
        if (pthread_mutex_init(lock, nullptr)) {
            error(mutexInitError);
        }
        _mapLock = lock;
    }

};

/*all the things that threads should share between each other*/
struct ShuffleContext {
    std::atomic<uint64_t> *_atomicCounter;
    OutputVec *_outputVec;
    const MapReduceClient *_client;
    pthread_mutex_t *_reduceLock;
    Barrier *_barrier;
    IntermediateMap *_intermediateMap;
    int _multiThreadLevel;
    MapReduceContext *_contexts;
    std::atomic<uint64_t> *_totalEmittedK2;
    std::vector<K2 *> *_reduceMapKeyVector;

    ShuffleContext() {

    }

    ShuffleContext(std::atomic<uint64_t> *_atomicCounter, OutputVec *_outputVec, const MapReduceClient *_client,
                   pthread_mutex_t *reduceLock, Barrier *barrier, IntermediateMap *intermediateMap,
                   int multiThreadLevel, MapReduceContext *contexts, std::atomic<uint64_t> *totalEmittedK2,
                   std::vector<K2 *> *reduceMapKeyVector)
            : _atomicCounter(_atomicCounter), _outputVec(_outputVec),
              _client(_client), _reduceLock(reduceLock), _barrier(barrier),
              _intermediateMap(intermediateMap), _multiThreadLevel(multiThreadLevel), _contexts(contexts),
              _totalEmittedK2(totalEmittedK2), _reduceMapKeyVector(reduceMapKeyVector) {}

};


void *mapReduceFunc(void *context) {
    auto contextPtr = static_cast<MapReduceContext *> (context);
    for (unsigned long oldValue = (*(contextPtr->_atomicCounter))++;
         (oldValue & EXTRACT_PROCCESSED) < contextPtr->_inputVec->size();) {
        oldValue &= EXTRACT_PROCCESSED;

        contextPtr->_client->map((*(contextPtr->_inputVec))[oldValue].first,
                                 (*(contextPtr->_inputVec))[oldValue].second, context);
        oldValue = (*(contextPtr->_atomicCounter))++;
    }
    contextPtr->_barrier->barrier();
    IntermediateMap &map = *(contextPtr->_intermediateMap);
    for (unsigned long oldValue = (*(contextPtr->_atomicCounter))++;
         (oldValue & EXTRACT_PROCCESSED) < contextPtr->_reduceMapKeyVector->size();) {
        oldValue &= EXTRACT_PROCCESSED;
        K2 *key = (*(contextPtr->_reduceMapKeyVector))[oldValue];

        contextPtr->_client->reduce(key, map[key], context);
        oldValue = (*(contextPtr->_atomicCounter))++;
    }
    return 0;
}


void *shuffle(void *context) {
    auto contextPtr = static_cast<ShuffleContext *> (context);
    IntermediateMap &map = *(contextPtr->_intermediateMap);
    uint64_t processedKeys = 0;
    while (contextPtr->_barrier->count < contextPtr->_multiThreadLevel - 1) {

        for (int i = 0; i < contextPtr->_multiThreadLevel - 1; ++i) {
            MapReduceContext *currentContext = contextPtr->_contexts + i;
            if (pthread_mutex_lock(currentContext->_mapLock)) {
                error(mutexLockError);
            }
            for (auto pair : currentContext->_threadsMapOutvec) {
                ++processedKeys;
                map[pair.first].push_back(pair.second);
            }
            currentContext->_threadsMapOutvec.resize(0);
            if (pthread_mutex_unlock(currentContext->_mapLock)) {
                error(mutexUnlockError);
            }
        }
    }
    auto initializeShuffleStage =
            (SHUFFLE_STAGE * (unsigned long) STAGE) + ((*(contextPtr->_totalEmittedK2)) << 31u) + processedKeys;
    *(contextPtr->_atomicCounter) =
            initializeShuffleStage;
    for (int i = 0; i < contextPtr->_multiThreadLevel - 1; ++i) {
        MapReduceContext *currentContext = contextPtr->_contexts + i;
        for (auto pair : currentContext->_threadsMapOutvec) {
            map[pair.first].push_back(pair.second);
            (*(contextPtr->_atomicCounter))++;

        }
    }

    auto initializeReduceStage = REDUCE_STAGE * (unsigned long) STAGE + (map.size() << 31u);
    *(contextPtr->_atomicCounter) = initializeReduceStage;

    /*creating a vector of keys*/
    for (auto it = map.begin(); it != map.end(); ++it) {
        contextPtr->_reduceMapKeyVector->push_back(it->first);
    }


    /*  unlocking the barrier   */
    contextPtr->_barrier->barrier();


    for (unsigned long oldValue = (*(contextPtr->_atomicCounter))++;
         (oldValue & EXTRACT_PROCCESSED) < contextPtr->_reduceMapKeyVector->size();) {
        oldValue &= EXTRACT_PROCCESSED;
        K2 *key = (*(contextPtr->_reduceMapKeyVector))[oldValue];
        contextPtr->_client->reduce(key, map[key], contextPtr->_contexts);
        oldValue = (*(contextPtr->_atomicCounter))++;
    }
    return 0;
}


struct JobContext {
    const InputVec *_inputVec;
    OutputVec *_outputVec;
    int _multiThreadLevel;
    const MapReduceClient *_client;
    std::atomic<uint64_t> _totalEmittedK2{0};
    Barrier *_barrier;
    IntermediateMap _intermediateMap;
    std::vector<K2 *> _reduceMapKeyVector;
    pthread_t *_threads;
    ShuffleContext *_shuffleContext;
    MapReduceContext *_contexts;
    std::atomic<uint64_t> _atomicCounter{0};
    pthread_mutex_t *_locks;
    pthread_mutex_t _reduceLock;
    bool _waiting = false;
    JobContext(const InputVec *inputVec, OutputVec *outputvec, int multiThreadLevel, const MapReduceClient *client) :
            _inputVec(inputVec), _outputVec(outputvec), _multiThreadLevel(multiThreadLevel), _client(client) {
        _atomicCounter = INITIALIZE_COUNTER_MAP_STATE + (inputVec->size() << 31);
        // initializes state and total k1 keys to process
        _locks = new pthread_mutex_t[_multiThreadLevel - 1];
        _threads = new pthread_t[_multiThreadLevel];
        _contexts = new MapReduceContext[_multiThreadLevel - 1];
        _barrier = new Barrier(_multiThreadLevel);
        if (pthread_mutex_init(&_reduceLock, nullptr)) {
            error(mutexInitError);
        }
        for (int i = 0; i < _multiThreadLevel - 1; ++i) {
            _contexts[i] = MapReduceContext(&_atomicCounter, inputVec, outputvec, client,
                                            &_reduceLock, _barrier, &_intermediateMap,
                                            &_totalEmittedK2, &_reduceMapKeyVector, _locks + i);
            if (pthread_create(&_threads[i], nullptr, mapReduceFunc, &_contexts[i])) {
                error(pthreadCreateError);
            }
        }
        // producing the shuffle thread
        _shuffleContext = new ShuffleContext(&_atomicCounter, outputvec, client, &_reduceLock, _barrier,
                                             &_intermediateMap,
                                             _multiThreadLevel,
                                             _contexts, &_totalEmittedK2, &_reduceMapKeyVector);
        if (pthread_create(&_threads[multiThreadLevel - 1], nullptr, shuffle, _shuffleContext)) {
            error(pthreadCreateError);
        }
    }

    ~JobContext() {
        for (int i = 0; i < _multiThreadLevel - 1; i++) {
            if (pthread_mutex_destroy(_locks + i)) {
                error(mutexDestroyError);
            }
        }
        if (pthread_mutex_destroy(&_reduceLock)) {
            error(mutexDestroyError);
        }
        delete[] _threads;
        delete[] _contexts;
        delete[] _locks;
        delete _barrier;
        delete _shuffleContext;

    }


};


void emit2(K2 *key, V2 *value, void *context) {

    auto jobContext = static_cast<MapReduceContext *> (context);
    if (pthread_mutex_lock(jobContext->_mapLock)) {
        error(mutexLockError);
    }

    jobContext->_threadsMapOutvec.push_back(IntermediatePair(key, value));
    (*(jobContext->_totalEmittedK2))++;
    if (pthread_mutex_unlock(jobContext->_mapLock)) {
        error(mutexUnlockError);
    }
}

void emit3(K3 *key, V3 *value, void *context) {
    auto jobContext = static_cast<MapReduceContext *> (context);
    if (pthread_mutex_lock(jobContext->_reduceLock)) {
        error(mutexLockError);
    }
    jobContext->_outputVec->push_back(OutputPair(key, value));
    if (pthread_mutex_unlock(jobContext->_reduceLock)) {
        error(mutexUnlockError);
    }


}

JobHandle
startMapReduceJob(const MapReduceClient &client, const InputVec &inputVec, OutputVec &outputVec, int multiThreadLevel) {
    return static_cast<JobHandle>(new JobContext(&inputVec, &outputVec, multiThreadLevel, &client));
}

void waitForJob(JobHandle job) {
    auto manager = static_cast<JobContext *>(job);
    if (!(manager->_waiting)) {
        for (int i = 0; i < manager->_multiThreadLevel; ++i) {
            if (pthread_join(manager->_threads[i], nullptr)) {
                error(pthreadJoinErrorMessage);
            }
        }
        manager->_waiting = true;
    }
}

void getJobState(JobHandle job, JobState *state) {

    auto manager = static_cast<JobContext *>(job);
    unsigned long tempAtomic = (manager->_atomicCounter);
    state->stage = static_cast<stage_t>((tempAtomic & EXTRACT_STAGE) >> 62);
    state->percentage = std::min((TO_PERCENTAGE * (float) (tempAtomic & EXTRACT_PROCCESSED) /
                                  ((tempAtomic & EXTRACT_TOTAL) >> 31)), (float) (100));
}

void closeJobHandle(JobHandle job) {
    waitForJob(job);
    auto manager = static_cast<JobContext *>(job);
    delete manager;
}

