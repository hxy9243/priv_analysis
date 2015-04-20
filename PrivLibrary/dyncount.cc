/*
 * Dynamic Counting library for LLVM Privilege Analysis pass
 * 
 * author: Kevin Hu <hxy9243@gmail.com>
 *
 */

#include "priv.h"
#include "dyncount.h"


#include <cstdio>
#include <cstdlib>
#include <string>
#include <map>
#include <utility>
#include <vector>
#include <algorithm>


static const char *CAPString[CAP_NUM] = {
    "CapChown",
    "CapDacOverride",
    "CapDacReadSearch",
    "CapFowner",
    "CapFsetid",
    "CapKill",
    "CapSetgid",
    "CapSetuid",
    "CapSetpCap",
    "CapLinuxImmutable",
    "CapNetBindService",
    "CapNetBroadcast",
    "CapNetAdmin",
    "CapNetRaw",
    "CapIpcLock",
    "CapIpcOwner",
    "CapSysModule",
    "CapSysRawio",
    "CapSysChroot",
    "CapSysPtrace",
    "CapSysPacct",
    "CapSysAdmin",
    "CapSysBoot",
    "CapSysNice",
    "CapSysResource",
    "CapSysTime",
    "CapSysTtyConfig",
    "CapMknod",
    "CapLease",
    "CapAuditWrite",
    "CapAuditControl",
    "CapSetfCap",
    "CapMacOverride",
    "CapMacAdmin",
    "CapSyslog",
    "CapWakeAlarm",
    "CapBlockSuspend"
};


// The capability set count of LOC
static std::map<uint64_t, int>CAPSetLOCCount;



// Internal method for printing out capabilities
void printCAP(uint64_t CAPArray)
{
    uint64_t mask = (uint64_t) 1 << CAP_NUM;

    if (CAPArray == (uint64_t)0) {
        printf("empty,");
        return;
    }

    for (int i = 0; CAPArray != (uint64_t) 0; ++i) {
        if (CAPArray & mask) {
            printf("%s,", CAPString[i]);
        }

        CAPArray &= ~mask;
        mask >>= 1;
    }
    return;
}


// Internal method for counting live capability in uint64_t CAPArray
int countCAP(uint64_t CAPArray)
{
    // TODO: This is naive and inefficient. Find a quicker way
    int count = 0;

    while(CAPArray) {
        count += CAPArray & 1;
        CAPArray >>= 1;
    }

    return count;
}



// Internal method for sorting map, compare CAPSetLOCCount by live CAPs in CAPArray 
bool compareCAPLOC(const std::pair<uint64_t, int> A,
                  const std::pair<uint64_t, int> B)
{
    return countCAP(A.first) > countCAP(B.first);
}



/* init the counting data structure */
int initCount()
{
    CAPSetLOCCount.clear();
    
    return 0;
}


/* add LOC to the CAPArray data structure 
 * This function is to be inserted into every Basic Block for 
 * counting the LOC of capability set of the Basic Block
 * param: LOC - the LOC of the capability set
 *        CAPArray - the capability set represented in uint64_t
 */
int addCount(int LOC, uint64_t CAPArray)
{

    CAPSetLOCCount[CAPArray] += LOC;
    
    return 0;
}


/* report the counting data structure */
int reportCount()
{
    // copy origin map to sort
    std::vector<std::pair<uint64_t, int> >CAPSetLOCCopy(CAPSetLOCCount.begin(),
                                                        CAPSetLOCCount.end());
    std::sort(CAPSetLOCCopy.begin(), CAPSetLOCCopy.end(), compareCAPLOC);

    for (std::vector<std::pair<uint64_t, int> >::iterator 
             i = CAPSetLOCCopy.begin(), e = CAPSetLOCCopy.end();
         i != e; ++i) {

        printCAP((*i).first);

        printf(" LOC count: %d\n", (*i).second);

    }

    return 0;
}
