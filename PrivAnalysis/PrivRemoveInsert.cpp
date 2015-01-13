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

    // Make function
    LLVMContext Context;
    Function *PrivRemoveCall = M.getFunction(PRIV_REMOVE_CALL);
    errs() << "Address of PrivRemove function is " << PrivRemoveCall << "\n";
    // Function *PrivRemoveCall = M.getOrInsertFunction(PRIV_REMOVE_CALL,
    //                                                  Type::getInt32Ty(Context),
    //                                                  Type::getInt32Ty(Context),
    //                                                  (Type *) NULL);

    // find all BBs with removable capabilities  
    for (auto BI = BBCAPTable_drop.begin(), BE = BBCAPTable_drop.end();
          BI != BE;
          ++BI) {
        BasicBlock *BB = BI->first;
        CAPArray_t &CAPArray = BI->second;
        std::vector<Value *> Args;
        
        // Add to the arg list
        int cap_num = 0;
        for (auto ci = CAPArray.begin(), ce = CAPArray.end();
             ci != ce;
             ++ci) {
            int cap = *ci;
            // skip invalide capabilities
            if (cap == 0) {
                continue;
            }
         
            // add to args vector
            cap_num++;
            Constant *arg = ConstantInt::get(IntegerType::get(getGlobalContext(), 32),
                                             cap);
            Args.push_back(arg);
        }

        // Add the number of args to the front
        ConstantInt *arg_num = ConstantInt::get(IntegerType::get(getGlobalContext(), 32),
                                                cap_num);
        Args.insert(Args.begin(), arg_num);

        // DEBUG
        errs() << "arg size " << Args.size() << "\n";

        errs() << "Adding remove call to BB in "
               << BB->getParent()->getName()
               << "\n";

        CallInst::Create(PrivRemoveCall, ArrayRef<Value *>(Args), PRIV_REMOVE_CALL, BB->getTerminator());
    }

    return true;
}


// register pass
char PrivRemoveInsert::ID = 0;
static RegisterPass<PrivRemoveInsert> I("PrivRemoveInsert", "Insert PrivRemove calls", true, true);

