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
using namespace llvm::splitBB;
using namespace llvm::globalLiveAnalysis;
using namespace llvm::dynCount;


// constructor
DynCount::DynCount() : ModulePass(ID) { };


// Preserve analysis usage
void DynCount::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.addRequired<GlobalLiveAnalysis>();
    AU.addRequired<SplitBB>();
    AU.addRequired<UnifyFunctionExitNodes>();
}


// get add count function
// param: M - module
// return: pointer to addcount function
Function* DynCount::getAddCountFunc(Module &M)
{
    std::vector<Type *> Params;
    Type* VoidType = Type::getVoidTy(getGlobalContext());
    Type *LOCType = IntegerType::get(getGlobalContext(), 32);
    Type *CAPArrayType = IntegerType::get(getGlobalContext(), 64);

    // First param for LOC
    Params.push_back(LOCType);

    // Second param for CAP set
    Params.push_back(CAPArrayType);

    FunctionType *AddCountFuncType = FunctionType::get(VoidType,
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
    Type* VoidType = Type::getVoidTy(getGlobalContext());

    // First param for LOC
    Params.push_back(VoidType);

    FunctionType *AddCountFuncType = FunctionType::get(VoidType,
                                     ArrayRef<Type *>(Params), false);
    Constant *InitCountFunc = M.getOrInsertFunction(INIT_COUNT_FUNC, 
                                                    AddCountFuncType);
    return dyn_cast<Function>(InitCountFunc);
}


// get report count function
// param: M - module
// return: pointer to initCount function
Function* DynCount::getReportCountFunc(Module &M)
{
    std::vector<Type *> Params;
    Type* VoidType = Type::getVoidTy(getGlobalContext());

    // First param for LOC
    Params.push_back(VoidType);

    FunctionType *AddCountFuncType = FunctionType::get(VoidType,
                                     ArrayRef<Type *>(Params), false);
    Constant *ReportCountFunc = M.getOrInsertFunction(REPORT_COUNT_FUNC,
                                                   AddCountFuncType);
    return dyn_cast<Function>(ReportCountFunc);
}


// Insert arguments to function type
// param: Args - the Args vector to insert into
//        CAPArray - the array of CAP to 
void DynCount::getAddCountArgs(std::vector<Value *>& Args, unsigned int LOC,
                               const CAPArray_t &CAPArray)
{
    uint64_t cap = 0;

    for (auto ci = CAPArray.begin(), ce = CAPArray.end();
         ci != ce; ++ci) {
        cap |= 1 << (*ci);
    }

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
    SplitBB &SB = getAnalysis<SplitBB>();
    GlobalLiveAnalysis &GA = getAnalysis<GlobalLiveAnalysis>();

    // Add function to module 
    Function *addCountFunction = getAddCountFunc(M);

    // iterate through all functions
    for (Module::iterator FI = M.begin(), FE = M.end(); FI != FE; ++FI) {
        Function *F = dyn_cast<Function>(FI);

        // iterate through all BBs to insert call instruction
        for (Function::iterator BI = F->begin(), BE = F->end(); BI != BE; ++BI) {
            BasicBlock *BB = dyn_cast<BasicBlock>(BI);

            // Insert callinst to all BBs 
            std::vector<Value *>Args = {};

            // Get rid of the final JMP instruction, as its CAP set may 
            // be different than rest of the BasicBlock
            unsigned long size = BB->size() - 1;

            // Insert addcount for all instructions in BB except terminator
            getAddCountArgs(Args, size, GA.BBCAPTable_in[BB]);
            CallInst::Create(addCountFunction, ArrayRef<Value *>(Args),
                             ADD_COUNT_FUNC, BB->getTerminator());

            // Insert addcount for terminator if it's not redundant jmp
            // created by splitBB
            if (findVector<BasicBlock *>(SB.ExtraJMPBB, BB)) {
                continue;
            }
            else {
                Args = {};
                getAddCountArgs(Args, 1, GA.BBCAPTable_out[BB]);
                CallInst::Create(addCountFunction, ArrayRef<Value *>(Args),
                                 ADD_COUNT_FUNC, BB->getTerminator());
            }
        }
    }

    // Insert init function call and report function call
    std::vector<Value *>Args = {};

    // Add initCount to entry block of main
    Function *mainFunc = M.getFunction("main");
    BasicBlock &entryBB = mainFunc->getEntryBlock();
    CallInst::Create(getInitCountFunc(M), ArrayRef<Value *>(Args),
                     INIT_COUNT_FUNC, entryBB.getFirstNonPHI());
    

    // Add reportCount to returnBB of main
    UnifyFunctionExitNodes &UnifyExitNode = getAnalysis<UnifyFunctionExitNodes>(*mainFunc);
    BasicBlock *returnBB = UnifyExitNode.getReturnBlock();
    CallInst::Create(getReportCountFunc(M), ArrayRef<Value *>(Args),
                     REPORT_COUNT_FUNC, returnBB->getTerminator());

    return false;
}


// find element in the element
// V - the vector to search in
// elem - the element to find
// return: true if found, false otherwise
template<typename T>
bool findVector(std::vector<T> V, T elem)
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
    SplitBB &SB = getAnalysis<SplitBB>();

    O << SB.ExtraJMPBB.size() << "\n";
}



// register pass
char DynCount::ID = 0;
static RegisterPass<DynCount> D("DynCount", "Dynamically count LOC in privilege set.", true, true);
