// ====----------  PropagateAnalysis.cpp ---------*- C++ -*---====
//
// Analysis the call graph, propagate the live privilege property
// from callee up to callers.
// The output of the pass is the whole set of the privilege each 
// function requires to run
//
// ====-------------------------------------------------------====

#include "llvm/Analysis/CallGraph.h"

#include "PropagateAnalysis.h"
#include "LocalAnalysis.h"

#include <array>
#include <vector>
#include <map>

using namespace llvm;
using namespace llvm::privAnalysis;
using namespace llvm::localAnalysis;
using namespace llvm::propagateAnalysis;

// PropagateAnalysis constructor
PropagateAnalysis::PropagateAnalysis() : ModulePass(ID) {}


// Require Analysis Usage
void PropagateAnalysis::getAnalysisUsage(AnalysisUsage &AU) const{
  errs() << "getting analysis usage\n";

  AU.setPreservesCFG();
  AU.addRequired<LocalAnalysis>();

  // preserve usage
  AU.setPreservesAll();
}


// Do initialization
// param: CG - the call graph
bool PropagateAnalysis::doInitialization(Module &M){
  // Init data structure
  // TODO:
  errs() << "initialization\n";
  CallGraph CG(M);

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
bool PropagateAnalysis::runOnModule(Module &M){

  LocalAnalysis &LA = getAnalysis<LocalAnalysis>();

  // Get CAPTable for propagation anlysis
  CAPTable = LA.CAPTable;

  // DEBUG
  errs() << "Dump table before propagation\n";
  dumpCAPTable(CAPTable);

  // Depth First Search Analysis for data propagation
  Propagate(M, CAPTable);

  errs() << "Dump table after propagation\n";
  dumpCAPTable(CAPTable);
  // Iterate over CallGraphNodes inside SCC
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


// Depth First Search data propagation analysis
// param: CG - the callgraph to analyse
//        CAPTable - the captable to store live analysis data
void PropagateAnalysis::Propagate(Module &M, CAPTable_t &CAPTable){

  // The ins and outs of function
  CAPTable_t CAPTable_in;
  CAPTable_t CAPTable_out;

  CopyTableKeys(CAPTable_in, CAPTable);
  CopyTableKeys(CAPTable_out, CAPTable);

  bool ischanged = true;

  // Get Callgraph
  CallGraph CG(M);

  // Keep iterating until converged
  while (ischanged){
    ischanged = false;

    // Iterate through the callgraph
    for (CallGraph::iterator CI = CG.begin(), CE = CG.end();
         CI != CE;
         ++ CI){
      // Get CallgraphNode
      CallGraphNode *N = CI->second;
      Function *FCaller = N->getFunction();
      // protector
      if (!FCaller){
        continue;
      }

      // Get Caller mapped array in CAPTables
      CAPArray_t &callerIn = CAPTable_in[FCaller];
      CAPArray_t &callerOut = CAPTable_out[FCaller];
      // Iterate through Callgraphnode for callees
      for (CallGraphNode::iterator RI = N->begin(), RE = N->end();
           RI != RE;
           ++ RI){
        // Get callee
        Function *FCallee = RI->second->getFunction();
        if (!FCallee){
          continue;
        }

        CAPArray_t &calleeIn = CAPTable_in[FCallee];
        // Propagate all information from callee to caller_out 
        UnionCAPArrays(callerOut, calleeIn);

      } // Iterate through Callgraphnode for callees

      // Propagate all information from caller_out to caller_in
      UnionCAPArrays(callerOut, CAPTable[FCaller]);
      ischanged |= UnionCAPArrays(callerIn, callerOut);

    } // iterator for caller nodes

    dumpCAPTable(CAPTable_in);

    errs() << "Propagate Iteration!\n";
  } // main loop

  CAPTable = CAPTable_in;

}




// register pass
char PropagateAnalysis::ID = 0;
static RegisterPass<PropagateAnalysis> X("PropagateAnalysis", "Privilege Propagate Analysis.");
