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
#include "llvm/Pass.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Support/raw_ostream.h"

#include <array>
#include <vector>
#include <map>

namespace llvm{
namespace propagateAnalysis{

  // PrivAnalysis structure
  struct PropagateAnalysis : public CallGraphSCCPass {
  public:
    // pass ID
    static char ID;

    // constructor
    PropagateAnalysis();

    // get analysis usage
    void getAnalysisUsage(AnalysisUsage &AU) const;

    // do initialization
    virtual bool doInitialization(CallGraph &CG);

    // Run on CallGraph SCC
    virtual bool runOnSCC(CallGraphSCC &SCC);

  }; 

} // namespace propagateAnalysis
} // namespace llvm

#endif
