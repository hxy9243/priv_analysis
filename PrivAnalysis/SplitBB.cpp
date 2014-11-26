// ====-------------------  SplitBB.h ------------*- C++ -*---====
//
// A BasicBlock Pass to split all priv_* instructions and related 
// function calls to single BBs for next step of analysis.
//
// ====-------------------------------------------------------====

#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "ADT.h"
#include "SplitBB.h"

#include <linux/capability.h>
#include <map>
#include <array>

using namespace llvm;
using namespace llvm::splitBB;


// Constructor
SplitBB::SplitBB() : BasicBlockPass(ID) {}


// do Initialization
bool SplitBB::doInitialization(Function &F){
  return false;
}


// run on Basic Block
bool SplitBB::runOnBasicBlock(BasicBlock &B){

  // Store the split location in BasicBlocks
  std::map<BasicBlock *, std::vector<Instruction *> >splitLocationInBB;

  // iterate through all instructions to find out split locations
  for(BasicBlock::iterator II = B.begin(), IE = B.end();
      II != IE;
      ++ II){

    // We are only interested in CallInst
    CallInst *CI = dyn_cast<CallInst>(II);
    if (CI == NULL){
      continue;
    }

    // Split the BasicBlock according to the instruction type
    // possible chances are:
    //    priv_raise, priv_lower calls
    //    non-external function calls
    Function *CalledFunc = CI->getCalledFunction();
    // privraise
    if (CalledFunc->getName() == PRIVRAISE){
      std::vector<Instruction *> &InstVector = splitLocationInBB[&B];
      if (II != B.begin()){
        // add Inst to split location
        InstVector.push_back(dyn_cast<Instruction>(II));
      }
    }

    // privlower
    else if (CalledFunc->getName() == PRIVLOWER){
      std::vector<Instruction *> &InstVector = splitLocationInBB[&B];
      if (II != B.end()){
        // add Inst to split location
        InstVector.push_back(dyn_cast<Instruction>(++ II));
      }
    }

    // other non-external library function calls
    // -- functions not defined in other libraries
    else if (!isExternLibCall(CalledFunc)){
      std::vector<Instruction *> &InstVector = splitLocationInBB[&B];
      if (II != B.begin()){
        // add Inst to split location
        InstVector.push_back(dyn_cast<Instruction>(II));
      }
      if (II != B.end()){
        // add Inst to split location
        InstVector.push_back(dyn_cast<Instruction>(++ II));
      }
    }
  }
  
  // It modifies CFG
  return true;
}


// run on Basic Block
bool SplitBB::isExternLibCall(Function *F){



  return true;
}


// getAnalysisUsage function
// param: AU
void SplitBB::getAnalysisUsage(AnalysisUsage &AU) const {

}


// Pass registry
char SplitBB::ID = 0;
static RegisterPass<SplitBB> B("SplitBB", "Split BasicBlock pass", 
                               false /* Modifies the CFG */,
                               false /* Modify the program */);


