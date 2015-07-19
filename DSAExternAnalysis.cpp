// ====------------  DSAExternAnalysis.h ----------*- C++ -*---====
//
// Find information about call sites from DSA analysis
//
// callsToExternNode: mapping from all callsites to externNode
// callgraphMap:      mapping from caller to callees. All callers
//                    here have calls to externNodes but are
//                    complete in DSA analysis
// instFunMap:        mapping from callsite instruction to 
//                    callees. Saved for CFG in Global Live
//                    Analysis. 
// ====------------------------------------------------------====


#include "DSAExternAnalysis.h"
#include "dsa/DataStructure.h"
#include "dsa/DSGraph.h"
#include "dsa/CallTargets.h"


using namespace llvm;
using namespace dsa;
using namespace privAnalysis;
using namespace localAnalysis;
using namespace dsaexterntarget;


DSAExternAnalysis::DSAExternAnalysis() : ModulePass(ID) { } 


// Get pass analysis usage
void DSAExternAnalysis::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.setPreservesCFG();

    // AU.addRequired<LocalAnalysis>();
    AU.addRequired<CallTargetFinder<TDDataStructures> >();    

    AU.setPreservesAll();
}


// Do initialization
bool DSAExternAnalysis::doInitialization(Module &M)
{
    return false;
}


// Find out all indirect callsites, save to callsToExternNode
void DSAExternAnalysis::findAllCallSites(CallTargetFinder<TDDataStructures> &CTF)
{
    callsToExternNode = {};

    // Iterate through the CallTargetFinder for all callsites
    for (std::list<CallSite>::iterator CSI = CTF.cs_begin(), CSE = CTF.cs_end();
         CSI != CSE; ++CSI) {
        CallSite &CS = *CSI;

        // If a direct call, don't bother
        Function *CF = CS.getCalledFunction();
        if (CF) { continue; }

        // skip strip pointer casts
        if (dyn_cast<Function>(CS.getCalledValue()->stripPointerCasts())) {
            // errs() << "strip Pointer casts\n";
            continue;
        }

        // skip NULL pointer casts
        if (isa<ConstantPointerNull>(CS.getCalledValue()->stripPointerCasts())) {
            continue;
        }

        // Add it to the data structure callsToExternNode
        // TODO: here it presumes that anything besides direct call is a call to 
        // TODO: CallsExternNode
        for (std::vector<const Function*>::iterator FI = CTF.begin(CS), 
                 FE = CTF.end(CS);
             FI != FE; ++FI) {
            Function* F = const_cast<Function*>(*FI);
            callsToExternNode[&CS].push_back(F);
        }
    }
}


// Save information to corresponding data structures
void DSAExternAnalysis::saveToMappings(CallTargetFinder<TDDataStructures> &CTF)
{
    std::vector<Function*>incompleteFuns = {};
    callgraphMap = {};
    instFunMap = {};

    // Based on callsites in callsExternNode, save information to corresponding
    // data structures
    // Save complete calls to callgraphMap and InstrFunMap
    // Save incomplete calls to incompleteFuns, so as to remove them 
    // from callgraphMap later
    for (CallSiteFunMap_t::iterator FI = callsToExternNode.begin(), 
             FE = callsToExternNode.end();
         FI != FE; ++FI) {
        CallSite* CS = FI->first;
        CallInst* callInst = dyn_cast<CallInst>(FI->first->getInstruction());
        Function* caller = callInst->getParent()->getParent();
        std::vector<Function*>callees = FI->second;

        // if function callsite is incomplete, then skip adding and delete 
        // them from callgraph
        if (!CTF.isComplete(*CS)) {
            incompleteFuns.push_back(caller);
            continue;
        }

        // Adding to mapping from callInst to all possible callees
        assert(callInst && "Call Instruction is NULL\n");
        instFunMap[callInst] = callees;

        // Adding to mapping from callers to callees 
        for (std::vector<Function*>::iterator CII = callees.begin(), 
                 CIE = callees.end();
             CII != CIE; ++CII) {
            Function *Fcallee = const_cast<Function*>(*CII);
            callgraphMap[caller].push_back(Fcallee);
        }
    }

    // remove all the incomplete functions from the callgraphMap
    // we still need to assume there are calls from these functions to callsExternNode 
    for (std::vector<Function*>::iterator FI = incompleteFuns.begin(), 
             FE = incompleteFuns.end();
         FI != FE; ++FI) {
        callgraphMap.erase(*FI);
    }

    // DEBUG
    fprintf(stderr, "Complete ratio is %.2f%%\n", 
            100*(float)(callgraphMap.size())/(float)(callsToExternNode.size()));

}


// Run on Module method for pass
bool DSAExternAnalysis::runOnModule(Module &M)
{
    CallTargetFinder<TDDataStructures> &CTF = 
        getAnalysis<CallTargetFinder<TDDataStructures> >();

    // Find all callsites that's calling to callsExternNode
    // Save results to callsToExternNode
    findAllCallSites(CTF);

    // Save the information from callsExternNode to mappings
    saveToMappings(CTF);
    
    return false;
}


// print out information for debugging purposes
void DSAExternAnalysis::print(raw_ostream &O, const Module *M) const
{
    /*
    // Dumping information from the callgraphMap
    O << "*****************************************\n" 
        "* Dumping information from the callgraphMap:\n"
      << "*****************************************\n\n";

    for (FunctionMap_t::const_iterator CGI = callgraphMap.begin(),
             CGE = callgraphMap.end(); CGI != CGE; ++CGI) {
        Function* caller = CGI->first;
        std::vector<Function*>callees = CGI->second;

        O << caller->getName() << " is calling: \n";

        for (std::vector<Function*>::iterator FI = callees.begin(),
                 FE = callees.end(); FI != FE; ++FI) {
            O << "\t" << (*FI)->getName() << "\n";
        }
    }

    // Dumping information from the instFunMap
    O << "\n*****************************************\n"
      << "* Dumping information from the instFunMap:\n"
      << "*****************************************\n\n";

    for (InstrFunMap_t::const_iterator II = instFunMap.begin(),
             IE = instFunMap.end(); II != IE; ++II) {
        Function* ParentFun = II->first->getParent()->getParent();
        std::vector<Function*>callees = II->second;

        O << ParentFun->getName() << " has instruction calling: \n";

        for (std::vector<Function*>::const_iterator FI = callees.begin(),
                 FE = callees.end(); FI != FE; ++FI) {
            O << "\t" << (*FI)->getName() << "\n";
        }
    }
*/

    return;
}



// register pass
char DSAExternAnalysis::ID = 0;
static RegisterPass<DSAExternAnalysis> X("DSAExternAnalysis", "Find calls to callsExternNode.",
                                         true, /* CFG only? */
                                         true /* Analysis Pass? */);
