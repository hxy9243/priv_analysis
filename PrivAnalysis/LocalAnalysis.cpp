// ====--------------  LocalAnalysis.cpp ---------*- C++ -*---====
//
// Local analysis of priv_lower calls in Function blocks.
// Find all the priv_lower calls inside each of the functions.
//
// ====-------------------------------------------------------====

#include "LocalAnalysis.h"

#include <linux/capability.h>
#include <map>
#include <array>

using namespace llvm;
using namespace llvm::localAnalysis;

// Pass registry
char LocalAnalysis::ID = 0;
static RegisterPass<LocalAnalysis> A("LocalAnalysis", "Local Privilege Analysis", true, true);


// Constructor
LocalAnalysis::LocalAnalysis() : ModulePass(ID) {}


// Do initialization
// param: Module 
bool LocalAnalysis::doInitialization(Module &M){
  return false;
}


// RetrieveAllCAP
// Retrieve all capabilities from params of function call
// param: CI - call instruction to retrieve from
//        CAParray - the array of capability to save to
void LocalAnalysis::RetrieveAllCAP(CallInst *CI, CAPArray_t &CAParray){
  int numArgs = (int) CI->getNumArgOperands();

  // Note: Skip the first param of priv_lower for it's num of args
  for (int i = 1; i < numArgs; i ++){
    // retrieve integer value
    Value *v = CI->getArgOperand(i);
    ConstantInt *I = dyn_cast<ConstantInt>(v);
    unsigned int iarg = I->getZExtValue();

    // Add it to the array
    CAParray[iarg] = 1;
  }
}


// Get the function where the CallInst is in, add to map
// param: tf - the function to add 
//        CAParray - the array of capability to add to CAPTable
void LocalAnalysis::AddFuncToMap(Function *tf, CAPArray_t CAParray){

  // DEBUG
  errs() << "Parent of the instruction is " << tf->getName() << "\n";

  // If not found in map, add to map
  if (CAPTable.find(tf) == CAPTable.end() ){
    CAPTable[tf] = CAParray;
  }
  // else, Union the two arrays
  else {
    for (int i = 0; i < CAP_TOTALNUM; ++ i){
      CAPTable[tf][i] |= CAParray[i];
    }
  }
}


// Run on Module start
// param: Module
bool LocalAnalysis::runOnModule(Module &M){
  Function *F = M.getFunction(TARGET_FUNC);

  // Protector: didn't find any function TARGET_FUNC
  if (F == NULL){
    errs() << "Didn't find function " << TARGET_FUNC << "\n";
    return false;
  }

  // Find all user instructions of function in the module
  for (Value::user_iterator UI = F->user_begin(), UE = F->user_end();
       UI != UE;
       ++UI){
    // If it's a call Inst calling the targeted function
    CallInst *CI = dyn_cast<CallInst>(*UI);
    if (CI == NULL || CI->getCalledFunction() != F){
      continue;
    }

    // Retrieve all capabilities from params of function call
    std::array<bool, CAP_TOTALNUM>CAParray = {0};
    RetrieveAllCAP(CI, CAParray);

    // Get the function where the Instr is in
    // Add CAP to Map (Function* => array of CAPs)
    Function *tf = CI->getParent()->getParent();
    AddFuncToMap(tf, CAParray);

  }

  // DEBUG purpose: dump map table
  // ---------------------//
  dumpCAPTable (CAPTable);
  // ----------------------//

  return false;
} 



// getAnalysisUsage function
// preserve all analyses
// param: AU
void LocalAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}


// dump CAPTable for Debugging purpose
namespace llvm{
namespace localAnalysis{

  void dumpCAPTable(CAPTable_t &CT){

    // iterate through captable, a map from func to array
    for (auto mi = CT.begin(), me = CT.end(); 
         mi != me;
         ++ mi){
      errs() << mi->first->getName() << ":\n";

      // iterate through cap array
      for (auto ai = mi->second.begin(), ae = mi->second.end();
           ai != ae;
           ++ ai){
        errs() << *ai << "\t";
      }
      errs() << "\n";
    }
    errs() << "\n";

  }

} // namespace localanalysis
} // namespace llvm
