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
  AU.addRequired<UnifyFunctionExitNodes>();
}


// Do initialization
bool GlobalLiveAnalysis::doInitialization(Module &M){

  return false;
}


// Run on Module
bool GlobalLiveAnalysis::runOnModule(Module &M){

  PropagateAnalysis &PA = getAnalysis<PropagateAnalysis>();
  UnifyFunctionExitNodes &UnifyExitNode = getAnalysis<UnifyFunctionExitNodes>();

  // retrieve all data structures
  FuncCAPTable_t &FuncUseCAPTable = PA.FuncCAPTable;
  BBCAPTable_t &BBCAPTable = PA.BBCAPTable;
  BBFuncTable_t &BBFuncTable = PA.BBFuncTable;
  
  // init data structure
  bool ischanged = true;
  
  // FuncLiveCAPTable maps from Functions to the 
  // live CAP in the Functions
  FuncCAPTable_t FuncLiveCAPTable;
  BBCAPTable_t BBCAPTable_in;
  BBCAPTable_t BBCAPTable_out;

  // Find all BBs that contains return;
  

  // iterate till convergence
  while (ischanged){
    ischanged = false;

    // iterate through all functions
    for (Module::iterator FI = M.begin(), FE = M.end();
         FI != FE;
         ++ FI){
      Function *F = dyn_cast<Function>(FI);
      if (F == NULL || F->empty()){
        continue;
      }

      UnifyExitNode.runOnFunction(*F);
      BasicBlock *ReturnBB = UnifyExitNode.getReturnBlock();

      // iterate all BBs
      for (Function::iterator BI = F->begin(), BE = F->end();
           BI != BE;
           ++ BI){
        BasicBlock *B = dyn_cast<BasicBlock>(BI);
        if (B == NULL){
          continue;
        }
        //////////////////////////////////
        // Propagate information in each BB
        //////////////////////////////////
        
        // if it's a terminating BB, propagate the info
        // from func live CAPTable to BB[out]
        if (ReturnBB == B){
          

        }

        // if it's a FunCall BB (find as key in BBFuncTable)
        // add the live info to 
        // func live CAPTable for callee processing
        if (BBFuncTable.find(BB) != BBFuncTable.end()){



        }

        // propagate live info to in[B]
        CAPArray_t &BBCAP;

        ischanged |= UnionCAPArrays(BBCAPTable_out[B], BBCAP);
        ischanged |= UnionCAPArrays(BBCAPTable_in[B], BBCAPTable_out[B]);

      } // iterate all BBs

    } // iterate all functions
  } // while change

  // find out the difference between in and out for each BB
  


  return false;
}



// register pass
char GlobalLiveAnalysis::ID = 0;
static RegisterPass<GlobalLiveAnalysis> L("GlobalLiveAnalysis", "Global privilege live analysis", true, true);

