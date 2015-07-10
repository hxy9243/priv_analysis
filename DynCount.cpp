// ====------------------- DynCount.cpp ---------*- C++ -*----====
//
// Dynamically count the lines of IR for each privilege set
//
// ====-------------------------------------------------------====

#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"

#include "DynCount.h"


using namespace llvm;
using namespace llvm::localAnalysis;
using namespace llvm::propagateAnalysis;
using namespace llvm::globalLiveAnalysis;
using namespace llvm::dynCount;


// constructor
DynCount::DynCount() : ModulePass(ID) { };


// Preserve analysis usage
void DynCount::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.addRequired<LocalAnalysis>();
    AU.addRequired<PropagateAnalysis>();
    AU.addRequired<GlobalLiveAnalysis>();
}


// get add count function
// param: M - module
// return: pointer to addcount function
Function* DynCount::getAddCountFunc(Module &M)
{
    std::vector<Type *> Params;
    Type *IntType = IntegerType::get(M.getContext(), 32);
    Type *Int64Type = IntegerType::get(M.getContext(), 64);

    // First param for LOC
    Params.push_back(IntType);

    // Second param for CAP set
    Params.push_back(Int64Type);

    FunctionType *AddCountFuncType = FunctionType::get(IntType,
                                                       ArrayRef<Type *>(Params), false);
    Constant *AddCountFunc = M.getOrInsertFunction(ADD_COUNT_FUNC,
                                                   AddCountFuncType);

    return dyn_cast<Function>(AddCountFunc);
}


// get init count function
// param: M - module
// return: pointer to initCount function
Function* DynCount::getInitCountFunc(Module &M)
{
    std::vector<Type *> Params;
    Type *IntType = IntegerType::get(M.getContext(), 32);

    FunctionType *InitCountFuncType = FunctionType::get(IntType,
                                                        ArrayRef<Type *>(Params), false);
    Constant *InitCountFunc = M.getOrInsertFunction(INIT_COUNT_FUNC, 
                                                    InitCountFuncType);
    return dyn_cast<Function>(InitCountFunc);
}


// get report count function
// param: M - module
// return: pointer to initCount function
Function* DynCount::getAtExitFunc(Module &M)
{
    std::vector<Type *> Params;
    Type *IntType = IntegerType::get(M.getContext(), 32);
    Type *VoidType = Type::getVoidTy(M.getContext());

    // insert function void atexit(void (*)(void))
    // get void function type
    FunctionType *VoidFuncType = FunctionType::get(VoidType,
                                                   ArrayRef<Type *>(Params), false);
    PointerType *VoidFuncPointer = VoidFuncType->getPointerTo();

    // get atexit function type
    Params.clear();
    Params.push_back(VoidFuncPointer);
    FunctionType *AtExitFuncType = FunctionType::get(IntType,
                                                     ArrayRef<Type *>(Params), false);

    Constant *AtExitFunc = M.getOrInsertFunction("atexit", AtExitFuncType);

    assert(AtExitFunc != NULL && "AtExit Function is NULL!");

    return dyn_cast<Function>(AtExitFunc);
}


// Insert arguments to function type
// param: Args - the Args vector to insert into
//        CAPArray - the array of CAP to 
void DynCount::getAddCountArgs(std::vector<Value *>& Args, unsigned int LOC,
                               const CAPArray_t &CAPArray)
{
    uint64_t cap = CAPArray;

    // add to args vector
    Constant *LOCArg = ConstantInt::get
        (IntegerType::get(getGlobalContext(), 32), LOC);
    Args.push_back(LOCArg);

    Constant *CAPArrayArg = ConstantInt::get
        (IntegerType::get(getGlobalContext(), 64), cap);
    Args.push_back(CAPArrayArg);

    return;
}


// Initialization
bool DynCount::doInitialization(Module &M)
{
    return true;
}


// run on module
bool DynCount::runOnModule(Module &M)
{
    LocalAnalysis &LA = getAnalysis<LocalAnalysis>();
    PropagateAnalysis &PA = getAnalysis<PropagateAnalysis>();
    GlobalLiveAnalysis &GA = getAnalysis<GlobalLiveAnalysis>();

    // Add function to module 
    Function *addCountFunction = getAddCountFunc(M);
    // Insert callinst to all BBs 
    std::vector<Value *>Args;
    FuncCAPTable_t FuncCAPTable = PA.FuncCAPTable;

    assert(addCountFunction && "The addCount function is NULL!\n");

    // iterate through all functions
    // for (Module::iterator FI = M.begin(), FE = M.end(); FI != FE; ++FI) {
    for (auto FI = FuncCAPTable.begin(), FE = FuncCAPTable.end(); FI != FE; ++FI) {
        Function *F = FI->first;
        if (F == NULL) {
            continue;
        }

        // iterate through all BBs to insert call instruction
        for (Function::iterator BI = F->begin(), BE = F->end(); BI != BE; ++BI) {
            BasicBlock *BB = dyn_cast<BasicBlock>(BI);
            if (BB == NULL) {
                continue;
            }

            Args.clear();

            // Get rid of the final JMP instruction, as its CAP set may 
            // be different than rest of the BasicBlock
            unsigned long size = BB->size() - 1;

            // Insert addcount for all instructions in BB except terminator
            getAddCountArgs(Args, size, GA.BBCAPTable_in[BB]);
            CallInst::Create(addCountFunction, ArrayRef<Value *>(Args),
                             ADD_COUNT_FUNC, BB->getTerminator());

            // Insert addcount for terminator if it's not redundant jmp
            // created by splitBB
            if (findVector<BasicBlock*>(LA.ExtraJMPBB, BB)) {
                continue;
            }
            else {
                Args.clear();
                getAddCountArgs(Args, 1, GA.BBCAPTable_out[BB]);
                CallInst::Create(addCountFunction, ArrayRef<Value *>(Args),
                                 ADD_COUNT_FUNC, BB->getTerminator());
            }
        }
    }

    // Insert init function call and report function call
    Args.clear();

    // Add initCount to entry block of main
    Function *mainFunc = M.getFunction("main");
    BasicBlock &entryBB = mainFunc->getEntryBlock();
    CallInst::Create(getInitCountFunc(M), ArrayRef<Value *>(Args),
                     INIT_COUNT_FUNC, entryBB.getFirstNonPHI());
    

    // Add atexit() call to the entry block of main
    Type *VoidType = Type::getVoidTy(M.getContext());

    FunctionType *VoidFuncType = FunctionType::get(VoidType, false);
    Constant *reportCountFuncConstant = M.getOrInsertFunction("reportCount",
                                                              VoidFuncType);
    assert(reportCountFuncConstant && "report count constant is NULL");

    // TODO: why this cause (reportCountFunc == NULL)? 
    // Function *reportCountFunc = dyn_cast<Function>(reportCountFuncConstant);
    // assert(reportCountFunc && "report count function is NULL");

    Args.push_back(reportCountFuncConstant);
    CallInst::Create(getAtExitFunc(M), Args, "", entryBB.getFirstNonPHI());

    return false;
}


// find element in the element
// V - the vector to search in
// elem - the element to find
// return: true if found, false otherwise
template<typename T>
bool DynCount::findVector(std::vector<T> V, T elem)
{
    for (auto I = V.begin(), E = V.end(); I != E; ++I) {
        if (elem == (*I)) {
            return true;
        }
    }
    return false;
}



void DynCount::print(raw_ostream &O, const Module *M) const
{
    LocalAnalysis &LA = getAnalysis<LocalAnalysis>();

    O << LA.ExtraJMPBB.size() << "\n";
}



// register pass
char DynCount::ID = 0;
static RegisterPass<DynCount> D("DynCount", "Dynamically count LOC in privilege set.", 
                                true, /* CFG only? */
                                false  /* If modifies the program? */);
