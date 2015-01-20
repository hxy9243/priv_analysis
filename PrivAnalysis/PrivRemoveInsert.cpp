// ====----------  PrivRemoveInsert.cpp ----------*- C++ -*---====
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
{ }


// Preserve analysis usage
void PrivRemoveInsert::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.addRequired<GlobalLiveAnalysis>();
}


// Initialization
// param M: the Module class
bool PrivRemoveInsert::doInitialization(Module &M)
{
    return false;
}


// Get PrivRemove function 
// param M: the Module class
Function *PrivRemoveInsert::getRemoveCall(Module &M)
{
    std::vector<Type *> Params;
    Type *TypeInt = IntegerType::get(getGlobalContext(), 32);

    Params.push_back(TypeInt);
    FunctionType *RemoveCallType = FunctionType::get(TypeInt,
                                                     ArrayRef<Type *>(Params),
                                                     true);
    Constant *PrivRemoveCall = M.getOrInsertFunction(PRIV_REMOVE_CALL,
                                                     RemoveCallType);
    
    return dyn_cast<Function>(PrivRemoveCall);
}


// Insert params to function type
// param: Args - the Args vector to insert into
//        CAPArray - the array of CAP to 
void PrivRemoveInsert::addToArgs(std::vector<Value *>& Args,
                                 const CAPArray_t &CAPArray)
{
    int cap_num = 0;
    int cap = 0;

    for (auto ci = CAPArray.begin(), ce = CAPArray.end();
         ci != ce; ++ci) {
        if (*ci == 0) {
            cap++;
            continue;
        }

        // add to args vector
        Constant *arg = ConstantInt::get
            (IntegerType::
             get(getGlobalContext(), 32), cap);
        Args.push_back(arg);

        cap++;
        cap_num++;
    }

    // Add the number of args to the front
    ConstantInt *arg_num = ConstantInt::get
        (IntegerType::get(getGlobalContext(), 32), 
         cap_num);
    Args.insert(Args.begin(), arg_num);

    return;
}


// Run on Module
bool PrivRemoveInsert::runOnModule(Module &M)
{
    GlobalLiveAnalysis &GA = getAnalysis<GlobalLiveAnalysis>();
    BBCAPTable_t BBCAPTable_drop = GA.BBCAPTable_drop;
    Function *PrivRemoveCall = getRemoveCall(M);

    // find all BBs with removable capabilities  
    for (auto BI = BBCAPTable_drop.begin(), BE = BBCAPTable_drop.end();
         BI != BE;
         ++BI) {
        BasicBlock *BB = BI->first;
        CAPArray_t &CAPArray = BI->second;
        std::vector<Value *> Args;

        addToArgs(Args, CAPArray);

        // DEBUG
        errs() << "arg size " << Args.size() << "\n";
        errs() << "Adding remove call to BB in "
               << BB->getParent()->getName()
               << "\n";

        // create call instruction
        assert(BB->getTerminator() != NULL && "BB has a NULL teminator!");
        CallInst::Create(PrivRemoveCall, ArrayRef<Value *>(Args), 
                         PRIV_REMOVE_CALL, BB->getTerminator());
    }

    return true;
}


// register pass
char PrivRemoveInsert::ID = 0;
static RegisterPass<PrivRemoveInsert> I("PrivRemoveInsert", "Insert PrivRemove calls", true, true);

