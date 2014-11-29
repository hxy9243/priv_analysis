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
bool PropagateAnalysis::doInitialization(Module &M){
  return false;
}


// Run on Module
// param: M - the program Module
bool PropagateAnalysis::runOnModule(Module &M){

  errs() << "\nRunning CallGraph propagation pass\n\n";

  LocalAnalysis &LA = getAnalysis<LocalAnalysis>();

  // Get all data structures for propagation analysis
  FuncCAPTable = LA.FuncCAPTable;
  BBCAPTable = LA.BBCAPTable;
  BBFuncTable = LA.BBFuncTable;

  // DEBUG
  errs() << "Dump table before propagation\n";
  dumpCAPTable(FuncCAPTable);

  // Depth First Search Analysis for data propagation
  Propagate(M, FuncCAPTable);

  errs() << "Dump table after propagation\n";

  dumpCAPTable(FuncCAPTable);

  return false;
}


// Depth First Search data propagation analysis
// param: M - the program module
//        FuncCAPTable - the captable to store live analysis data
void PropagateAnalysis::Propagate(Module &M, FuncCAPTable_t &FuncCAPTable){

  // The ins and outs of function
  FuncCAPTable_t FuncCAPTable_in;
  FuncCAPTable_t FuncCAPTable_out;

  CopyTableKeys(FuncCAPTable_in, FuncCAPTable);
  CopyTableKeys(FuncCAPTable_out, FuncCAPTable);

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

      // Get Caller mapped array in FuncCAPTables
      CAPArray_t &callerIn = FuncCAPTable_in[FCaller];
      CAPArray_t &callerOut = FuncCAPTable_out[FCaller];
      // Iterate through Callgraphnode for callees
      for (CallGraphNode::iterator RI = N->begin(), RE = N->end();
           RI != RE;
           ++ RI){
        // Get callee
        Function *FCallee = RI->second->getFunction();
        if (!FCallee){
          continue;
        }

        CAPArray_t &calleeIn = FuncCAPTable_in[FCallee];
        // Propagate all information from callee to caller_out 
        ischanged |= UnionCAPArrays(callerOut, calleeIn);

      } // Iterate through Callgraphnode for callees

      // Propagate all information from caller_out to caller_in
      ischanged |= UnionCAPArrays(callerOut, FuncCAPTable[FCaller]);
      ischanged |= UnionCAPArrays(callerIn, callerOut);

    } // iterator for caller nodes

  } // main loop

  FuncCAPTable = FuncCAPTable_in;

}


// register pass
char PropagateAnalysis::ID = 0;
static RegisterPass<PropagateAnalysis> X("PropagateAnalysis", "Privilege Propagate Analysis.");
