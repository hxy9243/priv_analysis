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

#include <linux/capability.h>
#include <map>
#include <array>

#define TARGET_FUNC "priv_raise"
#define CAP_TOTALNUM (CAP_LAST_CAP + 1)

namespace llvm{
namespace localAnalysis {

  struct LocalAnalysis : public ModulePass {
  public:
    static char ID;
    // Data structure for priv_lower capabilities in each function
    // Maps from InstCalls to -> Array of Capabilities
    std::map<Function *, std::array<bool, CAP_TOTALNUM> >CAPTable;

    // constructor
    LocalAnalysis();

    // initialization method
    virtual bool doInitialization(Module &M);

    // Run on Module start
    virtual bool runOnModule(Module &M);

    // Preserve all analysis usage
    void getAnalysisUsage(AnalysisUsage &AU) const;

  private:
    // Retrieve all capabilities from params of function call
    void RetrieveAllCAP(CallInst *CI, std::array<bool, CAP_TOTALNUM>&CAParray);

    // Get the function where the CallInst is in, add to map
    void AddFuncToMap(Function *tf, std::array<bool, CAP_TOTALNUM>CAParray);
  }; // endof struct PrivAnalysis

} // namespace localanalysis
} // namespace llvm


#endif



