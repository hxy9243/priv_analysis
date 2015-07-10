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
Function *PrivRemoveInsert::getRemoveFunc(Module &M)
{
    std::vector<Type *> Params;
    Type *IntType = IntegerType::get(getGlobalContext(), 32);
    Params.push_back(IntType);
    FunctionType *RemoveCallType = FunctionType::get(IntType,
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

    for (cap = 0; cap < CAP_TOTALNUM; ++cap) {
        if ((CAPArray & (1 << cap)) == 0) {
            continue;
        }

        // add to args vector
        Constant *arg = ConstantInt::get
            (IntegerType::get(getGlobalContext(), 32), cap);
        Args.push_back(arg);

        cap_num++;
    }

    // Add the number of args to the front
    ConstantInt *arg_num = ConstantInt::get
        (IntegerType::get(getGlobalContext(), 32), cap_num);
    Args.insert(Args.begin(), arg_num);

    return;
}


// Run on Module
bool PrivRemoveInsert::runOnModule(Module &M)
{
    GlobalLiveAnalysis &GA = getAnalysis<GlobalLiveAnalysis>();
    BBCAPTable_t BBCAPTable_dropEnd = GA.BBCAPTable_dropEnd;
    BBCAPTable_t BBCAPTable_dropStart = GA.BBCAPTable_dropStart;

    FuncCAPTable_t FuncLiveCAPTable_in = GA.FuncLiveCAPTable_in;

    // Insert remove call at the top of the main function
    Function *PrivRemoveFunc = getRemoveFunc(M);
    std::vector<Value *> Args = {};
    Function *mainFunc = M.getFunction("main");
    CAPArray_t &FirstCAPArray = FuncLiveCAPTable_in[mainFunc];

    // Find all CAPs that's not alive - reverse of FuncLiveIn
    ReverseCAPArray(FirstCAPArray);
    addToArgs(Args, FirstCAPArray);

    Instruction *firstInst = dyn_cast<Instruction>
        (mainFunc->begin()->begin());
    CallInst::Create(PrivRemoveFunc, ArrayRef<Value *>(Args), 
                     PRIV_REMOVE_CALL, firstInst);

    // Insert call to all BBs with removable capabilities  
    for (auto BI = BBCAPTable_dropEnd.begin(), BE = BBCAPTable_dropEnd.end();
         BI != BE; ++BI) {
        BasicBlock *BB = BI->first;
        CAPArray_t &CAPArray = BI->second;
        Args.clear();

        addToArgs(Args, CAPArray);

        // create call instruction
        assert(BB->getTerminator() != NULL && "BB has a NULL teminator!");
        CallInst::Create(PrivRemoveFunc, ArrayRef<Value *>(Args), 
                         PRIV_REMOVE_CALL, BB->getTerminator());
    }


    // Insert at the start of the dropStart
    for (auto BI = BBCAPTable_dropStart.begin(), BE = BBCAPTable_dropStart.end();
         BI != BE; ++BI) {
        BasicBlock *BB = BI->first;
        CAPArray_t &CAPArray = BI->second;
        Args.clear();

        addToArgs(Args, CAPArray);

        // create call instruction
        CallInst::Create(PrivRemoveFunc, ArrayRef<Value *>(Args), 
                         PRIV_REMOVE_CALL, BB->getFirstNonPHI());
    }

    return true;
}


// Print out information for debugging purposes
void PrivRemoveInsert::print(raw_ostream &O, const Module *M) const
{
}



// register pass
char PrivRemoveInsert::ID = 0;
static RegisterPass<PrivRemoveInsert> I("PrivRemoveInsert", "Insert PrivRemove calls", 
                                        true, /* CFG only? */
                                        false /* Analysis pass? */);

