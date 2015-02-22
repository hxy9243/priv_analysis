// ====-----------------------  ADT.h -----------*- C++ -*---====
//
// The ADT and configuration for the PrivAnalysis
//
// ====-------------------------------------------------------====

#ifndef __ADT_H__
#define __ADT_H__

#include "llvm/IR/Module.h"

#include <linux/capability.h>
#include <map>
#include <array>
#include <unordered_map>
#include <string>

// Constant Definition
#define TARGET_FUNC "priv_raise"
#define PRIVRAISE "priv_raise"
#define PRIVLOWER "priv_lower"
#define CAP_TOTALNUM (CAP_LAST_CAP + 1)

namespace llvm {
namespace privAnalysis {

// capability number to string for ROSA
static const std::string CAPString[CAP_TOTALNUM] = {
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


// type definition
// The array of bool representing all Capabilities
typedef std::array<bool, CAP_TOTALNUM> CAPArray_t;

// The map from functions to CAPArray
typedef std::map<Function *, CAPArray_t> FuncCAPTable_t;

// The map from basicblock to CAPArray
typedef std::map<BasicBlock *, CAPArray_t> BBCAPTable_t;

// The map from basicblock to CAPArray
typedef std::map<BasicBlock *, Function*> BBFuncTable_t;

// The unique capabiltiy set for all basic blocks
typedef std::map<CAPArray_t, BasicBlock *> CAPSet_t;


// Data manipulation functions
// Get the function where the CallInst is in, add to map
void AddToFuncCAPTable(FuncCAPTable_t &CAPTable, 
                       Function *F, CAPArray_t CAParray);

// Get the BasicBlock where the CallInst is in, add to map
void AddToBBCAPTable(BBCAPTable_t &CAPTable, 
                     BasicBlock *B, CAPArray_t CAParray);

// Copy CAPTable keys from src to dest, with array empty
void CopyTableKeys(FuncCAPTable_t &dest, FuncCAPTable_t &src);

// Union two arrays, save result to dest and test dest ischanged
bool UnionCAPArrays(CAPArray_t &dest, CAPArray_t &src);

// Diff two arrays, save result
// return if the difference has content (not all 0s)
bool DiffCAPArrays(CAPArray_t &dest, CAPArray_t &A, CAPArray_t &B); 

// Find the reverse of cap array
void ReverseCAPArray(CAPArray_t &A);

// If the CAPArray is empty
bool IsCAPArrayEmpty(CAPArray_t &A);


// for debugging purpose
void dumpCAPArray(raw_ostream &O, const CAPArray_t &A);

void dumpCAPArray(const CAPArray_t &A);

void dumpCAPTable(FuncCAPTable_t &CT);


} // namespace privAnalysis
} // namepsace llvm

#endif
