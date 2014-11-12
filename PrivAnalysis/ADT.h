// ====-----------------------  ADT.h -----------*- C++ -*---====
//
// The ADT and configuration for the PrivAnalysis
//
// ====-------------------------------------------------------====

#ifndef __ADT_H__
#define __ADT_H__


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


} // namespace privAnalysis
} // namepsace llvm

#endif
