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
#define CAP_TOTALNUM (CAP_LAST_CAP + 1)

namespace llvm{
namespace privAnalysis{

  // type definition
  // The array of bool representing all Capabilities
  typedef std::array<bool, CAP_TOTALNUM> CAPArray_t;
  // The map from functions to CAPArray
  typedef std::map<Function *, CAPArray_t> CAPTable_t;

  // Data manipulation functions
  // Copy CAPTable keys from src to dest, with array empty
  void CopyTableKeys(CAPTable_t &dest, CAPTable_t &src);

  // Union two arrays, save result to dest and test dest ischanged
  bool UnionCAPArrays(CAPArray_t &dest, CAPArray_t &src);

  // for debugging purpose
  void dumpCAPTable(CAPTable_t &CT);


} // namespace privAnalysis
} // namepsace llvm

#endif
