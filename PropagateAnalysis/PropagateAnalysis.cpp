// PropagateAnalysis.cpp
// 
// Analyze how the proporty of a value propagates 
// among function calls.
// Propagates priv_lower call up to caller function for
// global live analysis

 
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Support/raw_ostream.h"

#include <array>
#include <vector>
#include <map>

using namespace llvm;

namespace {

  // PrivAnalysis structure
  struct PropagateAnalysis : public CallGraphSCCPass {
    static char ID;
    PropagateAnalysis() : CallGraphSCCPass(ID) {}

    // Do initialization
    virtual bool doInitialization(CallGraph &CG){
      // Init data structure
      // TODO:


      // Iterate through the callgraph for callgraphnodes
      for (CallGraph::iterator CI = CG.begin(), CE = CG.end();
           CI != CE;
           ++ CI){

        const CallGraphNode *N = CI->second;
        Function *FCaller = N->getFunction();
        if (!FCaller){
          continue;
        }

        // Iterate through each callgraphnode for callees
        for (auto RI = N->begin(), RE = N->end(); RI != RE; ++ RI){
          Function *FCallee = RI->second->getFunction();
          if (!FCallee){
            continue;
          }

          // DEBUG
          //////////////
          errs() << FCaller->getName()
                 << " Calls function "
                 << FCallee->getName() 
                 << "\n";
          //////////////
        }
      }

      return false;
    }

    // Run on CallGraph SCC
    virtual bool runOnSCC(CallGraphSCC &SCC){
      
      // // Iterate over CallGraphNodes inside SCC
      // for (CallGraphNode *CGNI = SCC.begin (), *CGNE = SCC.end ();
      //      CGNI != CGNE;
      //      CGNI ++){
      //   // Get the function 
      //   Function *CalleeFunc = CGNI->getFunction ();
      //     // Iterate all Call Records inside CallGraphNodes 
      //   for (CallRecord CRI = SCC.begin(), CRE = SCC.end ();
      //        CRI != CRE;
      //        CRI ++){
      //     // Get the callee of current call record
      //     CallGraphNode *CalleeNode = CRI.second;
      //   }
      // }

      return false;
    }


    // preserve all analyses
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }

  }; // endof struct PropagateAnalysis
}

char PropagateAnalysis::ID = 0;
static RegisterPass<PropagateAnalysis> X("PropagateAnalysis", "Privilege Propagate Analysis.");

