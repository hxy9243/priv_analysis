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
bool SplitBB::doInitialization(Module &M)
{
  return false;
}


// run on Basic Block
bool SplitBB::runOnModule(Module &M)
{
    // Split on PrivRaise calls
    Function *FRaise = M.getFunction(PRIVRAISE);
    if (FRaise != NULL) {
        splitOnFunction(FRaise, SPLIT_HERE);
    }

    // Split on PrivLower calls
    Function *FLower = M.getFunction(PRIVLOWER);
    if (FLower != NULL) {
        splitOnFunction(FLower, SPLIT_NEXT);
    }

    // Split on non-extern Function call sites
    for (Module::iterator FI = M.begin(), FE = M.end();
         FI != FE; ++ FI) {
        Function *F = dyn_cast<Function>(FI);

        // skip priv_* calls
        if (F->getName() == PRIVRAISE ||
            F->getName() == PRIVLOWER){
            continue;
        }

        // skip external functions
        // if (F->empty()){
        // continue;
        // }

        splitOnFunction(F, SPLIT_HERE | SPLIT_NEXT);
    }

    return true;
}


// split on all calling site of the Function
// param: F - The function to split
//        splitLoc - SPLIT_HERE split on the instruction
//                   SPLIT_NEXT split on the next instruction
//                   SPLIT_HERE | SPLIT_NEXT split both locations
void SplitBB::splitOnFunction(Function *F, int splitLoc)
{
    // Iterate all uses for calling instruction
    for (Value::user_iterator UI = F->user_begin(), UE = F->user_end();
         UI != UE; ++ UI) {
        CallInst *CI = dyn_cast<CallInst>(*UI);
        if (CI == NULL || CI->getCalledFunction() != F) {
            continue;
        }
      
        // If split on the head of the calling instruction
        if (splitLoc & SPLIT_HERE){
            BasicBlock *BB = CI->getParent();

            if (dyn_cast<Instruction>(CI) !=
                dyn_cast<Instruction>(BB->begin())) {

                // Split on the instruction
                // Old BB now has an extra jmp as terminator,
                // save Old BB for later counting
                BasicBlock *NewBB = BB->splitBasicBlock(CI);

                ExtraJMPBB.push_back(BB);

                // Save to data structure for later use
                // If instruction is priv_raise, save to PrivBB
                // Else if a function call, save to CallSiteBB and BBFuncTable
                if (F->getName() == PRIVRAISE) {
                    PrivBB.push_back(NewBB);
                }
                else {
                    CallSiteBB.push_back(NewBB);
                    BBFuncTable[NewBB] = F;
                }
            }
            else {
                // else, push the original BB to data structure
                if (F->getName() == PRIVRAISE) {
                    PrivBB.push_back(BB);
                }
                else {
                    CallSiteBB.push_back(BB);
                    BBFuncTable[BB] = F;
                }
            }
        }

        // If split on next of the calling instruction
        if (splitLoc & SPLIT_NEXT) {
            BasicBlock *BB = CI->getParent();

            if (dyn_cast<Instruction>(CI) !=
                dyn_cast<Instruction>(BB->end())) {

                BB->splitBasicBlock(CI->getNextNode());

                ExtraJMPBB.push_back(BB);
            }
        }

    } // iterate all uses for calling instructions
} // split on function


void SplitBB::print(raw_ostream &O, const Module *M) const
{
    errs() << "Extra Jump BB size: " << ExtraJMPBB.size() << "\n";

    errs() << "Priv BB size: " << PrivBB.size() << "\n";

    errs() << "CallSite BB size: " << CallSiteBB.size() << "\n";
}




// getAnalysisUsage function
// param: AU
void SplitBB::getAnalysisUsage(AnalysisUsage &AU) const { }


// Pass registry
char SplitBB::ID = 0;
static RegisterPass<SplitBB> B("SplitBB", "Split BasicBlock pass", 
                               false, /* Modifies the CFG */
                               false /* Modify the program */);
