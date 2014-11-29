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

  errs() << "\nRunning Split BB Pass\n\n";

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

  // split on non-extern Function call sites
  for (Module::iterator FI = M.begin(), FE = M.end();
       FI != FE;
       ++ FI){
    Function *F = dyn_cast<Function>(FI);

    // skip priv_* calls
    if (F->getName() == PRIVRAISE ||
        F->getName() == PRIVLOWER){
      continue;
    }
    // skip external functions
    if (F->empty()){
      errs () << F->getName() << " is empty!\n";
      continue;
    }

    splitOnFunction(F, SPLIT_HERE | SPLIT_NEXT);
  }

  // DEBUG
  errs() << "Sizeof privBB is " << PrivBB.size() << "\n"
         << "Sizeof CallSiteBB is " << CallSiteBB.size() << "\n"
         << "Sizeof BBFuncTable is " << BBFuncTable.size() << "\n";

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
      

    // if split on the head of the calling instruction
    if (splitLoc & SPLIT_HERE){
      BasicBlock *BB = CI->getParent();

      if (dyn_cast<Instruction>(CI) !=
          dyn_cast<Instruction>(BB->begin())){

        // split on the instruction        
        BasicBlock *NewBB = BB->splitBasicBlock(CI);

        // save to data structure for later use
        if (F->getName() == PRIVRAISE){
          PrivBB.push_back(NewBB);
        }
        else{
          CallSiteBB.push_back(NewBB);
          BBFuncTable[NewBB] = F;
        }

        // DEBUG
        errs() << "split on " 
               << CI->getCalledFunction()->getName() << " in "
               << CI->getParent()->getParent()->getName() << "\n";
      }
      else{
        // else, push the original BB to data structure
        if (F->getName() == PRIVRAISE){
          PrivBB.push_back(BB);
        }
        else{
          CallSiteBB.push_back(BB);
          BBFuncTable[BB] = F;
        }

        // DEBUG
        errs() << CI->getCalledFunction()->getName()
               <<" is the start of a block in " 
               << CI->getParent()->getParent()->getName() 
               << " I'm not splitting you\n";
        ////////////////////////

      }
      
    }

    // if split on next of the calling instruction
    if (splitLoc & SPLIT_NEXT){
      BasicBlock *BB = CI->getParent();

      if (dyn_cast<Instruction>(CI) !=
          dyn_cast<Instruction>(BB->end())){
        //Instruction *splitPoint = CI->getNextNode();
        BB->splitBasicBlock(CI->getNextNode());
        errs() << "split on " 
               << CI->getCalledFunction()->getName() << " in "
               << CI->getParent()->getParent()->getName() << "\n";

      }
      else {
        errs() << CI->getCalledFunction()->getName()
               <<" is the start of a block in " 
               << CI->getParent()->getParent()->getName() 
               << " I'm not splitting you\n";

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


