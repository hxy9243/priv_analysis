// ====------------  DSAExternTarget.h ----------*- C++ -*---====
//
// Find information about call sites from DSA analysis
//
// callsToExternNode will contain calls to calls external node
// in the LLVM callgraph, which is complete in DSA
//
// ====------------------------------------------------------====


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

    // AU.addRequired<LocalAnalysis>();
    AU.addRequired<CallTargetFinder<TDDataStructures> >();    

    AU.setPreservesAll();
}


// Do initialization
bool DSAExternTarget::doInitialization(Module &M)
{
    return false;
}


// Find out all callsites, save to availCallSites
void DSAExternTarget::findAllCallSites(CallTargetFinder<TDDataStructures> &CTF)
{
    callsToExternNode = {};

    // Iterate through the CallTargetFinder for all callsites
    for (std::list<CallSite>::iterator CSI = CTF.cs_begin(), CSE = CTF.cs_end();
         CSI != CSE; ++CSI) {
        CallSite &CS = *CSI;

        // // If callsite is still incomplete, we don't know all callees of callsite,  skip it
        // if (!CTF.isComplete(CS)) {
        //     continue;
        // }
        
        // If a direct call, don't bother
        Function *CF = CS.getCalledFunction();
        if (CF) {
            continue;
        }
        
        if (dyn_cast<Function>(CS.getCalledValue()->stripPointerCasts())) {
            errs() << "strip Pointer casts\n";

            continue;
        }

        if (isa<ConstantPointerNull>(CS.getCalledValue()->stripPointerCasts())) {
            continue;
        }

        // Add it to the data structure callsToExternNode
        // TODO: here it presumes that anything besides direct call is a call to 
        // TODO: CallsExternNode
        for (std::vector<const Function*>::iterator FI = CTF.begin(CS), FE = CTF.end(CS);
             FI != FE; ++FI) {
            callsToExternNode[&CS].push_back(*FI);
        }
    }
}


// Run on Module method for pass
bool DSAExternTarget::runOnModule(Module &M)
{
    CallTargetFinder<TDDataStructures> &CTF = 
        getAnalysis<CallTargetFinder<TDDataStructures> >();

    findAllCallSites(CTF);

    functionMap = {};
    instFunMap = {};
    std::vector<Function*>incompleteFuns = {};

    // Find all functions calling to callsExternNode, see if they're complete in DSA analysis
    for (CallSiteMap_t::iterator FI = callsToExternNode.begin(), FE = callsToExternNode.end();
         FI != FE; ++FI) {
        CallSite *CS = FI->first;
        CallInst *callInst = dyn_cast<CallInst>(FI->first->getInstruction());
        Function* caller = callInst->getParent()->getParent();
        std::vector<const Function*>callees = FI->second;

        // if function callsite is incomplete, then skip adding and delete them from callgraph
        if (!CTF.isComplete(*CS)) {
            incompleteFuns.push_back(caller);
            continue;
        }

        // adding to instruction to mapping
        assert(callInst && "Call Instruction is NULL\n");
        instFunMap[callInst] = callees;

        // adding to function mapping, push all callees to caller's function vector
        for (std::vector<const Function*>::iterator CII = callees.begin(), CIE = callees.end();
             CII != CIE; ++CII) {
            functionMap[caller].push_back(*CII);
        }
    }

    // remove all the incomplete functions from the mappings
    // we still need to assume there are calls from these functions to callsExternNode  
    for (std::vector<Function*>::iterator FI = incompleteFuns.begin(), FE = incompleteFuns.end();
         FI != FE; ++FI) {
        functionMap.erase(*FI);
    }
    

    // ------------------------------------- //
    // Find all info for CFG analysis
    // ------------------------------------- //
    // iterate through all the callsToExternNode
    for (CallSiteMap_t::iterator FI = callsToExternNode.begin(), FE = callsToExternNode.end();
         FI != FE; ++FI) {

        // TODO: 

    }
    
    return false;
}


// print out information for debugging purposes
void DSAExternTarget::print(raw_ostream &O, const Module *M) const
{
    for (CallSiteMap_t::const_iterator FMI = callsToExternNode.begin(),
             FME = callsToExternNode.end(); FMI != FME; ++FMI) {
        Function *Caller = FMI->first->getInstruction()->getParent()->getParent();

        if (Caller != NULL) {
            O << Caller->getName() << ":\t";
        }
        else {
            O << "NULL\n";
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
