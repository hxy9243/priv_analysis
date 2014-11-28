// ====------------- GlobalLiveAnalysis.cpp ------*- C++ -*---====
//
// Globally Privilge Live Analysis for each basic
// blocks. The output would be the privileges to drop for each
// basic blocks
//
// ====-------------------------------------------------------====

#include "llvm/Analysis/CallGraph.h"
#include "llvm/Pass.h"

#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
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
  AU.setPreservesAll();

  AU.addRequired<UnifyFunctionExitNodes>();
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

  int i = 0;

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

      // Find the exit node of the Function
      UnifyFunctionExitNodes &UnifyExitNode = getAnalysis<UnifyFunctionExitNodes>(*F);
      UnifyExitNode.runOnFunction(*F);
      BasicBlock *ReturnBB = UnifyExitNode.getReturnBlock();

      // iterate all BBs
      for (Function::iterator BI = F->begin(), BE = F->end();
           BI != BE;
           ++ BI){

        ++i;

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
          ischanged |= UnionCAPArrays(BBCAPTable_out[B], FuncLiveCAPTable[F]);
        }

        // if it's a FunCall BB (find as key in BBFuncTable)
        // add the live info to
        // func live CAPTable for callee processing
        if (BBFuncTable.find(B) != BBFuncTable.end()){
          ischanged |= UnionCAPArrays(BBCAPTable_in[B],
                                      FuncUseCAPTable[BBFuncTable[B]]);

          ischanged |= UnionCAPArrays(FuncLiveCAPTable[F],
                                      BBCAPTable_out[B]);

        }

        // if it's a Priv Call BB
        if (BBCAPTable.find(B) != BBCAPTable.end()){
          ischanged |= UnionCAPArrays(BBCAPTable_in[B], BBCAPTable[B]);
        }

        // propagate live info to in[B]
        ischanged |= UnionCAPArrays(BBCAPTable_in[B], BBCAPTable_out[B]);

      } // iterate all BBs

    } // iterate all functions
  } // while change

  // find out the difference between in and out for each BB
  errs() << "converged with " << i << " iterations\n";

  errs() << "BBCAPTable_in size " << BBCAPTable_in.size() << "\n";

  // Find Difference of BB in and out CAPArrays
  // Save it to the output 
  for (auto bi = BBCAPTable_out.begin(); bi != BBCAPTable_out.end(); ++bi){

    BasicBlock *B = bi->first;
    CAPArray_t &CAPArray_out = bi->second;
    CAPArray_t &CAPArray_in = BBCAPTable_in[B];
    
    DiffCAPArrays(BBCAPTable_drop[B], CAPArray_in, CAPArray_out);
  }


  // DEBUG
  errs() << "BBCAPTable_in size " << BBCAPTable_drop.size() << "\n";

  int count = 0;
  for (auto bi = BBCAPTable_drop.begin(); bi != BBCAPTable_drop.end(); ++bi){
    BasicBlock *B = bi->first;
    CAPArray_t &CAPArray_drop = bi->second;
    
    ++count;
    
    errs() << "BB" << count
           << " in " << B->getParent()->getName() << ":  \t";

    for (unsigned int i = 0; i < CAPArray_drop.size(); ++i){
      if(CAPArray_drop[i]){
        errs() << i << "\t";
      }
    }
    errs() << "\n";
  }
  errs() << "\n";

  return false;
}



// register pass
char GlobalLiveAnalysis::ID = 1;
static RegisterPass<GlobalLiveAnalysis> L("GlobalLiveAnalysis", "Global privilege live analysis", true, true);
