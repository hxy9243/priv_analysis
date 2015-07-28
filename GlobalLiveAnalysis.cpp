// ====------------- GlobalLiveAnalysis.cpp ------*- C++ -*---====
//
// Globally Privilge Live Analysis for each basic
// blocks. The output would be the privileges to drop for each
// basic blocks
//
// ====-------------------------------------------------------====

#include "llvm/Analysis/CallGraph.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"

#include "ADT.h"
#include "GlobalLiveAnalysis.h"
#include "DSAExternAnalysis.h"
#include "PropagateAnalysis.h"
#include "LocalAnalysis.h"

#include <utility>
#include <algorithm>
#include <array>
#include <vector>
#include <map>

#include <cstdlib>

using namespace llvm;
using namespace llvm::propagateAnalysis;
using namespace llvm::splitBB;
using namespace llvm::dsaexterntarget;
using namespace llvm::globalLiveAnalysis;


// GlobalLiveAnalysis constructor
GlobalLiveAnalysis::GlobalLiveAnalysis() : ModulePass(ID) {}


// Require Analysis usage
void GlobalLiveAnalysis::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.setPreservesAll();

    AU.addRequired<UnifyFunctionExitNodes>();
    AU.addRequired<DSAExternAnalysis>();
    AU.addRequired<PropagateAnalysis>();
    AU.addRequired<SplitBB>();
}


// Do initialization
bool GlobalLiveAnalysis::doInitialization(Module &M)
{
    return false;
}


// Run on Module
bool GlobalLiveAnalysis::runOnModule(Module &M)
{
    PropagateAnalysis &PA = getAnalysis<PropagateAnalysis>();

    // retrieve all data structures
    FuncCAPTable_t &FuncUseCAPTable = PA.FuncCAPTable;
    // TODO: retrieve information directly from LocalAnalysis?
    BBCAPTable_t &BBCAPTable = PA.BBCAPTable;
    BBFuncTable_t &BBFuncTable = PA.BBFuncTable;

    Function* callsNodeFunc = PA.callsNodeFunc;

    const DSAExternAnalysis &DSAFinder = getAnalysis<DSAExternAnalysis>();
    CallSiteFunMap_t callsToExternNode = DSAFinder.callsToExternNode;
    InstrFunMap_t instFunMap = DSAFinder.instFunMap;

    // find the returnBB of all functions
    FuncReturnBB_t funcReturnBB;
    findReturnBB(M, funcReturnBB);

    // init data structure
    bool ischanged;

    // iterate the algorithm till convergence
    do {
        ischanged = false;

        // iterate through all functions
        for (auto FI = FuncUseCAPTable.begin(), FE = FuncUseCAPTable.end(); 
             FI != FE; ++FI) {
            Function *F = FI->first;
            if (F == NULL || F->empty()) { continue; }

            // Iterate through all BBs for information propagation
            // Traversing BBs in reverse order now because it's closer to
            // topologically reverse order of how BBs are arranged in LLVM, 
            // and it's faster for dataflow analysis to converge
            Function::iterator BI = F->end(), BBegin = F->begin();
            while (1) {
                if (BI != BBegin) { --BI; }
                else { break; }

                BasicBlock *B = dyn_cast<BasicBlock>(BI);
                if (B == NULL) { continue; }

                // ---------------------------------------------------------- //
                // Propagate information in each BB
                // ---------------------------------------------------------- //
                // if it's a FunCall BB (found as key in BBFuncTable), add the 
                // live info to CAPTable of callee's exit BB
                // TODO: Consider cases for external nodes and 
                // TODO: DSA related info here
                if (BBFuncTable.find(B) != BBFuncTable.end()) {
                    Function* funcall = BBFuncTable[B];

                    // Find the callinst of the BB
                    Instruction* BBcallInst = B->getFirstNonPHI();

                    CallSite CS(BBcallInst);

                    // If calling to externnode
                    if (callsToExternNode.find(&CS) != callsToExternNode.end()) {

                        // Skip LLVM intrinsic functions
                        if (isa<IntrinsicInst>(BBcallInst)) { continue; }

                        // DEBUG
                        errs() << "Empty function: " << funcall->getName() << "\n";

                        // If complete from DSA analysis
                        if (instFunMap.find(BBcallInst) != instFunMap.end()) {
                            std::vector<Function*> Instcallees = instFunMap[BBcallInst];
                            for (std::vector<Function*>::iterator II = Instcallees.begin(),
                                     IE = Instcallees.end(); II != IE; ++II) {
                                Function* callee = *II;
                                ischanged |= UnionCAPArrays(BBCAPTable_in[B],
                                                            FuncUseCAPTable[callee]);
                                ischanged |= UnionCAPArrays(BBCAPTable_out[funcReturnBB[callee]],
                                                            BBCAPTable_out[B]);
                            }
                        }
                        // else if incomplete, propagate from callsExternNode
                        else {
                            ischanged |= UnionCAPArrays(BBCAPTable_in[B],
                                                        FuncUseCAPTable[callsNodeFunc]);
                        }
                    }

                    ischanged |= UnionCAPArrays(BBCAPTable_in[B],
                                                FuncUseCAPTable[funcall]);
                    // propagate information to returnBB of function
                    Function *callee = BBFuncTable[B];
                    ischanged |= UnionCAPArrays(BBCAPTable_out[funcReturnBB[callee]],
                                                BBCAPTable_out[B]);
                }

                // if it's a Priv Call BB, Propagate privilege to the in of BB
                if (BBCAPTable.find(B) != BBCAPTable.end()) {
                    ischanged |= UnionCAPArrays(BBCAPTable_in[B], BBCAPTable[B]);
                    //ischangedFunc |= ischanged;
                }

                // propagate from all its successors
                TerminatorInst *BBTerm = B->getTerminator();

                for (unsigned BSI = 0, BSE = BBTerm->getNumSuccessors(); 
                     BSI != BSE; ++ BSI) {
                    BasicBlock *SuccessorBB = BBTerm->getSuccessor(BSI);
                    assert(SuccessorBB && "Successor BB is NULL!");
                    ischanged |= UnionCAPArrays(BBCAPTable_out[B], 
                                                BBCAPTable_in[SuccessorBB]);
                    // ischangedFunc |= ischanged;
                }
                // propagate live info from out[B] to in[B] for each BB
                ischanged |= UnionCAPArrays(BBCAPTable_in[B], BBCAPTable_out[B]);
            } // iterate all BBs

        } // iterate all functions
    } while (ischanged); // main loop

    // ------------------------------------------ //
    // Find Difference of BB in and out CAPArrays
    // Save it to the output 
    // ------------------------------------------ //
    for (auto bi = BBCAPTable_out.begin(); bi != BBCAPTable_out.end(); ++bi) {
        BasicBlock *B = bi->first;
        if (B == NULL ) { continue; }

        CAPArray_t &CAPArray_out = bi->second;
        CAPArray_t &CAPArray_in = BBCAPTable_in[B];

        // compare the in and the out of the same BB
        if (DiffCAPArrays(BBCAPTable_dropEnd[B], CAPArray_in, CAPArray_out) == 0) {
            BBCAPTable_dropEnd.erase(B);
        }

        // compare the out with all ins of the child BB, put in drop start of children
        const TerminatorInst *BBTerm = B->getTerminator();

        for(unsigned BSI = 0, BSE = BBTerm->getNumSuccessors(); 
            BSI != BSE; ++ BSI) {
            BasicBlock *SuccessorBB = BBTerm->getSuccessor(BSI);
            CAPArray_t CAPSuccessor_in = BBCAPTable_in[SuccessorBB];

            DiffCAPArrays(BBCAPTable_dropStart[SuccessorBB], 
                          CAPArray_out, CAPSuccessor_in);
        }
    }

    // Remove unnecessary BBDropstart
    for (auto bi = BBCAPTable_out.begin(); 
         bi != BBCAPTable_out.end(); ++bi) {

        if (IsCAPArrayEmpty(BBCAPTable_dropStart[bi->first])) {
            BBCAPTable_dropStart.erase(bi->first);
        }
    }

    // ----------------------------------- //
    // DEBUG
    // ----------------------------------- //
    // Dump the table for debugging
    // dumpTable();

    // Find unique set for debug output or ROSA
    // findUniqueSet();

    return false;
}


// get the unique privilege set, save the result to CAPSet
void GlobalLiveAnalysis::findUniqueSet()
{
    // Analyze BBCAPTable_in
    for (auto BI = BBCAPTable_in.begin(), BE = BBCAPTable_in.end();
         BI != BE; ++BI) {
        if (CAPSet.find(BI->second) == CAPSet.end()) {
            CAPSet[BI->second] = findCAPArraySize(BI->second);
        }
    }
}


// find the exit BB of all functions using Unify Exit Node
void GlobalLiveAnalysis::findReturnBB(Module &M, FuncReturnBB_t& FuncReturnBB)
{
    for (auto FI = M.begin(), FE = M.end(); FI != FE; ++FI) {
        Function *F = dyn_cast<Function>(FI);
        if (F == NULL || F->empty()) {
            continue;
        }

        // Find the exit node of the Function
        // TODO: Separate UnifyExitNodes as an individual transformation pass?
        UnifyFunctionExitNodes &UnifyExitNode = getAnalysis<UnifyFunctionExitNodes>(*F);
        UnifyExitNode.runOnFunction(*F);
        BasicBlock *ReturnBB = UnifyExitNode.getReturnBlock();
        BasicBlock *UnReachableBB = UnifyExitNode.getUnreachableBlock();
        BasicBlock *UnwindBB = UnifyExitNode.getUnwindBlock();

        assert(UnwindBB == NULL && "So far not dealing with unwind block\n");
        assert((ReturnBB != NULL || UnReachableBB != NULL) && "Return BB is NULL\n");

        FuncReturnBB[F] = ReturnBB;
    }
}


// helper function for sorting alogrithm used in print
bool compareSet(const std::pair<CAPArray_t, int> A,
                const std::pair<CAPArray_t, int> B)
{
    return (A.second > B.second);
}


// Print out information for debugging purposes
void GlobalLiveAnalysis::print(raw_ostream &O, const Module *M) const
{
    errs() << "Dumping information for unique capability set.\n\n";

    std::vector<std::pair<CAPArray_t, int> >Setcopy(CAPSet.begin(), CAPSet.end());

    std::sort(Setcopy.begin(), Setcopy.end(), compareSet);
    
    int line = 0;

    // dump the uniq set of capabilities
    for (auto BI = Setcopy.begin(), BE = Setcopy.end(); BI != BE; ++BI) {
        O << line++ << ": ";

        dumpCAPArray(O, BI->first);
    }
}


// Dump BBCAPTable for Debugging purpose
void GlobalLiveAnalysis::dumpTable()
{
    errs() << "BBCAPTable_in size " << BBCAPTable_in.size() << "\n";

    ////////////////////////////////////////
    // DEBUG
    ////////////////////////////////////////
    errs() << "BBCAPTable size " << BBCAPTable_dropEnd.size() << "\n";
    // Dump in and out for each BB
    int count = 0;
    for (auto bi = BBCAPTable_in.begin(); 
         bi != BBCAPTable_in.end(); ++bi) {
        BasicBlock *B = bi->first;
        CAPArray_t &CAPArray_in = bi->second;
        CAPArray_t &CAPArray_out = BBCAPTable_out[B];
        ++count;
        // In 
        errs() << "BB" << count
               << " in " << B->getParent()->getName() << ":  \t";
        for (int i = 0; i < CAP_TOTALNUM; ++i) {
            if (CAPArray_in & ((uint64_t) 1 << i)){
                errs() << i << "\t";
            }
        }

        errs() << "\nBB" << count
               << " out " << B->getParent()->getName() << ":  \t";
        // Out
        for (unsigned int i = 0; i < CAP_TOTALNUM; ++i) {
            if(CAPArray_out & ((uint64_t) 1 << i)){
                errs() << i << "\t";
            }
        }
        errs() << "\n";
    }
    errs() << "\n";

    // Dump the drop for each BB
    for (auto bi = BBCAPTable_dropEnd.begin(); 
         bi != BBCAPTable_dropEnd.end(); ++bi) {
        BasicBlock *B = bi->first;
        CAPArray_t &CAPArray_drop = bi->second;
        ++count;
        errs() << "BB" << count
               << " drop End " << B->getParent()->getName() << ":  \t";
        for (int i = 0; i < CAP_TOTALNUM; ++i) {
            if (CAPArray_drop & ((uint64_t) 1 << i)){
                errs() << i << "\t";
            }
        }
        errs() << "\n";
    }
    errs() << "\n";

    for (auto bi = BBCAPTable_dropStart.begin(); 
         bi != BBCAPTable_dropStart.end(); ++bi) {
        BasicBlock *B = bi->first;
        CAPArray_t &CAPArray_drop = bi->second;
        ++count;
        errs() << "BB" << count
               << " drop Start " << B->getParent()->getName() << ":  \t";
        for (unsigned int i = 0; i < CAP_TOTALNUM; ++i) {
            if(CAPArray_drop & ((uint64_t) 1 << i)){
                errs() << i << "\t";
            }
        }
        errs() << "\n";
    }
    errs() << "\n";
    
}


// register pass
char GlobalLiveAnalysis::ID = 1;
static RegisterPass<GlobalLiveAnalysis> L("GlobalLiveAnalysis", "Global privilege live analysis", 
                                          true, /* CFG only? */
                                          true  /* Analysis Pass? */);
