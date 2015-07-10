// ====-----------------------  ADT.h -----------*- C++ -*---====
//
// The ADT and configuration for the PrivAnalysis
//
// ====------------------------------------------------------====

#include "ADT.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <array>
#include <algorithm>
#include <utility>


namespace llvm {
namespace privAnalysis {


// capability number to string for ROSA
const char *CAPString[CAP_TOTALNUM] = {
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


// Add to the FuncCAPTable, if Func exists, merge the CAPArray
// param: CAPTable - ref to the FuncCAPTable
//        F - the function to add
//        CAParray - the array of capability to add to FuncCAPTable
void AddToFuncCAPTable(FuncCAPTable_t &CAPTable, Function *F, 
                       CAPArray_t CAParray)
{
    // If not found in map, add to map
    if (CAPTable.find(F) == CAPTable.end()) {
        CAPTable[F] = CAParray;
    }
    // else, Union the two arrays
    else {
        CAPTable[F] |= CAParray;
    }
}


// Add to the BBCAPTable, if BB exists, merge the CAPArray
// param: CAPTable - ref to the BBCAPTable
//        B - the BasicBlock to add
//        CAParray - the array of capability to add to FuncCAPTable
void AddToBBCAPTable(BBCAPTable_t &CAPTable, BasicBlock *B, 
                     CAPArray_t CAParray)
{
    // If not found in map, add to map
    if (CAPTable.find(B) == CAPTable.end() ) {
        CAPTable[B] = CAParray;
    }
    // else, Union the two arrays
    else {
        CAPTable[B] |= CAParray;
    }
}


// Copy CAPTable keys from src to dest
// After this operation, dest would have all keys src
// has, with empty CAPArrays mapping to each keys
// param: dest - dest CAPTable
//        src  - src CAPTable
void CopyTableKeys(FuncCAPTable_t &dest, const FuncCAPTable_t &src)
{
    CAPArray_t emptyArray = 0;

    for(auto MI = src.begin(), ME = src.end(); MI != ME; ++ MI) {
  
        Function *tf = MI->first;

        dest[tf] = emptyArray;
    }
}


// Find the size of the input array
// param: A - the input array
// return the number of the capablities inside CAPArray 
int findCAPArraySize(const CAPArray_t &A)
{
    // to be refactored
    int size = 0;
    for (int i = 0; i < CAP_TOTALNUM; ++i){
        size += ((uint64_t)1 << i) & A;
    }

    return size;
}

  
// Union two arrays, save result to dest
// param: dest - dest CAPArray
//        src  - src CAPArray
// return: ischanged - if the dest's value is changed
bool UnionCAPArrays(CAPArray_t &dest, const CAPArray_t &src)
{
    bool ischanged = false;

    ischanged = ~dest & src;
    dest = dest | src;

    return ischanged;
}


// Diff two arrays, save result to dest
// param: dest - dest CAPArray
//        A - the CAPArray to subtract from
//        B - the CAPArray to subtract
// return: if there is difference between A and B
bool DiffCAPArrays(CAPArray_t &dest, const CAPArray_t &A, const CAPArray_t &B) 
{
    assert( (A | ~B) && "\nBUG in Diff CAPArrays!\n\n");
    dest = A & ~B;
    return (bool)dest;
} 


// Find the reverse of cap array
// param: the CAParray to reverse
void ReverseCAPArray(CAPArray_t &A) 
{
    A = ~A;
}

// If the CAPArray is empty
// @A: the CAPArray to examine
// return: if it's empty
bool IsCAPArrayEmpty(const CAPArray_t &A)
{
    return A == 0;
}


// dump CAPArray for debugging purpose
void dumpCAPArray(raw_ostream &O, const CAPArray_t &A) {
    if (A == 0) {
        O << "empty,";
    }

    for (int i = 0; i < CAP_TOTALNUM; ++i) {
        if (A & ((uint64_t)1 << i)) {
            O << CAPString[i] << ",";
        }
    }

    O << "\n";
}

void dumpCAPArray(const CAPArray_t &A) {
    
    dumpCAPArray(errs(), A);

}


// dump CAPTable for Debugging purpose
// param: CT - the CAPTable to dump
void dumpCAPTable(const FuncCAPTable_t &CT)
{
    // iterate through captable, a map from func to array
    for (auto mi = CT.begin(), me = CT.end();
         mi != me; ++ mi){

        errs() << mi->first->getName() << " Privileges:\t";

        // iterate through cap array
        for (int i = 0; i < CAP_TOTALNUM; ++i) {
            if (mi->second & ((uint64_t)1 << i)) {
                errs() << CAPString[i] << "\t";
            }
        }
        errs() << "\n";
    }
    errs() << "\n";
}


} // namespace privAnalysis
} // namepsace llvm
