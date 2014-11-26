// ====-------------------  SplitBB.h ------------*- C++ -*---====
//
// A BasicBlock Pass to split all priv_* instructions and related 
// function calls to single BBs for next step of analysis.
//
// ====-------------------------------------------------------====

#ifndef __SPLITBB_H__
#define __SPLITBB_H__

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>

#include "ADT.h"

using namespace llvm::privAnalysis;

namespace llvm {
namespace splitBB {

  // SplitBB pass
  // Separate Priv related Insts to Single BBs
  struct SplitBB : public BasicBlockPass {
  public:
    static char ID;
    SplitBB();
    
    // Vector to store BB info for analysis
    std::vector<BasicBlock *> BBtoAnalyze;

    // initialization
    virtual bool doInitialization(Function &F);

    // Run on BasicBlock Start
    virtual bool runOnBasicBlock(BasicBlock &B);

    // Preserve analysis usage
    void getAnalysisUsage(AnalysisUsage &AU) const;

  private:
    // Find out if instruction is from extern library,
    // which doesn't 
    bool isExternLibCall(Function *F);

  }; // struct splitBB

} // namespace slitBB
} // namespace llvm

#endif
