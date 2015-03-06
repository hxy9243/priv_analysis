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

#include "GlobalLiveAnalysis.h"
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
using namespace llvm::globalLiveAnalysis;


// GlobalLiveAnalysis constructor
GlobalLiveAnalysis::GlobalLiveAnalysis() : ModulePass(ID) {}


// Require Analysis usage
void GlobalLiveAnalysis::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.setPreservesAll();

    AU.addRequired<UnifyFunctionExitNodes>();
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

    // init data structure
    bool ischanged = true;

    // iterate till convergence
    while (ischanged) {

        ischanged = false;

        // iterate through all functions
        for (Module::iterator FI = M.begin(), FE = M.end(); FI != FE; ++FI) {
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
            
            if (ReturnBB == NULL && UnReachableBB == NULL) {
                bool haveReturn = 0;
                for (Function::iterator I = F->begin(), E = F->end(); I != E; ++I) {
                    BasicBlock *RetBB = dyn_cast<BasicBlock>(I);
                    
                    if (isa<ReturnInst>(RetBB->getTerminator())){
                        errs() << "I have a terminator\n";
                        haveReturn = 1;
                    }
                }
            }

            assert((ReturnBB != NULL || UnReachableBB != NULL) && "Return BB is NULL\n");

            // Push information to the entry of function live table
            //      BasicBlock &EntryBB = F->getEntryBlock();
            //ischanged |= UnionCAPArrays(FuncLiveCAPTable_in[F], FuncUseCAPTable[F]);
      
            // iterate all BBs
            for (Function::iterator BI = F->begin(), BE = F->end();
                 BI != BE; ++ BI) {
                BasicBlock *B = dyn_cast<BasicBlock>(BI);
                if (B == NULL) {
                    continue;
                }
                //////////////////////////////////
                // Propagate information in each BB
                //////////////////////////////////

                // if it's a terminating BB, propagate the info from func live CAPTable
                // to BB[out]. Note that only returnBBs are considered in data propagation
                // UnreachableBBs are not.
                if (ReturnBB == B) {
                    ischanged |= UnionCAPArrays(BBCAPTable_out[B], 
                                                FuncLiveCAPTable_out[F]);
                }

                // if it's a FunCall BB (found as key in BBFuncTable), add the live
                // info to func live CAPTable for callee processing
                if (BBFuncTable.find(B) != BBFuncTable.end()) {
                    ischanged |= UnionCAPArrays(BBCAPTable_in[B],
                                                FuncUseCAPTable[BBFuncTable[B]]);

                    ischanged |= UnionCAPArrays(FuncLiveCAPTable_out[BBFuncTable[B]],
                                                BBCAPTable_out[B]);
                }

                // if it's a Priv Call BB, Propagate privilege to the in of BB
                if (BBCAPTable.find(B) != BBCAPTable.end()) {
                    ischanged |= UnionCAPArrays(BBCAPTable_in[B], BBCAPTable[B]);
                }

                // propagate from all its successors
                TerminatorInst *BBTerm = B->getTerminator();
                for(unsigned BSI = 0, BSE = BBTerm->getNumSuccessors(); 
                    BSI != BSE; ++ BSI) {
                    BasicBlock *SuccessorBB = BBTerm->getSuccessor(BSI);
                    ischanged |= UnionCAPArrays(BBCAPTable_out[B], 
                                                BBCAPTable_in[SuccessorBB]);
                }
                // propagate live info from out[B] to in[B] for each BB
                ischanged |= UnionCAPArrays(BBCAPTable_in[B], BBCAPTable_out[B]);
            } // iterate all BBs
        } // iterate all functions

    } // while change

    ////////////////////////////////////////
    // Find Difference of BB in and out CAPArrays
    // Save it to the output 
    ////////////////////////////////////////
    for (auto bi = BBCAPTable_out.begin(); bi != BBCAPTable_out.end(); ++bi) {
        BasicBlock *B = bi->first;
        CAPArray_t &CAPArray_out = bi->second;
        CAPArray_t &CAPArray_in = BBCAPTable_in[B];

        // compare the in and the out of the same BB
        if (DiffCAPArrays(BBCAPTable_dropEnd[B], CAPArray_in, CAPArray_out) == 0) {
            BBCAPTable_dropEnd.erase(B);
        }

        // compare the out with all ins of the child BB, put in drop start of children
        TerminatorInst *BBTerm = B->getTerminator();
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

    ////////////////////////////////////////
    // DEBUG
    ////////////////////////////////////////

    // errs() << "BBCAPTable_in size " << BBCAPTable_in.size() << "\n";

    // ////////////////////////////////////////
    // // DEBUG
    // ////////////////////////////////////////
    // errs() << "BBCAPTable size " << BBCAPTable_dropEnd.size() << "\n";
    // // Dump in and out for each BB
    // int count = 0;
    // for (auto bi = BBCAPTable_in.begin(); bi != BBCAPTable_in.end(); ++bi) {
    //     BasicBlock *B = bi->first;
    //     CAPArray_t &CAPArray_in = bi->second;
    //     CAPArray_t &CAPArray_out = BBCAPTable_out[B];
    //     ++count;
    //     // In 
    //     errs() << "BB" << count
    //            << " in " << B->getParent()->getName() << ":  \t";
    //     for (unsigned int i = 0; i < CAPArray_in.size(); ++i) {
    //         if(CAPArray_in[i]){
    //             errs() << i << "\t";
    //         }
    //     }
    //     errs() << "\n";
    //     errs() << "BB" << count
    //            << " out " << B->getParent()->getName() << ":  \t";
    //     // Out
    //     for (unsigned int i = 0; i < CAPArray_in.size(); ++i) {
    //         if(CAPArray_out[i]){
    //             errs() << i << "\t";
    //         }
    //     }
    //     errs() << "\n";
    // }
    // errs() << "\n";

    // Dump the drop for each BB
    // for (auto bi = BBCAPTable_dropEnd.begin(); bi != BBCAPTable_dropEnd.end(); ++bi) {
    //     BasicBlock *B = bi->first;
    //     CAPArray_t &CAPArray_drop = bi->second;
    //     ++count;
    //     errs() << "BB" << count
    //            << " drop End " << B->getParent()->getName() << ":  \t";
    //     for (unsigned int i = 0; i < CAPArray_drop.size(); ++i) {
    //         if(CAPArray_drop[i]){
    //             errs() << i << "\t";
    //         }
    //     }
    //     errs() << "\n";
    // }
    // errs() << "\n";

    // for (auto bi = BBCAPTable_dropStart.begin(); bi != BBCAPTable_dropStart.end(); ++bi) {
    //     BasicBlock *B = bi->first;
    //     CAPArray_t &CAPArray_drop = bi->second;
    //     ++count;
    //     errs() << "BB" << count
    //            << " drop Start " << B->getParent()->getName() << ":  \t";
    //     for (unsigned int i = 0; i < CAPArray_drop.size(); ++i) {
    //         if(CAPArray_drop[i]){
    //             errs() << i << "\t";
    //         }
    //     }
    //     errs() << "\n";
    // }
    // errs() << "\n";
    ////////////////////////////////////////

    findUniqueSet();

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





// register pass
char GlobalLiveAnalysis::ID = 1;
static RegisterPass<GlobalLiveAnalysis> L("GlobalLiveAnalysis", "Global privilege live analysis", 
                                          true, /* CFG only? */
                                          true  /* Analysis Pass? */);
