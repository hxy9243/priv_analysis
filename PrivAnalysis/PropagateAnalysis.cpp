// ====----------  PropagateAnalysis.cpp ---------*- C++ -*---====
//
// Analysis the call graph, propagate the live privilege property
// from callee up to callers.
// The output of the pass is the whole set of the privilege each 
// function requires to run
//
// ====-------------------------------------------------------====

#include "llvm/IR/DerivedTypes.h"
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


// Adding dummy function of type void(*)(void)
// param: M    - the module
//        name - function name
Function* PropagateAnalysis::InsertDummyFunc(Module &M, const StringRef name)
{
    Type *voidTy = Type::getVoidTy(M.getContext());
    std::vector<Type *>Params;
    
    FunctionType *dummyType = FunctionType::get(voidTy, ArrayRef<Type *>(Params),
                                                false);
    Constant *func = M.getOrInsertFunction(name, dummyType);
    return dyn_cast<Function>(func);
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

    // Add external calls node function as NULL
    CallGraphNode* callsNode = CG.getCallsExternalNode();
    CallGraphNode* callingNode = CG.getExternalCallingNode();

    Function *callsNodeFunc = InsertDummyFunc(M, "CallsExternNode");
    Function *callingNodeFunc = InsertDummyFunc(M, "CallsExternNode");
    FuncCAPTable[callsNodeFunc] = {};
    FuncCAPTable[callingNodeFunc] = {};

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
                 RI != RE; ++RI) {
                if (RI->second == callingNode) { continue; }

                // Get callee
                Function *FCallee;
                if (RI->second == callsNode) { FCallee = callsNodeFunc; }
                else { FCallee = RI->second->getFunction(); }

                // calls external node has no corresponding function
                // if (FCallee == NULL) { continue; }

                CAPArray_t &calleeIn = FuncCAPTable_in[FCallee];
                // Propagate all information from callee to caller_out 
                ischanged |= UnionCAPArrays(callerOut, calleeIn);
            } // Iterate through Callgraphnode for callees

            // Propagate all information from caller_out to caller_in
            ischanged |= UnionCAPArrays(callerOut, FuncCAPTable[FCaller]);
            ischanged |= UnionCAPArrays(callerIn, callerOut);
        } // iterator for caller nodes

        // special handle calls external node, propagate callees of external
        // calling node to this calls external node

        CAPArray_t &callerIn = FuncCAPTable_in[callingNodeFunc];
        CAPArray_t &callerOut = FuncCAPTable_out[callingNodeFunc];
        
        for (CallGraphNode::iterator RI = callingNode->begin(), 
                 RE = callingNode->end(); RI != RE; ++RI) {
            Function *FCallee = RI->second->getFunction();

            if (FCallee == M.getFunction("main")) { continue; }

            CAPArray_t &calleeIn = FuncCAPTable_in[FCallee];

            ischanged |= UnionCAPArrays(callerOut, calleeIn);
        }
        ischanged |= UnionCAPArrays(callerOut, FuncCAPTable[callingNodeFunc]);
        ischanged |= UnionCAPArrays(callerIn, callerOut);

    } // main loop

    FuncCAPTable_in.erase(callsNodeFunc);
    FuncCAPTable_in.erase(callingNodeFunc);

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
    }
}


// register pass
char PropagateAnalysis::ID = 0;
static RegisterPass<PropagateAnalysis> X("PropagateAnalysis", "Privilege Propagate Analysis.",
                                         true, /* CFG only? */
                                         true /* Analysis Pass? */);
