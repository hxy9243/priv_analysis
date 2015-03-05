// ====-----------------------  ADT.h -----------*- C++ -*---====
//
// The ADT and configuration for the PrivAnalysis
//
// ====-------------------------------------------------------====

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
void AddToFuncCAPTable(FuncCAPTable_t &CAPTable,
                       Function *F, 
                       CAPArray_t CAParray)
{
    // If not found in map, add to map
    if (CAPTable.find(F) == CAPTable.end()) {
        CAPTable[F] = CAParray;
    }
    // else, Union the two arrays
    else {
        for (int i = 0; i < CAP_TOTALNUM; ++ i) {
            CAPTable[F][i] |= CAParray[i];
        }
    }
}


// Add to the BBCAPTable, if BB exists, merge the CAPArray
// param: CAPTable - ref to the BBCAPTable
//        B - the BasicBlock to add
//        CAParray - the array of capability to add to FuncCAPTable
void AddToBBCAPTable(BBCAPTable_t &CAPTable,
                     BasicBlock *B, 
                     CAPArray_t CAParray) 
{
    // If not found in map, add to map
    if (CAPTable.find(B) == CAPTable.end() ) {
        CAPTable[B] = CAParray;
    }
    // else, Union the two arrays
    else {
        for (int i = 0; i < CAP_TOTALNUM; ++ i) {
            CAPTable[B][i] |= CAParray[i];
        }
    }
}


// Copy CAPTable keys from src to dest
// After this operation, dest would have all keys src
// has, with empty CAPArrays mapping to each keys
// param: dest - dest CAPTable
//        src  - src CAPTable
void CopyTableKeys(FuncCAPTable_t &dest, const FuncCAPTable_t &src)
{
    CAPArray_t emptyArray = {{0}};

    for(auto MI = src.begin(), ME = src.end(); MI != ME; ++ MI) {
  
        Function *tf = MI->first;

        dest[tf] = emptyArray;
    }
}


// Find the size of the input array
// param: A - the input array
// return the number of the capablities inside CAPArray 
int findCAPArraySize(CAPArray_t &A)
{
    int size = 0;
    for (auto I = A.begin(), E = A.end(); I != E; ++I) {
        if (*I) {
            size++;
        }
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

    for (unsigned int i = 0; i < dest.size(); ++ i) {
        // changed only when dest is 0 and src is 1
        ischanged |= (~dest[i] & src[i]);
        dest[i] |= src[i];
    }
    return ischanged;
}


// Diff two arrays, save result
// param: dest - dest CAPArray
//        A - the CAPArray to subtract from
//        B - the CAPArray to subtract
// return: if the array has content (not all 0s),
//         return 0 if all capabilities are 0
bool DiffCAPArrays(CAPArray_t &dest, const CAPArray_t &A, const CAPArray_t &B) 
{
    bool hasContent = false;
    for (unsigned int i = 0; i < A.size(); ++i){
        assert( (A[i] | ~B[i]) && "\nBUG in Diff CAPArrays!\n\n");

        dest[i] |= A[i] & (~B[i]);
        hasContent |= dest[i];
    }
    return hasContent;
} 


// Find the reverse of cap array
// param: the CAParray to reverse
void ReverseCAPArray(CAPArray_t &A) 
{
    for (auto i = A.begin(), e = A.end(); i != e; ++i) {
        *i = !(*i);
    }
}

// If the CAPArray is empty
// @A: the CAPArray to examine
// return: if it's empty
bool IsCAPArrayEmpty(const CAPArray_t &A)
{
    bool notEmpty = false;
    for (auto i = A.begin(), e = A.end(); i != e; ++i) {
        notEmpty |= *i;
    }
    return !notEmpty;
}


// dump CAPArray for debugging purpose
void dumpCAPArray(raw_ostream &O, const CAPArray_t &A) {
    int cap = 0;
    bool isempty = true;

    for (auto i = A.begin(), e = A.end(); i != e; ++i) {
        if (*i) {
            if(!isempty) {
                O << ",";
            }
            isempty &= false;
            O << CAPString[cap];
        }
        cap++;
    }

    if (isempty) {
        O << "empty";
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
         mi != me;
         ++ mi){
        errs() << mi->first->getName() << " Privileges:\n";

        // iterate through cap array
        for (unsigned int i = 0; i < mi->second.size(); ++ i) {
            if (mi->second[i]) {
                errs() << i << "\t";
            }
        }
        errs() << "\n";
    }
    errs() << "\n";
}


} // namespace privAnalysis
} // namepsace llvm
