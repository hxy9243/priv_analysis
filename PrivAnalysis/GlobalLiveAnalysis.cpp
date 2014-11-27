// ====------------- GlobalLiveAnalysis.cpp ------*- C++ -*---====
//
// Globally Privilge Live Analysis for each basic
// blocks. The output would be the privileges to drop for each
// basic blocks
//
// ====-------------------------------------------------------====

#include "llvm/Analysis/CallGraph.h"
#include "llvm/Pass.h"

#include "GlobalLiveAnalysis.h"
#include "PropagateAnalysis.h"
#include "LocalAnalysis.h"

#include <array>
#include <vector>
#include <map>


using namespace llvm;
using namespace llvm::propagateAnalysis;
using namespace llvm::splitBB;
using namespace llvm::globalLiveAnalysis;


// GlobalLiveAnalysis constructor
GlobalLiveAnalysis::GlobalLiveAnalysis() : ModulePass(ID) {}


// Require Analysis usage
void GlobalLiveAnalysis::getAnalysisUsage(AnalysisUsage &AU) const{
  AU.addRequired<PropagateAnalysis>();
  AU.addRequired<SplitBB>();

}


// Do initialization
bool GlobalLiveAnalysis::doInitialization(Module &M){

  return false;
}


// Run on Module
bool GlobalLiveAnalysis::runOnModule(Module &M){

  PropagateAnalysis &PA = getAnalysis<PropagateAnalysis>();

  // retrieve all data structures
  FuncCAPTable_t &FuncCAPTable = PA.FuncCAPTable;
  BBCAPTable_t &BBCAPTable = PA.BBCAPTable;
  BBFuncTable_t &BBFuncTable = PA.BBFuncTable;
  
  // init data structure
  bool change = true;
  

  // iterate till convergence
  while (change){
    change = false;

    // iterate through all functions
    for (Module::iterator FI = M.begin(), FE = M.end();
         FI != FE;
         ++ FI){
      Function *F = dyn_cast<Function>(FI);

      // iterate all BBs
      for (Function::iterator BI = F->begin(), BE = F->end();
           BI != BE;
           ++ BI){
        


      } // iterate all BBs

    } // iterate all functions
  } // while change

  return false;
}


// register pass
char GlobalLiveAnalysis::ID = 0;
static RegisterPass<GlobalLiveAnalysis> L("GlobalLiveAnalysis", "Global privilege live analysis", true, true);

