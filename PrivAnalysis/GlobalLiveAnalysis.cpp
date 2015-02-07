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

#include <array>
#include <vector>
#include <map>


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
    errs() << "\nRunning Global Live Analysis pass\n\n";

    PropagateAnalysis &PA = getAnalysis<PropagateAnalysis>();

    // retrieve all data structures
    FuncCAPTable_t &FuncUseCAPTable = PA.FuncCAPTable;
    // TODO: retrieve information directly from LocalAnalysis?
    BBCAPTable_t &BBCAPTable = PA.BBCAPTable;
    BBFuncTable_t &BBFuncTable = PA.BBFuncTable;

    // init data structure
    bool ischanged = true;

    // FuncLiveCAPTable maps from Functions to the
    // live CAP in the Functions
//    FuncCAPTable_t FuncLiveCAPTable_in;
//    FuncCAPTable_t FuncLiveCAPTable_out;
    BBCAPTable_t BBCAPTable_in;
    BBCAPTable_t BBCAPTable_out;

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

            // Push information to the entry of function live table
            //      BasicBlock &EntryBB = F->getEntryBlock();
            ischanged |= UnionCAPArrays(FuncLiveCAPTable_in[F], FuncUseCAPTable[F]);
      
            // iterate all BBs
            for (Function::iterator BI = F->begin(), BE = F->end();
                 BI != BE;
                 ++ BI) {
                BasicBlock *B = dyn_cast<BasicBlock>(BI);
                if (B == NULL) {
                    continue;
                }
                //////////////////////////////////
                // Propagate information in each BB
                //////////////////////////////////

                // if it's a terminating BB, propagate the info
                // from func live CAPTable to BB[out]
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
                    BSI != BSE;
                    ++ BSI) {
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
        if (DiffCAPArrays(BBCAPTable_drop[B], CAPArray_in, CAPArray_out) == 0) {
            BBCAPTable_drop.erase(B);
        }

        // OUTPUT Live Set change info
        else {
            errs() << "// ---------------------- //\n"
                   << "Debug output for Live CAP set change in function "
                   << B->getParent()->getName()
                   << "\n// ---------------------- //\n";

            errs() << "BB in:\n";
            for (unsigned i = 0; i < CAPArray_in.size(); ++i) {
                if (CAPArray_in[i]) {
                    errs() << i << "\t";
                }
            }

            errs() << "\nBB out:\n";
            for (unsigned i = 0; i < CAPArray_out.size(); ++i) {
                if (CAPArray_out[i]) {
                    errs() << i << "\t";
                }
            }

            errs() << "\nBB change:\n";
            for (unsigned i = 0; i < BBCAPTable_drop[B].size(); ++i) {
                if (BBCAPTable_drop[B][i]) {
                    errs() << i << "\t";
                }
            }
            errs() << "\n// ---------------------- //\n"
                   << "Debug output for Live CAP set change\n"
                   << "// ---------------------- //\n";

        }

    }

    ////////////////////////////////////////
    // DEBUG
    ////////////////////////////////////////

    errs() << "BBCAPTable_in size " << BBCAPTable_in.size() << "\n";

    ////////////////////////////////////////
    // DEBUG
    ////////////////////////////////////////
    errs() << "BBCAPTable size " << BBCAPTable_drop.size() << "\n";
    // Dump in and out for each BB
    int count = 0;
    for (auto bi = BBCAPTable_in.begin(); bi != BBCAPTable_in.end(); ++bi) {
        BasicBlock *B = bi->first;
        CAPArray_t &CAPArray_in = bi->second;
        CAPArray_t &CAPArray_out = BBCAPTable_out[B];
        ++count;
        // In 
        errs() << "BB" << count
               << " in " << B->getParent()->getName() << ":  \t";
        for (unsigned int i = 0; i < CAPArray_in.size(); ++i) {
            if(CAPArray_in[i]){
                errs() << i << "\t";
            }
        }
        errs() << "\n";
        errs() << "BB" << count
               << " out " << B->getParent()->getName() << ":  \t";
        // Out
        for (unsigned int i = 0; i < CAPArray_in.size(); ++i) {
            if(CAPArray_out[i]){
                errs() << i << "\t";
            }
        }
        errs() << "\n";
    }
    errs() << "\n";

    // Dump the drop for each BB
    for (auto bi = BBCAPTable_drop.begin(); bi != BBCAPTable_drop.end(); ++bi) {
        BasicBlock *B = bi->first;
        CAPArray_t &CAPArray_drop = bi->second;
        ++count;
        errs() << "BB" << count
               << " drop " << B->getParent()->getName() << ":  \t";
        for (unsigned int i = 0; i < CAPArray_drop.size(); ++i) {
            if(CAPArray_drop[i]){
                errs() << i << "\t";
            }
        }
        errs() << "\n";
    }
    errs() << "\n";
    ////////////////////////////////////////

    return false;
}


// register pass
char GlobalLiveAnalysis::ID = 1;
static RegisterPass<GlobalLiveAnalysis> L("GlobalLiveAnalysis", "Global privilege live analysis", true, true);
