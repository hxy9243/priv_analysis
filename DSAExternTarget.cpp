// ====------------  DSAExternTarget.h ----------*- C++ -*---====
//
// Find information about call sites from DSA analysis
//
// callsToExternNode will contain calls to calls external node
// in the LLVM callgraph, which is complete in DSA
//
// ====-------------------------------------------------------====


#include "DSAExternTarget.h"



using namespace llvm;
using namespace dsa;
using namespace privAnalysis;
using namespace localanalysis;
using namespace dsaexterntarget;


DSAExternTarget::DSAExternTarget() : ModulePass(ID) { } 


void DSAExternTarget::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.setPreservesCFG();

    AU.addRequired<LocalAnalysis>();
    AU.addRequired<CallTargetFinder<TDDataStructures> >();    

    AU.setPreservesAll();
}


// Do initialization
bool DSAExternTarget::doInitialization(Module &M)
{
    return false;
}


bool DSAExternTarget::runOnModule(Module &M)
{







    return false;
}


// register pass
char DSAExternTarget::ID = 0;
static RegisterPass<DSAExternTarget> X("DSAExternTarget", "Find calls to callsExternNode.",
                                         true, /* CFG only? */
                                         true /* Analysis Pass? */);
