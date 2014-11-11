// ====---------------  PrivAnalysis.cpp ---------*- C++ -*---====
// 
// Analysis the call graph, propagate the live privilege property
// from callee up to callers.
//
// ====-------------------------------------------------------====

#include "PropagateAnalysis.h" 
#include "LocalAnalysis.h"

#include <array>
#include <vector>
#include <map>

using namespace llvm;
using namespace llvm::localAnalysis;
using namespace llvm::propagateAnalysis;

char PropagateAnalysis::ID = 0;
static RegisterPass<PropagateAnalysis> X("PropagateAnalysis", "Privilege Propagate Analysis.");

// PropagateAnalysis constructor
PropagateAnalysis::PropagateAnalysis() : CallGraphSCCPass(ID) {}


// Require Analysis Usage
void PropagateAnalysis::getAnalysisUsage(AnalysisUsage &AU) const{
  AU.setPreservesCFG();
  AU.addRequired<LocalAnalysis>();

  // preserve usage
  AU.setPreservesAll();
}


// Do initialization
// param: CG - the call graph
bool PropagateAnalysis::doInitialization(CallGraph &CG){
  // Init data structure
  // TODO:
  LocalAnalysis &LA = getAnalysis<LocalAnalysis>();
  dumpCAPTable(LA.CAPTable);
  
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
// param: SCC - call graph strongly coupled components
bool PropagateAnalysis::runOnSCC(CallGraphSCC &SCC){
      
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

