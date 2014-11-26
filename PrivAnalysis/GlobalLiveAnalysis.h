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

using namespace llvm::privAnalysis;

namespace llvm{
namespace globalLiveAnalysis{

  struct GlobalLiveAnalysis : public ModulePass{
  public:
    static char ID;
    GlobalLiveAnalysis();

    // initialization
    virtual bool doInitialization(Module &M);

    // Run on BasicBlock Start
    virtual bool runOnModule(Module &M);

    // Preserve analysis usage
    void getAnalysisUsage(AnalysisUsage &AU) const;

  private:
    

  };

} // namespace globalLiveAnalysis
} // namespace llvm

#endif
