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
SplitBB::SplitBB() : ModulePass(ID) {}


// do Initialization
bool SplitBB::doInitialization(Module &M){
  return false;
}


// run on Basic Block
bool SplitBB::runOnModule(Module &M){
  // split on PrivRaise calls
  Function *FRaise = M.getFunction(PRIVRAISE);
  if (FRaise != NULL){
    splitOnFunction(FRaise, SPLIT_HERE);
  }

  // split on PrivLower calls
  Function *FLower = M.getFunction(PRIVLOWER);
  if (FLower != NULL){
    splitOnFunction(FLower, SPLIT_NEXT);
  }

  


  // It modifies CFG
  return true;
}



// split on all calling site of the Function
// param: F - The function to split
//        splitLoc - SPLIT_HERE split on the instruction
//                   SPLIT_NEXT split on the next instruction
//                   SPLIT_HERE | SPLIT_NEXT split both locations
void SplitBB::splitOnFunction(Function *F, int splitLoc){

  // iterate all uses for calling instruction
  for (Value::user_iterator UI = F->user_begin(), UE = F->user_end();
       UI != UE;
       ++ UI){
    CallInst *CI = dyn_cast<CallInst>(*UI);
    if (CI == NULL || CI->getCalledFunction() != F){
      continue;
    }
      
    // split on the instruction
    BasicBlock *BB = CI->getParent();

    // if split on the head of the calling instruction
    if (splitLoc & SPLIT_HERE){
      if (dyn_cast<Instruction>(CI) !=
          dyn_cast<Instruction>(BB->begin())){
        BB->splitBasicBlock(CI);

        // DEBUG
        errs() << "split on " << CI->getParent()->getParent()->getName() << "\n";
      }
      // DEBUG
      else{
        errs() << "you're the start of " << CI->getParent()->getParent()->getName() << " I'm not splitting you\n";
      }
    }

    // if split on next of the calling instruction
    if (splitLoc & SPLIT_NEXT){
      if (dyn_cast<Instruction>(CI) !=
          dyn_cast<Instruction>(BB->end())){
        //Instruction *splitPoint = CI->getNextNode();
        BB->splitBasicBlock(CI->getNextNode());
        errs() << "split on " << CI->getParent()->getParent()->getName() << "\n";
      }
      else {
        errs() << "you're the end of " << CI->getParent()->getParent()->getName() << " I'm not splitting you\n";
      }
    }
  }
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


