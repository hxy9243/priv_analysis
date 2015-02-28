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
    Type *IntType = IntegerType::get(getGlobalContext(), 32);
    Params.push_back(IntType);

    FunctionType *AddCountFuncType = FunctionType::get(VoidType,
                                                       ArrayRef<Type *>(Params), false);
    Constant *AddCountFunc = M.getOrInsertFunction(ADD_COUNT_FUNC,
                                                   AddCountFuncType);

    return dyn_cast<Function>(AddCountFunc);
}


// Insert arguments to function type
// param: Args - the Args vector to insert into
//        CAPArray - the array of CAP to 
void DynCount::getAddCountArgs(std::vector<Value *>& Args,
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
            (IntegerType::get(getGlobalContext(), 32), cap);
        Args.push_back(arg);

        cap++;
        cap_num++;
    }

    // Add the number of args to the front
    ConstantInt *arg_num = ConstantInt::get
        (IntegerType::get(getGlobalContext(), 32), cap_num);
    Args.insert(Args.begin(), arg_num);

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
    //    GlobalLiveAnalysis &GA = getAnalysis<GlobalLiveAnalysis>();

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
