// ====-------------  LocalAnalysis.h -----------*- C++ -*---====
//
// Local analysis of priv_lower calls in Function blocks.
// Find all the priv_lower calls inside each of the functions.
//
// ====-------------------------------------------------------====

#ifndef __LOCALANALYSIS_H__
#define __LOCALANALYSIS_H__

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "ADT.h"

using namespace llvm::privAnalysis;

namespace llvm{
namespace localAnalysis {

// LocalAnlysis pass
struct LocalAnalysis : public ModulePass 
{
public:
    static char ID;
    // Data structure for local priv capability use in each function
    // Maps from InstCalls to -> Array of Capabilities
    FuncCAPTable_t FuncCAPTable;

    // Data structure for priv capability use in each BB
    // Maps from BB to -> Array of Capabilities
    BBCAPTable_t BBCAPTable;

    // Map from BB to its non-external Function Calls
    BBFuncTable_t BBFuncTable;

    // Vector to store new direct JMP as terminator
    std::vector<BasicBlock *> ExtraJMPBB;

    // constructor
    LocalAnalysis();

    // initialization method
    virtual bool doInitialization(Module &M);

    // Run on Module start
    virtual bool runOnModule(Module &M);

    // Preserve all analysis usage
    void getAnalysisUsage(AnalysisUsage &AU) const;

    // Print out information for debugging purposes
    void print(raw_ostream &O, const Module *M) const;
private:
    // Retrieve all capabilities from params of function call
    void RetrieveAllCAP(CallInst *CI, CAPArray_t &CAParray);

}; // endof struct PrivAnalysis


} // namespace localanalysis
} // namespace llvm


#endif
