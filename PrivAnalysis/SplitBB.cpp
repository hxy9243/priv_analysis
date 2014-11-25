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

  // iterate through all instructions
  for(BasicBlock::iterator II = B.begin(), IE = B.end();
      II != IE;
      ++ II){

    // We are only interested in CallInst
    CallInst *CI = dyn_cast<CallInst>(II);
    if (CI == NULL){
      continue;
    }

    // if Instruction is priv_raise
    // split the Instruction between priv_raise 
    // and priv_lower as new separate BB
    Function *CalledFunc = CI->getCalledFunction();
    errs() << CalledFunc->getName() << "\n";

    if (CalledFunc->getName() == PRIVRAISE){

      errs() << CalledFunc->getName() << "\n";

      // split the basic block
      // TODO: Is this right?
      // SplitBlock(&B, CI, this);
    }
  }

  // It modifies CFG
  return true;
}


// run on Basic Block
bool SplitBB::isExternLibcall(Instruction &I){



  return true;
}



// getAnalysisUsage function
// param: AU
void SplitBB::getAnalysisUsage(AnalysisUsage &AU) const {

}


// Pass registry
char SplitBB::ID = 0;
static RegisterPass<SplitBB> B("SplitBB", "Split BasicBlock pass", true, true);


