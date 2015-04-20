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


// Get pass analysis usage
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


// Run on Module method for pass
bool DSAExternTarget::runOnModule(Module &M)
{
    CallTargetFinder<TDDataStructures> &CTF = 
        getAnalysis<CallTargetFinder<TDDataStructures> >();

    callsToExternNode = {};

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
        // FunctionMap[CF] = {};
        for (std::vector<const Function*>::iterator FI = CTF.begin(CS), FE = CTF.end(CS);
             FI != FE; ++FI) {
            callsToExternNode[CF].push_back(*FI);
        }
    }

    return false;
}


// print out information for debugging purposes
void DSAExternTarget::print(raw_ostream &O, const Module *M) const
{
    for (FunctionMap::const_iterator FMI = callsToExternNode.begin(),
             FME = callsToExternNode.end(); FMI != FME; ++FMI) {
        Function *Caller = FMI->first;

        if (Caller != NULL) {
            O << Caller->getName() << ":\t";
        }
        else {
            continue;
        }

        // Iterate through all callees of the function
        std::vector<const Function*> callees = FMI->second;
        for (std::vector<const Function*>::iterator FI = callees.begin(),
                 FE = callees.end(); FI != FE; ++FI) {

            assert((*FI) && "Caller is NULL function\n");
            O << (*FI)->getName() << ", ";
        }
        O << "\n";
    }
}



// register pass
char DSAExternTarget::ID = 0;
static RegisterPass<DSAExternTarget> X("DSAExternTarget", "Find calls to callsExternNode.",
                                         true, /* CFG only? */
                                         true /* Analysis Pass? */);
