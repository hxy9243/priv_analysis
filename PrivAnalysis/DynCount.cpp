// ====------------------- DynCount.cpp ---------*- C++ -*----====
//
// Dynamically count the lines of IR for each privilege set
//
// ====-------------------------------------------------------====

#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
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
}


// get add count function
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
    //    SplitBB &SB = getAnalysis<SplitBB>();
    GlobalLiveAnalysis &GA = getAnalysis<GlobalLiveAnalysis>();

    // Add function to module 
    Function *addCountFunction = getAddCountFunc(M);

    // iterate through all functions
    for (Module::iterator FI = M.begin(), FE = M.end(); FI != FE; ++FI) {
        Function *F = dyn_cast<Function>(FI);

        // iterate through all BBs to insert function
        for (Function::iterator BI = F->begin(), BE = F->end(); BI != BE; ++BI) {
            BasicBlock *BB = dyn_cast<BasicBlock>(BI);

            // Insert callinst to all BBs 
            std::vector<Value *>Args = {};

            CAPArray_t CAPArray = GA.BBCAPTable_in[BB];

            getAddCountArgs(Args, (unsigned int)BB->size(), CAPArray);

            CallInst::Create(addCountFunction, ArrayRef<Value *>(Args),
                             ADD_COUNT_FUNC, BB->getTerminator());
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
