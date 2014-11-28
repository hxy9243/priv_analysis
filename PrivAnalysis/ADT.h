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

// Constant Definition
#define TARGET_FUNC "priv_raise"
#define PRIVRAISE "priv_raise"
#define PRIVLOWER "priv_lower"
#define CAP_TOTALNUM (CAP_LAST_CAP + 1)

namespace llvm{
namespace privAnalysis{

  // type definition
  // The array of bool representing all Capabilities
  typedef std::array<bool, CAP_TOTALNUM> CAPArray_t;

  // The map from functions to CAPArray
  typedef std::map<Function *, CAPArray_t> FuncCAPTable_t;

  // The map from basicblock to CAPArray
  typedef std::map<BasicBlock *, CAPArray_t> BBCAPTable_t;

  // The map from basicblock to CAPArray
  typedef std::map<BasicBlock *, Function*> BBFuncTable_t;

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
  void DiffCAPArrays(CAPArray_t &dest, CAPArray_t &A, CAPArray_t &B); 


  // for debugging purpose
  void dumpCAPTable(FuncCAPTable_t &CT);


} // namespace privAnalysis
} // namepsace llvm

#endif
