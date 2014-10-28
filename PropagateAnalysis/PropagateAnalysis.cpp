// PropagateAnalysis.cpp
// 
// Analyze how the proporty of a value propagates 
// among function calls.
// Propagates priv_lower call up to caller function for
// global live analysis

 
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#incude <unordered_map>

#define TARGET_FUNC "priv_lower"

using namespace llvm;

namespace {

  // PrivAnalysis structure
  struct PropagateAnalysis : public CallGraphSCCPass {
    static char ID;
    PrivAnalysis() : FunctionPass (ID) {}

    // Do initialization
    virtual bool doInitialization(CallGraph &CG){
      // Init data structure
      // TODO:
    }

    // Run on Each Function
    virtual bool runOnSCC (CallGraphSCC &SCC){
      

      return false;
    }
  }; // endof struct PropagateAnalysis
}

char PrivAnalysis::ID = 0;
static RegisterPass<PropagateAnalysis> X("PropagateAnalysis", "Value propagate analysis among call graph.");

