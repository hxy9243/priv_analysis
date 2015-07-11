// ====------------- GlobalLiveAnalysis.h --------*- C++ -*---====
//
// Globally Privilge Live Analysis for each basic
// blocks. The output would be the privileges to drop for each
// basic blocks
//
// ====-------------------------------------------------------====

#ifndef __GLOBALLIVEANALYSIS__
#define __GLOBALLIVEANALYSIS__

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "ADT.h"
#include "SplitBB.h"

#include <map>

using namespace llvm::privAnalysis;

namespace llvm {
namespace globalLiveAnalysis {

struct GlobalLiveAnalysis : public ModulePass
{
public:
    static char ID;

    // Data structures to save data
    BBCAPTable_t BBCAPTable_in;
    BBCAPTable_t BBCAPTable_out;
    BBCAPTable_t BBCAPTable_dropEnd;
    BBCAPTable_t BBCAPTable_dropStart;
    FuncCAPTable_t FuncLiveCAPTable_in;
    FuncCAPTable_t FuncLiveCAPTable_out;

    // Record exit BB of 
    typedef std::map<Function*, BasicBlock*> FuncReturnBB_t;

    // The unique capability set
    CAPSet_t CAPSet;

    GlobalLiveAnalysis();

    // Initialization
    virtual bool doInitialization(Module &M);

    // Run on Module Start
    virtual bool runOnModule(Module &M);

    // Preserve analysis usage
    void getAnalysisUsage(AnalysisUsage &AU) const;

    // Print out information for debugging purposes
    void print(raw_ostream &O, const Module *M) const;

private:
    // find exit BB of functions inside a Module
    void findReturnBB(Module& M, FuncReturnBB_t&);

    // get the unique privilege set 
    void findUniqueSet();

    void dumpTable();
};

} // namespace globalLiveAnalysis
} // namespace llvm

#endif
