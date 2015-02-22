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
using namespace llvm::dynCount;

// constructor
DynCount::DynCount() : ModulePass(ID) { };





// run on module
bool DynCount::runOnModule(Module &M)
{
    // StructType::create(M.getContext(), "CAPArray_t");

    

    return false;
}



// register pass
char DynCount::ID = 1;
static RegisterPass<DynCount> D("DynCount", "Dynamically count LOC in privilege set.", true, true);
