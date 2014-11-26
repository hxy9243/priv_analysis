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


  return false;
}


// register pass
char GlobalLiveAnalysis::ID = 0;
static RegisterPass<GlobalLiveAnalysis> L("GlobalLiveAnalysis", "Global privilege live analysis", true, true);

