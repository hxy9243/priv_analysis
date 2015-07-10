// ====-----------------------  ADT.h -----------*- C++ -*---====
//
// The ADT and configuration for the PrivAnalysis
//
// ====-------------------------------------------------------====

#ifndef __ADT_H__
#define __ADT_H__

#include "llvm/IR/Module.h"

#include <linux/capability.h>
#include <cstdint>

#include <map>
#include <array>
#include <unordered_map>
#include <string>

// Constant Definition
#define TARGET_FUNC  "priv_raise"
#define PRIVRAISE    "priv_raise"
#define PRIVLOWER    "priv_lower"
#define CAP_TOTALNUM (CAP_LAST_CAP + 1)

namespace llvm {
namespace privAnalysis {


// type definition

// using 64bit long unsigned to represent all 37 capabilities
// typedef std::array<bool, CAP_TOTALNUM> CAPArray_t;
typedef uint64_t CAPArray_t;

// The map from functions to CAPArray
typedef std::unordered_map<Function*, CAPArray_t> FuncCAPTable_t;

// The map from functions to CAPArray
typedef std::unordered_map<Function*, CAPArray_t> FuncCAPTable_t;

// The map from basicblocks to CAPArray
typedef std::unordered_map<BasicBlock*, CAPArray_t> BBCAPTable_t;

// The map from basicblocks to functions
typedef std::unordered_map<BasicBlock*, Function*> BBFuncTable_t;

// The unique capabiltiy set for all basic blocks mapped to the number of its CAPs
typedef std::map<CAPArray_t, int> CAPSet_t;

// --------------------------- //
// Data manipulation functions
// --------------------------- //
// Get the function where the CallInst is in, 
// add to map from Function to CAPArray
void AddToFuncCAPTable(FuncCAPTable_t &CAPTable, 
                       Function *F, CAPArray_t CAParray);

// Get the BasicBlock where the CallInst is in
// add to map from BasicBlock to CAPArray
void AddToBBCAPTable(BBCAPTable_t &CAPTable, 
                     BasicBlock *B, CAPArray_t CAParray);

// Copy CAPTable keys from src to dest, with array empty
void CopyTableKeys(FuncCAPTable_t &dest, const FuncCAPTable_t &src);

// ------------------- //
// Array manipulations
// ------------------- //
// Find the size of the input array
int findCAPArraySize(const CAPArray_t &A);

// Union two arrays, save result to dest and test dest ischanged
bool UnionCAPArrays(CAPArray_t &dest, const CAPArray_t &src);

// Diff two arrays, save result
// return if there is difference
bool DiffCAPArrays(CAPArray_t &dest, const CAPArray_t &A, const CAPArray_t &B); 

// Find the reverse of cap array
void ReverseCAPArray(CAPArray_t &A);

// If the CAPArray is empty
bool IsCAPArrayEmpty(const CAPArray_t &A);

// Dump CAPArray 
void dumpCAPArray(raw_ostream &O, const CAPArray_t &A);

void dumpCAPArray(const CAPArray_t &A);

// dump CAPTable for Debugging purpose
// param: CT - the CAPTable to dump
void dumpCAPTable(const FuncCAPTable_t &CT);

} // namespace privAnalysis
} // namepsace llvm

#endif
