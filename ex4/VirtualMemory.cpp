#include <cstdint>
#include <cassert>
#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include <math.h>
#include <algorithm>

#define P1_WIDTH ((VIRTUAL_ADDRESS_WIDTH % OFFSET_WIDTH) ? (VIRTUAL_ADDRESS_WIDTH % OFFSET_WIDTH) : OFFSET_WIDTH)
#define P1_SIZE  (1<<P1_WIDTH)

#define MAX_LEAVES NUM_FRAMES

void clearTable(uint64_t frameIndex) {
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}

void VMaccess(uint64_t virtualAddress, uint64_t &remainingAddress, word_t &physicalWordAddress);


void reinitialize(word_t *physicalLeafArr, uint64_t *parentArr, uint64_t *virtualLeafArr, int &maxFrameIndex,
                  int &leafAmount) {
    maxFrameIndex = 0;
    leafAmount = 0;
    for (int j = 0; j < MAX_LEAVES; ++j) {
        physicalLeafArr[0] = 0;
        virtualLeafArr[0] = 0;
        parentArr[0] = 0;
    }

}

bool notLocked(word_t frame, word_t *locked) {
    for (int i = 0; i < TABLES_DEPTH; ++i) {
        if (locked[i] == frame) {
            return false;
        }
    }
    return true;
}

int lock(word_t frame, word_t *locked) {
    if (frame == -1) {
        return 0;
    }
    for (int i = 0; i < TABLES_DEPTH; ++i) {
        if (!locked[i]) {
            locked[i] = frame;
            return 0;
        }
    }
    return 1;
}

void VMinitialize() {
    clearTable(0);
}


int getFarthestFrameIndex(uint64_t pageSwappedIn, const uint64_t *virtualArr, int &leafAmount) {
    int retval = -1;
    uint64_t maxDistance = 0;
    for (int i = 0; i < leafAmount; ++i) {
        long realDistance = std::abs(static_cast<long>(pageSwappedIn - virtualArr[i]));
        uint64_t curDistance = std::min(static_cast<long>(NUM_PAGES - realDistance), realDistance);
        if (curDistance >= maxDistance) {
            retval = i;
            maxDistance = curDistance;
        }
    }
    return retval;
}


void disconnect(uint64_t address, uint64_t page, uint64_t frame) {
    PMwrite(address, 0);
    PMevict(frame, page);
}

uint64_t getPartial(int depth, int index) {
    return pow(PAGE_SIZE, TABLES_DEPTH - depth) * index;
}

uint64_t getLinePhysicalAddress(int frame, int offset) {
    assert(offset >= 0 && offset < PAGE_SIZE);
    return (frame * PAGE_SIZE) + offset;
}

word_t getFrameFromAddress(uint64_t address) {
    return address / PAGE_SIZE;
}

bool isLegalAddress(uint64_t virtualAddress) {
    return virtualAddress >= 0 && virtualAddress < VIRTUAL_MEMORY_SIZE;

}

void updateLeaves(word_t **leafArr, uint64_t **parentArr, uint64_t **leafVirtualAddrArr, int &leafAmount,
                  const uint64_t *addrArr, const int depth, const uint64_t virtualAddress, const word_t curFrame) {
    **leafArr = curFrame;
    **parentArr = addrArr[depth - 1];
    **leafVirtualAddrArr = virtualAddress;
    leafAmount++;
    (*leafArr)++;
    (*parentArr)++;
    (*leafVirtualAddrArr)++;
}

int
getFreeFrame(int &maxFrameIndex, word_t *leafArr, uint64_t *parentArr, uint64_t *leafVirtualAddrArr, word_t *safe,
             int &leafAmount) {
    uint64_t addrArr[MAX_LEAVES] = {0};
    int depth = 0;
    uint64_t virtualAddress = 0;
    bool hasChildren;
    bool subtreeDone;
    word_t curFrame = 0;
    word_t child = 0;
    auto frameBaseAddr = getLinePhysicalAddress(curFrame, 0);

    for (int i = 0; i < P1_SIZE; ++i) {
        PMread(frameBaseAddr + i, &child);
        virtualAddress = 0;
        if (child) {
            virtualAddress += getPartial(depth + 1, i);
            addrArr[depth] = getLinePhysicalAddress(0, i);  // add child in layer 1
            depth++;
            maxFrameIndex++;
            curFrame = child;
            subtreeDone = false;
            if (depth == TABLES_DEPTH) {
                updateLeaves(&leafArr, &parentArr, &leafVirtualAddrArr, leafAmount, addrArr, depth,
                             virtualAddress, curFrame);
            }
            while (depth > 0 && depth < TABLES_DEPTH && !subtreeDone) {
//                frameBaseAddr = getLinePhysicalAddress(curFrame,0);
                hasChildren = false;
                for (int j = 0; j < PAGE_SIZE + 1; ++j)  // check if it has children
                {
                    frameBaseAddr = getLinePhysicalAddress(curFrame, 0);
                    if (j == PAGE_SIZE) // Done looking here, go back up tree.
                    {
                        if (!hasChildren &&
                            notLocked(curFrame, safe))   // frame is empty and we can use it (just tell it's parent)
                        {
                            // disconnect
                            PMwrite(addrArr[depth - 1], 0);
                            return curFrame;
                        }
                        depth--;

                        curFrame = getFrameFromAddress(addrArr[depth]);   // else restore father's state
                        if (!curFrame) {
                            frameBaseAddr = 0;
                            subtreeDone = true;
                            break;
                        }
                        j = static_cast<int>(addrArr[depth]) - (curFrame * PAGE_SIZE);  //TODO check validity
                        virtualAddress -= getPartial(depth + 1, j);
                        hasChildren = true;
                        continue;
                    }
                    PMread(frameBaseAddr + j, &child);
                    if (child) {
                        virtualAddress += getPartial(depth + 1, j);
                        hasChildren = true;
                        addrArr[depth] = getLinePhysicalAddress(curFrame, j);
                        depth++;
                        maxFrameIndex++;  //todo
                        curFrame = child;

                        if (depth == TABLES_DEPTH)  // this is a leaf checker
                        {
//                            if (isEmptyTable(curFrame)) {
//                                // disconnect
//                                PMwrite(addrArr[depth - 1], 0);
//                                return curFrame;
//                            }
                            updateLeaves(&leafArr, &parentArr, &leafVirtualAddrArr, leafAmount, addrArr, depth,
                                         virtualAddress, curFrame);
                            //restore state of father and continue checking leaves.
                            curFrame = getFrameFromAddress(addrArr[depth - 1]);

                            j = static_cast<int>(addrArr[depth - 1]) - (curFrame * PAGE_SIZE);
                            virtualAddress -= getPartial(depth, j);
                            depth--;
                            hasChildren = true;
                            continue;
                        }
                        break;  // not leaf
                    }
                }
            }
        }
    }
    return -1;
}

int VMwrite(uint64_t virtualAddress, word_t value) {
    if (!isLegalAddress(virtualAddress)) {
        return 0;
    }
    uint64_t remainingAddress;
    word_t physicalWordAddress;
    VMaccess(virtualAddress, remainingAddress, physicalWordAddress);
    PMwrite(physicalWordAddress * PAGE_SIZE + remainingAddress, value);
    return 1;
}

word_t connectNewFrame(uint64_t virtualAddress, uint64_t currentSegment, word_t prevAddress, int i, int freeFrame) {
    clearTable(freeFrame);
    //is leaf
    if (i == TABLES_DEPTH - 1) {
        PMrestore(freeFrame, virtualAddress >> OFFSET_WIDTH);
    }
    PMwrite(prevAddress * PAGE_SIZE + currentSegment, freeFrame);
    return freeFrame;
}

void VMaccess(uint64_t virtualAddress, uint64_t &remainingAddress, word_t &physicalWordAddress) {
    remainingAddress = virtualAddress;
    physicalWordAddress = 0;
    int maxFrameIndex = 0;
    word_t physicalLeafArr[MAX_LEAVES] = {0}; //todo check if root is 0
    uint64_t parentArr[MAX_LEAVES] = {0};
    uint64_t virtualLeafArr[MAX_LEAVES] = {0};
    int leafAmount = 0;
    word_t safe[TABLES_DEPTH] = {0};
    word_t prevAddress = 0;
    int i = 0;
    unsigned int curLength = VIRTUAL_ADDRESS_WIDTH;
    uint64_t currentSegment = remainingAddress >> (curLength - P1_WIDTH);
    remainingAddress = remainingAddress - (currentSegment << (curLength - P1_WIDTH));
    curLength -= P1_WIDTH;
    PMread(0 + currentSegment, &physicalWordAddress);


    if (physicalWordAddress == 0) {
        int freeFrame = getFreeFrame(maxFrameIndex, physicalLeafArr, parentArr, virtualLeafArr, safe, leafAmount);
        if (freeFrame == -1) {
            if (maxFrameIndex < NUM_FRAMES - 1) {
                freeFrame = maxFrameIndex + 1;
            } else {         //is leaf
                int index = getFarthestFrameIndex(virtualAddress >> OFFSET_WIDTH, virtualLeafArr, leafAmount);
                disconnect(parentArr[index], virtualLeafArr[index], physicalLeafArr[index]);
                freeFrame = physicalLeafArr[index];
            }
        }
        lock(freeFrame, safe);
        clearTable(freeFrame);
        if (i == TABLES_DEPTH - 1) {
            PMrestore(freeFrame, virtualAddress >> OFFSET_WIDTH);
        }
        PMwrite(prevAddress * PAGE_SIZE + currentSegment, freeFrame);
        physicalWordAddress = freeFrame;
    }


    for (i = 1; i < TABLES_DEPTH; ++i) {
        currentSegment = remainingAddress >> (curLength - OFFSET_WIDTH);  // extracting Pi bits
        remainingAddress = remainingAddress -
                           (currentSegment << (curLength - OFFSET_WIDTH));  // chopping Pi from the remainingAddress
        curLength -= OFFSET_WIDTH;
        prevAddress = physicalWordAddress;
        lock(prevAddress, safe);
        PMread(physicalWordAddress * PAGE_SIZE + currentSegment, &physicalWordAddress);
        if (physicalWordAddress == 0) {    // Dead end
            reinitialize(physicalLeafArr, parentArr, virtualLeafArr, maxFrameIndex, leafAmount);
            int freeFrame = getFreeFrame(maxFrameIndex, physicalLeafArr, parentArr, virtualLeafArr, safe,
                                         leafAmount);

            if (freeFrame == -1) {
                if (maxFrameIndex < NUM_FRAMES - 1) {
                    freeFrame = maxFrameIndex + 1;
                } else {
                    int index = getFarthestFrameIndex(virtualAddress >> OFFSET_WIDTH, virtualLeafArr, leafAmount);
                    disconnect(parentArr[index], virtualLeafArr[index], physicalLeafArr[index]);
                    freeFrame = physicalLeafArr[index];
                    safe[i] = freeFrame;
                }
            }
            physicalWordAddress = connectNewFrame(virtualAddress, currentSegment, prevAddress, i, freeFrame);
        }

    }
}

int VMread(uint64_t virtualAddress, word_t *value) {
    if (!isLegalAddress(virtualAddress)) {
        return 0;
    }
    uint64_t remainingAddress;
    word_t physicalWordAddress;
    VMaccess(virtualAddress, remainingAddress, physicalWordAddress);
    PMread(physicalWordAddress * PAGE_SIZE + remainingAddress, value);
    return 1;
}