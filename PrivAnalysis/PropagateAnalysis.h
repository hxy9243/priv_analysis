// ====----------  PropagateAnalysis.cpp ---------*- C++ -*---====
//
// Analysis the call graph, propagate the live privilege property
// from callee up to callers.
//
// ====-------------------------------------------------------====

#ifndef __PROPAGTEANALYSIS_H__
#define __PROPAGTEANALYSIS_H__

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/raw_ostream.h"

#include "ADT.h"

using namespace llvm::privAnalysis;

namespace llvm{
namespace propagateAnalysis{

  // PropagateAnalysis class
  struct PropagateAnalysis : public ModulePass {
  public:
    // pass ID
    static char ID;

    // CAPTable after info propagation
    CAPTable_t CAPTable;

    // constructor
    PropagateAnalysis();

    // get analysis usage
    void getAnalysisUsage(AnalysisUsage &AU) const;

    // do initialization
    virtual bool doInitialization(Module &M);

    // Run on CallGraph SCC
    virtual bool runOnModule(Module &M);

  };

} // namespace propagateAnalysis
} // namespace llvm

#endif
