// ====----------  PrivRemoveInsert.h ------------*- C++ -*---====
//
// Based on the information from GlobalLiveAnalysis, insert 
// privRemove calls at the end of BasicBlocks, to remove 
// unnecessary privileges. 
//
// ====-------------------------------------------------------====


#include <llvm/IR/Constant.h>
#include <llvm/IR/LLVMContext.h>

#include <map>

#include "ADT.h"
#include "PrivRemoveInsert.h"
#include "GlobalLiveAnalysis.h"


using namespace llvm;
using namespace llvm::globalLiveAnalysis;
using namespace llvm::privremoveinsert;


// PrivRemoveInsert constructor
PrivRemoveInsert::PrivRemoveInsert() : ModulePass(ID)
{

}


// Preserve analysis usage
void PrivRemoveInsert::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.addRequired<GlobalLiveAnalysis>();
}


// Initialization
bool PrivRemoveInsert::doInitialization(Module &M)
{

    return false;
}


// Run on Module
bool PrivRemoveInsert::runOnModule(Module &M)
{
    GlobalLiveAnalysis &GA = getAnalysis<GlobalLiveAnalysis>();

    BBCAPTable_t BBCAPTable_drop = GA.BBCAPTable_drop;
    Function *PrivRemoveCall = M.getFunction(PRIV_REMOVE_CALL);

    // find all BBs with removable capabilities  
    for (auto BI = BBCAPTable_drop.begin(), BE = BBCAPTable_drop.end();
          BI != BE;
          ++ BI) {
        BasicBlock *BB = BI->first;
        CAPArray_t &CAPArray = BI->second;
        std::vector<Value *> Args;
        
        // Add to the arg list
        int cap_num = 0;
        for (auto ci = CAPArray.begin(), ce = CAPArray.end();
             ci != ce;
             ++ci) {
            int cap = *ci;
            if (ci == 0) {
                continue;
            }
            
            cap_num++;
            Constant *arg = ConstantInt::get(IntegerType::get(getGlobalContext(), 32),
                                             cap);
            Args.push_back(arg);
        }

        // Add the number of args to the front
        ConstantInt *arg_num = ConstantInt::get(IntegerType::get(getGlobalContext(), 32),
                                                cap_num);
        Args.insert(Args.begin(), arg_num);

        CallInst::Create(PrivRemoveCall, ArrayRef<Value *>(Args), PRIV_REMOVE_CALL, BB);
    }

    return true;
}


// register pass
char PrivRemoveInsert::ID = 0;
static RegisterPass<PrivRemoveInsert> I("PrivRemoveInsert", "Insert PrivRemove calls", true, true);

