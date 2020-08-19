#include "VirtualMemory.cpp"
#include <cstdio>
#include <array>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <random>

#include <gtest/gtest.h>
#include <unordered_map>

using page_t = std::vector<word_t>;
extern std::vector<page_t> RAM;
extern std::unordered_map<uint64_t, page_t> swapFile;

void printMemory(int start, int end) {
//    for (word_t frameIndex = 0; frameIndex < ; frameIndex++) {
    word_t word;
    for (uint64_t i = start*PAGE_SIZE; i < end*PAGE_SIZE; ++i) {
        if (!(i % PAGE_SIZE))
        {
            std::cout<<std::endl;
            std::cout<<"frame "<<i / PAGE_SIZE << ": ";
        }
        PMread(i, &word);
        std::cout<<word<<" ";
    }
    std::cout<<std::endl;
//    }
}



//TEST(internalTests, getFrame)
//{
//    EXPECT_EQ(getFrameFromAddress(10), 0);
//    EXPECT_EQ(getFrameFromAddress(16), 1);
//    EXPECT_EQ(getFrameFromAddress(35), 2);
//    EXPECT_EQ(getFrameFromAddress(167), 10);
//}

//TEST(internalTests, getLine)
//{
//    EXPECT_EQ(getLinePhysicalAddress(15, 10), 15*PAGE_SIZE+ 10);
//}

//TEST(internalTests, getFreeFrame)
//{
//    int i = 0;
////    VMinitialize();
////    PMwrite(0,1);
////    EXPECT_EQ(getFreeFrame(i), 1);
////
////    PMwrite(0,1);
////    PMwrite(1, 2);
////    EXPECT_EQ(getFreeFrame(i), 1);
////
////    PMwrite(0,1);
////    PMwrite(PAGE_SIZE + 3, 3);
////    printMemory(0,2);
////    EXPECT_EQ(getFreeFrame(i), 3);
//
//    VMinitialize();
//
//    PMwrite(5,1);
//    PMwrite(PAGE_SIZE + 7, 2);
//    PMwrite(PAGE_SIZE*2 + 3, 5);
//    PMwrite(PAGE_SIZE*2 + 4, 4);
//    PMwrite(PAGE_SIZE*2 + 5, 3);
//    printMemory(0,3);
//    int val = getFreeFrame(i);
//    EXPECT_EQ(val, 5);
//
//}

//TEST(internalTests, flowFrameTest1) {
//    VMinitialize();
//    PMwrite(0,1);
//    PMwrite(1, 0);
//    PMwrite(2, 5);
//    PMwrite(3, 2);
//    PMwrite(4,0);
//    PMwrite(5,3);
//    PMwrite(6,4);
//    PMwrite(7,0);
//    PMwrite(8,9);
//    PMwrite(9,3);
////    PMwrite(10,0);
////    PMwrite(11,0);
////    PMwrite(12,0);
////    PMwrite(13,0);
////    PMwrite(14,0);
////    PMwrite(15,0);
//    int i = 0;
//    printMemory(0,8);
//    int val = getFreeFrame(i, nullptr, nullptr, nullptr, nullptr);
//    printMemory(0,8);
//    EXPECT_EQ(val, 5);
//}
//
//TEST(internalTests, flowFrameTest2) {
//    VMinitialize();
//    PMwrite(0,1);
//    PMwrite(1, 4);
//    PMwrite(2, 5);
//    PMwrite(3, 2);
//    PMwrite(4,0);
//    PMwrite(5,3);
//    PMwrite(6,0);
//    PMwrite(7,0);
//    PMwrite(8,0);
//    PMwrite(9,0);
//    PMwrite(10,0);
//    PMwrite(11,6);
//    PMwrite(12,0);
//    PMwrite(13,7);
//    PMwrite(14,4);
//    PMwrite(15,2);
//    int i = 0;
//    printMemory(0,8);
//    int val = getFreeFrame(i, nullptr, nullptr, nullptr, nullptr);
//    printMemory(0,8);
//    EXPECT_EQ(val, 3);
//}
//
//TEST(internalTests, counterTest) {
//    PMwrite(0,1);
//    PMwrite(1, 0);
//    PMwrite(2, 5);
//    PMwrite(3, 2);
//    PMwrite(4,0);
//    PMwrite(5,3);
//    PMwrite(6,4);
//    PMwrite(7,0);
//    PMwrite(8,45);
//    PMwrite(9,3);
//    PMwrite(10,0);
//    PMwrite(11,6);
//    PMwrite(12,0);
//    PMwrite(13,0);
//    PMwrite(14,45);
//    PMwrite(15,211);
//    int i = 0;
//    word_t leafArr[MAX_LEAVES] = {0}; //todo check if root is 0
//    uint64_t parentArr[MAX_LEAVES] = {0};
//    uint64_t virtArr[MAX_LEAVES] = {0};
//    printMemory(0,8);
//    word_t safe[TABLES_DEPTH] = {0};
//    safe[0] = 6;
//    int val = getFreeFrame(i, leafArr, parentArr, virtArr, safe);
//    printMemory(0,8);
//    EXPECT_EQ(val, -1);
//    EXPECT_EQ(i, 6);
//
////    EXPECT_EQ(*leafArr, 7);
////    EXPECT_EQ(*parentArr, 6);
////    EXPECT_EQ(*virtArr, 3);
////
////    EXPECT_EQ(*(leafArr + 1), 4);
////    EXPECT_EQ(*(parentArr + 1), 3);
////    EXPECT_EQ(*(virtArr + 1), 6);
//    std::cout<<" ";
//}

TEST(internalTests, writeTest) {
    PMwrite(0,1);
    PMwrite(1, 0);
    PMwrite(2, 5);
    PMwrite(3, 2);
    PMwrite(4,0);
    PMwrite(5,3);
    PMwrite(6,4);
    PMwrite(7,0);
    PMwrite(8,45);
    PMwrite(9,3);
    PMwrite(10,0);
    PMwrite(11,6);
    PMwrite(12,0);
    PMwrite(13,7);
    PMwrite(14,88);
    PMwrite(15,3);
    word_t val;
    printMemory(0,8);
    VMwrite(18,50);
    printMemory(0,8);
    PMread(14, &val);
//    EXPECT_EQ(val, 50);



}

TEST(flowTest, bikosTest)
{
    VMinitialize();
    VMwrite(13, 3);
    EXPECT_EQ(RAM, (std::vector<page_t>{{1, 0}, {0, 2}, {0, 3}, {4, 0}, {0, 3}, {0, 0}, {0, 0}, {0, 0}}));
    EXPECT_TRUE(swapFile.empty());
    word_t value = 0;

    VMread(13, &value);
    EXPECT_EQ(3, value);
    EXPECT_EQ(RAM,(std::vector<page_t> {{1, 0}, {0, 2}, {0, 3}, {4, 0}, {0, 3}, {0, 0}, {0, 0}, {0, 0}}));
    EXPECT_TRUE(swapFile.empty());

    VMread(6, &value);  // Reading garbage just to fill the paging structures
    EXPECT_EQ(value, 0);  // But we know the garbage is actually 0
    EXPECT_EQ(RAM,(std::vector<page_t> {{1, 0}, {5, 2}, {0, 3}, {4, 0}, {0, 3}, {0, 6}, {0, 7}, {0, 0}}));
    EXPECT_TRUE(swapFile.empty());


    VMread(31, &value);  // Let's read some more garbage
    EXPECT_EQ(value, 0);
    EXPECT_EQ(RAM,(std::vector<page_t> {{1, 4}, {5, 0}, {0, 7}, {0, 2}, {0, 3}, {0, 6}, {0, 0}, {0, 0}}));
    EXPECT_EQ(swapFile, (std::unordered_map<uint64_t, page_t> {{3, {0, 0}}, {6, {0, 3}}}));

    std::puts("Example flow passed");
}

//TEST(flowTest, shai) {
//    VMinitialize();
//    for (uint64_t i = 1; i < (VIRTUAL_MEMORY_SIZE); i++) {
//        printf("writing to %llu\n", (long long int) i);
//        VMwrite(i, i);
//        i = i * 10;
//    }
//
//    for (uint64_t i = 1; i < (VIRTUAL_MEMORY_SIZE); i++) {
//        word_t value;
//        VMread(i, &value);
//        printf("reading from %llu %d\n", (long long int) i, value);
//        EXPECT_EQ(value, i);
//        i = i * 10;
//    }
//}

TEST(simpletest, simple)
{
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
}

//TEST(internalTests, flowFrameTest3) {
//    VMinitialize();
//    PMwrite(0,1);
//    PMwrite(1, 4);
//    PMwrite(2, 0);
//    PMwrite(3, 2);
//    PMwrite(4,0);
//    PMwrite(5,3);
//    PMwrite(6,0);
//    PMwrite(7,5);
//    PMwrite(8,0);
//    PMwrite(9,0);
//    PMwrite(10,8);
//    PMwrite(11,0);
//    PMwrite(12,0);
//    PMwrite(13,0);
//    PMwrite(14,0);
//    PMwrite(15,0);
//    int i = 0;
//    printMemory(0,8);
//    int val = getFreeFrame(i);
//    printMemory(0,8);
//    EXPECT_EQ(val, 4);
//}


//TEST(MattanTests, waitAndCloseTest) {
//
//    CounterClient client;
//    auto s1 = new VString("This string is full of characters");
//    auto s2 = new VString("Multithreading is awesome");
//    auto s3 = new VString("conditions are race bad");
//    client.inputVec.push_back({nullptr, s1});
//    client.inputVec.push_back({nullptr, s2});
//    client.inputVec.push_back({nullptr, s3});
//    JobState state;
//    JobState last_state={UNDEFINED_STAGE,0};
//    JobHandle job = startMapReduceJob(client, client.inputVec, client.outputVec, 3);
//    getJobState(job, &state);
//    waitForJob(job);
//
//    // Should work without system error, since we are supposed to check if join has already been called.
//    closeJobHandle(job);
//
//}
//
//TEST(MattanTests, errorMessageTest) {
//	CounterClient client;
//	auto s1 = new VString("This string is full of characters");
//	auto s2 = new VString("Multithreading is awesome");
//	auto s3 = new VString("conditions are race bad");
//	client.inputVec.push_back({nullptr, s1});
//	client.inputVec.push_back({nullptr, s2});
//	client.inputVec.push_back({nullptr, s3});
//	ASSERT_EXIT(startMapReduceJob(client, client.inputVec, client.outputVec, 20000000),
//	            ::testing::ExitedWithCode(1),
//	            ::testing::MatchesRegex("system error: .*\n")
//	) << "When starting too many threads, thread creation should fail, causing program to exit with code 1 and print an error";
//}
//
//TEST(MattanTests, outputTest) {
//	CounterClient client;
//	std::vector<std::string> a;
//	std::string line;
//	std::ifstream f(RANDOM_STRINGS_PATH);
//	if (f.is_open()) {
//		while (getline(f, line)) {
//			a.push_back(line);
//		}
//		for (std::string &str : a) {
//			auto v = new VString(str);
//			client.inputVec.push_back({nullptr, v});
//		}
//	} else {
//		FAIL() << "(Technical error) Coludn't find strings file at " << RANDOM_STRINGS_PATH << " - maybe you deleted it by mistake?";
//	}
//
//	std::cout << "Starting job" << std::endl;
//
//	JobState state;
//	JobState last_state = {UNDEFINED_STAGE, 0};
//	JobHandle job = startMapReduceJob(client, client.inputVec, client.outputVec, 200);
//	getJobState(job, &state);
//	last_state = state;
//	while (!(state.stage == REDUCE_STAGE && state.percentage == 100.0)) {
//
//		if (last_state.stage != state.stage || last_state.percentage != state.percentage) {
//			printf("stage %d, %f%% \n", state.stage, state.percentage);
//		}
//		last_state = state;
//		getJobState(job, &state);
//	}
//
//	closeJobHandle(job);
//	std::map<char, int> expectedOutput = {{'0',  13258},
//	                                      {'1',  13014},
//	                                      {'2',  13241},
//	                                      {'3',  13246},
//	                                      {'4',  13042},
//	                                      {'5',  13479},
//	                                      {'6',  13085},
//	                                      {'7',  12949},
//	                                      {'8',  13168},
//	                                      {'9',  13033},
//	                                      {'a',  13119},
//	                                      {'b',  13078},
//	                                      {'c',  13360},
//	                                      {'d',  13126},
//	                                      {'e',  13055},
//	                                      {'f',  13460},
//	                                      {'g',  12940},
//	                                      {'h',  13231},
//	                                      {'i',  13112},
//	                                      {'j',  13063},
//	                                      {'k',  13232},
//	                                      {'l',  13016},
//	                                      {'m',  13172},
//	                                      {'n',  13161},
//	                                      {'o',  12989},
//	                                      {'p',  13148},
//	                                      {'q',  13148},
//	                                      {'r',  13200},
//	                                      {'s',  13238},
//	                                      {'t',  12964},
//	                                      {'u',  13106},
//	                                      {'v',  13334},
//	                                      {'w',  13039},
//	                                      {'x',  13202},
//	                                      {'y',  13199},
//	                                      {'z',  13010},
//	                                      {'A',  13149},
//	                                      {'B',  13103},
//	                                      {'C',  13305},
//	                                      {'D',  13132},
//	                                      {'E',  13009},
//	                                      {'F',  13217},
//	                                      {'G',  13107},
//	                                      {'H',  13050},
//	                                      {'I',  13394},
//	                                      {'J',  13248},
//	                                      {'K',  13283},
//	                                      {'L',  13129},
//	                                      {'M',  13095},
//	                                      {'N',  13190},
//	                                      {'O',  13426},
//	                                      {'P',  13187},
//	                                      {'Q',  13223},
//	                                      {'R',  13360},
//	                                      {'S',  13056},
//	                                      {'T',  13249},
//	                                      {'U',  13175},
//	                                      {'V',  13117},
//	                                      {'W',  13244},
//	                                      {'X',  13228},
//	                                      {'Y',  13023},
//	                                      {'Z',  13032},
//	                                      {'!',  13352},
//	                                      {'"',  13271},
//	                                      {'#',  13242},
//	                                      {'$',  13176},
//	                                      {'%',  13172},
//	                                      {'&',  13121},
//	                                      {'\'', 13274},
//	                                      {'(',  13250},
//	                                      {')',  13107},
//	                                      {'*',  13157},
//	                                      {'+',  13199},
//	                                      {',',  13143},
//	                                      {'-',  13206},
//	                                      {'.',  13191},
//	                                      {'/',  13266},
//	                                      {':',  13308},
//	                                      {';',  13309},
//	                                      {'<',  13158},
//	                                      {'=',  13345},
//	                                      {'>',  13061},
//	                                      {'?',  13238},
//	                                      {'@',  13130},
//	                                      {'[',  13050},
//	                                      {'\\', 13152},
//	                                      {']',  13123},
//	                                      {'^',  13143},
//	                                      {'_',  13010},
//	                                      {'`',  13226},
//	                                      {'{',  13054},
//	                                      {'|',  13043},
//	                                      {'}',  13164},
//	                                      {'~',  13234},
//	                                      {' ',  13096}};
//	for (OutputPair &pair: client.outputVec) {
//		char c = ((const KChar *) pair.first)->c;
//		int count = ((const VCount *) pair.second)->count;
//		printf("The character %c appeared %u time%s\n",
//		       c, count, count > 1 ? "s" : "");
//		auto iter = expectedOutput.find(c);
//		if(iter != expectedOutput.end())
//		{
//			//element found;
//			if (count!=iter->second) {
//				FAIL() << "Your program reported the value "<< count << " for the key " << c << std::endl
//				       << "the actual value is "<< iter->second << std::endl;
//			} else {
//				expectedOutput.erase(iter);
//			}
//		} else {
//			FAIL() << "The key "<<c<<" with value "<<count<<"Does not exist!" << std::endl;
//		}
//	}
//	if (expectedOutput.size() > 0) {
//		auto iter = expectedOutput.begin();
//		FAIL() << "Your program has missed " << expectedOutput.size() << " letters, the first letter you missed is: " << iter->first << " whose count should be " << iter->second;
//	}
//}
//
//
//
//TEST(MattanTests, progressTest) {
//    for (int i = 0; i < REPEATS; ++i)
//    {
//        std::cout<<"repetition #"<<i<<std::endl;
//        CounterClient client;
//        auto s1 = new VString("This string is full of characters");
//        auto s2 = new VString("Multithreading is awesome");
//        auto s3 = new VString("conditions are race bad");
//        client.inputVec.push_back({nullptr, s1});
//        client.inputVec.push_back({nullptr, s2});
//        client.inputVec.push_back({nullptr, s3});
//        JobState state;
//        JobState last_state={UNDEFINED_STAGE,0};
//        JobHandle job = startMapReduceJob(client, client.inputVec, client.outputVec, 6);
//        getJobState(job, &state);
//
//        while (state.stage != REDUCE_STAGE || state.percentage != 100.0)
//        {
//            if (last_state.stage != state.stage || last_state.percentage != state.percentage) {
//                printf("stage %d, %f%% \n", state.stage, state.percentage);
//                if (state.percentage > 100 || state.percentage < 0) {
//                    FAIL() << "Invalid percentage(not in 0-100): " << state.percentage << ", encountered during stage " << state.stage << ")";
//                }
//                if (last_state.stage == state.stage && state.percentage < last_state.percentage) {
//                    FAIL() << "Bad percentage(smaller than previous percentage at same stage): " << state.percentage << ", encountered during stage " << state.stage << ")";
//                }
//                if (last_state.stage > state.stage) {
//                    FAIL() << "Bad stage " << state.stage << " - smaller than previous stage, encountered with percentage " << state.percentage;
//                }
//            }
//            last_state = state;
//            getJobState(job, &state);
//        }
//        printf("Done!\n");
//
//        closeJobHandle(job);
//
//    }
//}
//
//TEST(MattanTests, deadlockTest) {
//    for (int i = 0; i < DEADLOCK_REPEATS; ++i)
//    {
//        std::cout<<"repetition #"<<i<<std::endl;
//        CounterClient client;
//        auto s1 = new VString("This string is full of characters");
//        auto s2 = new VString("Multithreading is awesome");
//        auto s3 = new VString("conditions are race bad");
//        client.inputVec.push_back({nullptr, s1});
//        client.inputVec.push_back({nullptr, s2});
//        client.inputVec.push_back({nullptr, s3});
//        JobState state;
//        JobState last_state={UNDEFINED_STAGE,0};
//        JobHandle job = startMapReduceJob(client, client.inputVec, client.outputVec, 3);
//        getJobState(job, &state);
//        while (state.stage != REDUCE_STAGE || state.percentage != 100.0)
//        {
//            last_state = state;
//            getJobState(job, &state);
//        }
//        closeJobHandle(job);
//    }
//}
//
//void randbody(int iterations) {
//
//
//    std::default_random_engine generator(time(nullptr));
//    std::uniform_int_distribution<int> bernouli(0, 1);
//    std::uniform_int_distribution<int> trinary(0, 2);
//    std::uniform_int_distribution<int> dist(1, 1000);
//    std::uniform_int_distribution<int> concurrentJobAmount(1, 20);
//    std::vector<std::uniform_int_distribution<int>> dists;
//
//    std::uniform_int_distribution<int> large_amount(100, 500);
//    std::uniform_int_distribution<int> small_amount(6, 100);
//    std::uniform_int_distribution<int> tiny_amount(2, 5);
//    dists.push_back(large_amount);
//    dists.push_back(small_amount);
//    dists.push_back(tiny_amount);
//
//    for (int i = 0; i < iterations; ++i) {
//        int activeJobs = concurrentJobAmount(generator);
////        std::cout << "Job amount: " << activeJobs << std::endl;
//
//        std::vector<CounterClient> clients(activeJobs);
//        std::vector<JobHandle> jobs(activeJobs, nullptr);
//        std::vector<std::pair<JobState, JobState>> jobStates(activeJobs); // [0]=prevstate [1]=curstate
//        std::vector<int> levels;
//        std::cout << "repetition #" << i << std::endl;
//        int lineAmount = dist(generator);
////        std::cout<<"line amount: "<<lineAmount<<std::endl;
//
//        for (int j = 0; j < activeJobs; ++j) {
////            std::cout<<"Job number: "<<j<<std::endl;
//            auto &client = clients.at(j);
//            std::vector<std::string> a;
//            std::string line;
//            std::ifstream f(RANDOM_STRINGS_PATH);
//            if (f.is_open()) {
//                int k = 0;
//                while (getline(f, line) && k < lineAmount) {
//                    if (bernouli(generator)) {
//                        a.push_back(line);
//                        k++;
//                    }
//                }
//                for (std::string &str : a) {
//                    auto v = new VString(str);
//                    client.inputVec.push_back({nullptr, v});
//                }
//            } else {
//                FAIL() << "(Technical error) Coludn't find strings file at " << RANDOM_STRINGS_PATH
//                       << " - maybe you deleted it by mistake?";
//            }
//            int level = dists[trinary(generator)](generator);
//            levels.push_back(level);
////            std::cout<<"Thread Amount: "<<level<<std::endl;
//        }
//
//        jobs.reserve(activeJobs);
//        for (int j = 0; j < activeJobs; ++j) {
////            std::cout<<"job "<<j<<std::endl;
//            auto &client = clients[j];
//            JobHandle handle = startMapReduceJob(client, client.inputVec, client.outputVec, levels[j]);
//            jobs[j] = handle;
//        }
//        int totalJobs = activeJobs;
//        for (int j = 0; activeJobs > 0; ++j) {
//            if (jobs[j] != nullptr) {
//                JobHandle job = jobs[j];
//                JobState &state = jobStates[j].second;
//                getJobState(job, &state);
//                JobState &last_state = jobStates[j].first;
//                if (!(state.stage == REDUCE_STAGE && last_state.percentage == 100.0)) {
//                    if (last_state.stage != state.stage || last_state.percentage != state.percentage) {
//                        if (state.percentage > 100 || state.percentage < 0) {
//                            FAIL() << "Invalid percentage(not in 0-100): " <<
//                                   "Current stage:" << state.stage << " " << state.percentage << "%" << std::endl <<
//                                   "Previous stage: " << last_state.stage << " " << last_state.percentage << "%"
//                                   << std::endl;
//                        }
//                        if (last_state.stage == state.stage && state.percentage < last_state.percentage) {
//                            FAIL() << "Bad percentage(smaller than previous percentage at same stage): " <<
//                                   "Current stage:" << state.stage << " " << state.percentage << "%" << std::endl <<
//                                   "Previous stage:" << last_state.stage << " " << last_state.percentage << "%"
//                                   << std::endl;
//                        }
//                        if (last_state.stage > state.stage) {
//                            FAIL() << "Bad stage: " <<
//                                   "Current stage:" << state.stage << " " << state.percentage << "%" << std::endl <<
//                                   "Previous stage:" << last_state.stage << " " << last_state.percentage << "%"
//                                   << std::endl;
//                        }
//                    }
//                    last_state = state;
//                    getJobState(job, &state);
//                } else {
//                    closeJobHandle(job);
//                    jobs[j] = nullptr;
//                    activeJobs--;
//                }
//            }
//            if (j == totalJobs - 1) {
//                j = -1;
//            }
//        }
//
//    }
//}
//
//TEST(MattanTests, randomTest) {
//	EXPECT_EXIT(randbody(RANDOM_REPEATS), ::testing::KilledBySignal(24), ::testing::MatchesRegex(""));
//    //TODO  If you fail this test, comment the line above and uncomment the line below to see what exit code you failed with more easily.
//    // The task should be killed by signal 24 (SIGXCPU) which means the cpu time limit was exceeded. That should be the only reason that
//    // the task dies. This should happen only after more than 100 iterations. Running sanitizer will slow the program down
//    // a lot and could cause a false positive.
//    // Meant to be run on aquarium computers since on private computers you probably won't ever get the kill signal.
////    randbody(RANDOM_REPEATS);
//
//}
//
//
//

//TEST(internalTests, flowFrameTest3) {
//    VMinitialize();
//    PMwrite(0*PAGE_SIZE + 4,1);
//    PMwrite(0*PAGE_SIZE + 3,1);
//    PMwrite(1*PAGE_SIZE + 5, 2);
////    PMwrite(2, 5);
////    PMwrite(3, 2);
////    PMwrite(4,0);
////    PMwrite(5,3);
////    PMwrite(6,0);
////    PMwrite(7,0);
////    PMwrite(8,0);
////    PMwrite(9,0);
////    PMwrite(10,0);
////    PMwrite(11,6);
////    PMwrite(12,0);
////    PMwrite(13,7);
////    PMwrite(14,4);
////    PMwrite(15,2);
//    int i = 0;
//    printMemory(0,2);
//    int val = getFreeFrame(i);
//    printMemory(0,2);
//    EXPECT_EQ(val, 2);
//    EXPECT_EQ(i, 2);
//}



