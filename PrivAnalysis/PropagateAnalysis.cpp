// ====----------  PropagateAnalysis.cpp ---------*- C++ -*---====
//
// Analysis the call graph, propagate the live privilege property
// from callee up to callers.
// The output of the pass is the whole set of the privilege each 
// function requires to run
//
// ====-------------------------------------------------------====

#include "llvm/Analysis/CallGraph.h"

#include "PropagateAnalysis.h"
#include "LocalAnalysis.h"

#include <array>
#include <vector>
#include <map>
#include <stack>

using namespace llvm;
using namespace llvm::privAnalysis;
using namespace llvm::localAnalysis;
using namespace llvm::propagateAnalysis;

// PropagateAnalysis constructor
PropagateAnalysis::PropagateAnalysis() : ModulePass(ID) { }


// Require Analysis Usage
void PropagateAnalysis::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.setPreservesCFG();
    AU.addRequired<LocalAnalysis>();

    // preserve usage
    AU.setPreservesAll();
}


// Do initialization
bool PropagateAnalysis::doInitialization(Module &M)
{
    return false;
}


// Run on Module
// param: M - the program Module
bool PropagateAnalysis::runOnModule(Module &M)
{
    LocalAnalysis &LA = getAnalysis<LocalAnalysis>();

    // Get all data structures for propagation analysis
    FuncCAPTable = LA.FuncCAPTable;
    BBCAPTable = LA.BBCAPTable;
    BBFuncTable = LA.BBFuncTable;

    Propagate(M);

    return false;
}


// Depth First Search data propagation analysis
// param: M - the program module
//        FuncCAPTable - the captable to store live analysis data
void PropagateAnalysis::Propagate(Module &M)
{
    // The ins and outs of function
    FuncCAPTable_t FuncCAPTable_in;
    FuncCAPTable_t FuncCAPTable_out;
    CallGraph CG(M);
    bool ischanged = true;

    // copy to FuncCAPTable_in
    // TODO: Is FuncCAPTable_in really needed here?
    // TODO: Could be using FuncCAPTable directly?
    CopyTableKeys(FuncCAPTable_in, FuncCAPTable);
    CopyTableKeys(FuncCAPTable_out, FuncCAPTable);

    // Keep iterating until converged
    while (ischanged) {
        ischanged = false;

        // iterate the whole callgraph
        for (CallGraph::iterator NI = CG.begin(), NE = CG.end();
             NI != NE; ++NI) {

            // Get CallgraphNode
            CallGraphNode *N = NI->second;
            Function *FCaller = N->getFunction();

            // protector
            if (!FCaller) { continue; }

            // Get Caller mapped array in FuncCAPTables
            CAPArray_t &callerIn = FuncCAPTable_in[FCaller];
            CAPArray_t &callerOut = FuncCAPTable_out[FCaller];

            // Iterate through Callgraphnode for callees
            for (CallGraphNode::iterator RI = N->begin(), RE = N->end();
                 RI != RE; ++ RI) {
                // Get callee
                Function *FCallee = RI->second->getFunction();
                if (!FCallee) { continue; }

                // push to the worklist
                bool found = false;
                for (auto II = internalFuncList.begin(), 
                         IE = internalFuncList.end(); II != IE; ++II) {
                    if (*II == FCallee) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    internalFuncList.push_back(FCallee);
                    worklist.push(CG[FCallee]);
                }

                CAPArray_t &calleeIn = FuncCAPTable_in[FCallee];
                // Propagate all information from callee to caller_out 
                ischanged |= UnionCAPArrays(callerOut, calleeIn);
            } // Iterate through Callgraphnode for callees

            // Propagate all information from caller_out to caller_in
            ischanged |= UnionCAPArrays(callerOut, FuncCAPTable[FCaller]);
            ischanged |= UnionCAPArrays(callerIn, callerOut);
        } // iterator for caller nodes
    } // main loop
    FuncCAPTable = FuncCAPTable_in;
}


// Print out information for debugging purposes
void PropagateAnalysis::print(raw_ostream &O, const Module *M) const
{
    for (auto I = FuncCAPTable.begin(), E = FuncCAPTable.end();
         I != E; ++I) {
        Function *F = (*I).first;
        CAPArray_t A = (*I).second;

        O << F->getName() << ": ";
        dumpCAPArray(O, A);
        O << "\n";
    }
}


// register pass
char PropagateAnalysis::ID = 0;
static RegisterPass<PropagateAnalysis> X("PropagateAnalysis", "Privilege Propagate Analysis.",
                                         true, /* CFG only? */
                                         true /* Analysis Pass? */);
