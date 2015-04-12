// ====-----------  FindExternNodes.cpp ----------*- C++ -*---====
//
// Helper pass. Find external nodes which use privileges 
//
// In LLVM, there's an external calling node calling all functions
// with external visiblities
// 
// This pass identifies functions with following properties
// 1. Still have external linkage after 'internalize' opt pass
// 2. Not empty (not function declaration)
// 3. Uses privileges inside function
// 
// These are functions we need to pay special attention to. As
// they are used as function pointers and their callsites aren't 
// specifically marked in the control flow graph.
// 
// ====-------------------------------------------------------====


#include "FindExternNodes.h"

#include <vector>


using namespace llvm;
using namespace llvm::privAnalysis;
using namespace llvm::localAnalysis;
using namespace llvm::propagateAnalysis;
using namespace llvm::findexternnodes;


FindExternNodes::FindExternNodes() : ModulePass(ID) { } 


// Require analysis usage
void FindExternNodes::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.setPreservesCFG();
    AU.addRequired<PropagateAnalysis>();

    AU.setPreservesAll();
}


// Do initialization
bool FindExternNodes::doInitialization(Module &M)
{
    return false;
}


// Run on Module
bool FindExternNodes::runOnModule(Module &M)
{
    PropagateAnalysis &PA = getAnalysis<PropagateAnalysis>();

    // get data structures
    FuncCAPTable_t FuncCAPTable;
    
    // get all nodes calling from externcallingnode
    // TODO: problem here
    for (auto FI = PA.FuncCAPTable.begin(), FE = PA.FuncCAPTable.end();
         FI != FE; ++FI) {
        if (!IsCAPArrayEmpty(FI->second)) {
            FuncCAPTable[FI->first] = FI->second;
        }
    }

    CallGraph CG(M);
    CallGraphNode *externNode = CG.getExternalCallingNode();

    for (CallGraphNode::iterator NI = externNode->begin(), NE = externNode->end();
         NI != NE; ++NI) {
        Function* CalledFunc = NI->second->getFunction();
        assert(CalledFunc != NULL && "Callgraph node is NULL\n");

        if (CalledFunc->empty()) { continue; }

        if (FuncCAPTable.find(CalledFunc) != FuncCAPTable.end()) {
            ExternPrivNodes[CalledFunc] = FuncCAPTable[CalledFunc];
        }

    }

    return false;
} 


// print out 
void FindExternNodes::print(raw_ostream &O, const Module *M) const
{
    for (auto FI = ExternPrivNodes.begin(), FE = ExternPrivNodes.end();
         FI != FE; ++FI) {
        O << FI->first->getName() << ":\t";
        dumpCAPArray(O, FI->second);
    }
}


// register pass
char FindExternNodes::ID = 0;
static RegisterPass<FindExternNodes> F("FindExternNodes", "Find External Nodes",
                                       true, /* CFG only? */
                                       true  /* Analysis pass? */);
