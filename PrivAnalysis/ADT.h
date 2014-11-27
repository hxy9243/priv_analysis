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

  // Data manipulation functions
  // Copy CAPTable keys from src to dest, with array empty
  void CopyTableKeys(FuncCAPTable_t &dest, FuncCAPTable_t &src);

  // Union two arrays, save result to dest and test dest ischanged
  bool UnionCAPArrays(CAPArray_t &dest, CAPArray_t &src);

  // for debugging purpose
  void dumpCAPTable(FuncCAPTable_t &CT);


} // namespace privAnalysis
} // namepsace llvm

#endif
