// ====-----------  FindExternNodes.h ------------*- C++ -*---====
//
// Helper pass. Find external nodes which use privileges 
//
// In LLVM, there's an external calling node calling all functions
// with external visiblities
// 
// This pass identifies functions with following properties
// 1. Still have external linkage after 'internalize' opt pass
// 2. Not empty (not function declaration)
// 3. Uses privileges inside function
// 
// These are functions we need to pay special attention to. As
// they are used as function pointers and their callsites aren't 
// specifically marked in the control flow graph.
// 
// ====-------------------------------------------------------====


#ifndef __FINDEXTERNNODES_H__
#define __FINDEXTERNNODES_H__

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/raw_ostream.h"

#include "ADT.h"
#include "LocalAnalysis.h"
#include "PropagateAnalysis.h"

using namespace llvm::privAnalysis;

namespace llvm {
namespace findexternnodes {

// PropagateAnalysis class
struct FindExternNodes : public ModulePass 
{
public:
    // pass ID
    static char ID;

    // CAPTable after info propagation
    FuncCAPTable_t ExternPrivNodes;

    FindExternNodes();

    void getAnalysisUsage(AnalysisUsage &AU) const;

    bool doInitialization(Module &M);

    bool runOnModule(Module &M);

    void print(raw_ostream &O, const Module *M) const;
};

} // namespace findexternnodes
} // namespace llvm

#endif
