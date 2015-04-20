// ====------------  DSAExternTarget.h ----------*- C++ -*---====
//
// Find information about call sites from DSA analysis
//
// callsToExternNode will contain calls to calls external node
// in the LLVM callgraph, which is complete in DSA
//
// ====-------------------------------------------------------====


#include "DSAExternTarget.h"
#include "dsa/DataStructure.h"
#include "dsa/DSGraph.h"
#include "dsa/CallTargets.h"


using namespace llvm;
using namespace dsa;
using namespace privAnalysis;
using namespace localAnalysis;
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
    CallTargetFinder<TDDataStructures> &CTF = 
        getAnalysis<CallTargetFinder<TDDataStructures> >();

    for (std::list<CallSite>::iterator CSI = CTF.cs_begin(), CSE = CTF.cs_end();
         CSI != CSE; ++CSI) {
        CallSite &CS = *CSI;

        // If callsite is still incomplete, we don't know all callees of callsite,  skip it
        if (!CTF.isComplete(CS)) {
            continue;
        }
        
        // If a direct call, don't bother
        Function *CF = CS.getCalledFunction();
        if (CF) {
            continue;
        }
        
        if (dyn_cast<Function>(CS.getCalledValue()->stripPointerCasts())) {
            continue;
        }

        if (isa<ConstantPointerNull>(CS.getCalledValue()->stripPointerCasts())) {
            continue;
        }

        // Add it to the data structure
        // TODO:


    }
     

    return false;
}


// register pass
char DSAExternTarget::ID = 0;
static RegisterPass<DSAExternTarget> X("DSAExternTarget", "Find calls to callsExternNode.",
                                         true, /* CFG only? */
                                         true /* Analysis Pass? */);
