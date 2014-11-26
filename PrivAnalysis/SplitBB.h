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

#define SPLIT_HERE  1
#define SPLIT_NEXT  2


using namespace llvm::privAnalysis;

namespace llvm {
namespace splitBB {

  // SplitBB pass
  // Separate Priv related Insts to Single BBs
  struct SplitBB : public ModulePass {
  public:
    static char ID;
    SplitBB();
    // Store the split location in BasicBlocks
    std::map<BasicBlock *, std::vector<Instruction *> >splitLocationInBB;

    // Vector to store BB info for analysis
    std::vector<BasicBlock *> BBtoAnalyze;

    // initialization
    virtual bool doInitialization(Module &M);

    // Run on BasicBlock Start
    virtual bool runOnModule(Module &M);

    // Preserve analysis usage
    void getAnalysisUsage(AnalysisUsage &AU) const;

  private:
    // Split instruction on all the Function calling sites
    void splitOnFunction(Function *F, int splitLoc);

  }; // struct splitBB

} // namespace slitBB
} // namespace llvm

#endif
