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

#include <unordered_map>

using namespace llvm;

#define TARGET_FUNC "priv_lower"

std::unordered_map<llvm::Function, std::array>CAPTable;

namespace {

  // PrivAnalysis structure
  struct PropagateAnalysis : public CallGraphSCCPass {
    static char ID;
    PropagateAnalysis () : CallGraphSCCPass (ID) {}

    // Do initialization
    virtual bool doInitialization (CallGraph &CG){
      // Init data structure
      // TODO:



    }

    // Run on CallGraph SCC
    virtual bool runOnSCC (CallGraphSCC &SCC){
      
      // Iterate over CallGraphNodes inside SCC
      for (CallGraphNode *CGNI = SCC.begin (), *CGNE = SCC.end ();
	   CGNI != CGNE;
	   CGNI ++){

	// Get the function 
	Function *CalleeFunc = CGNI->getFunction ();

	  // Iterate all Call Records inside CallGraphNodes 
	for (CallRecord CRI = SCC.begin(), CRE = SCC.end ();
	     CRI != CRE;
	     CRI ++){

	  // Get the callee of current call record
	  CallGraphNode *CalleeNode = CRI.second;



	}

      }

      return false;
    }


    // preserve all analyses
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }

  }; // endof struct PropagateAnalysis
}

char PrivAnalysis::ID = 0;
static RegisterPass<PropagateAnalysis> X("PropagateAnalysis", "Value propagate analysis among call graph.");

