// ====--------------  LocalAnalysis.cpp ---------*- C++ -*---====
//
// Local analysis of priv_lower calls in Function blocks.
// Find all the priv_lower calls inside each of the functions.
//
// ====-------------------------------------------------------====


#include "ExternFunCall.h"


using namespace llvm;
using namespace externfuncall;

// Constructor
ExternFunCall::ExternFunCall() : ModulePass(ID) {}


// Do initialization
// param: Module
bool ExternFunCall::doInitialization(Module &M)
{
    return false;
}


// Run on Module start
bool ExternFunCall::runOnModule(Module &M)
{
    // Iterate through all functions
    for (Module::iterator FI = M.begin(), FE = M.end();
         FI != FE; ++FI) {
        Function *F = dyn_cast<Function>(FI);
        if (F == NULL) {
            continue;
        }

        if (F->begin() == F->end()) {
            errs() << F->getName() << "\n";
        }

    }

    return false;
}


// Get Analysis Usage from other passes
void ExternFunCall::getAnalysisUsage(AnalysisUsage &AU) const 
{

}


// Pass registry
char ExternFunCall::ID = 0;
static RegisterPass<ExternFunCall> X("ExternFunCall", "Find all external function calls in program.", true, true);

