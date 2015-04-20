// ====-------------  ExternalFunCall.h -----------*- C++ -*---====
//
// Global anlaysis of external function calls in the program.
// 
//
// ====-------------------------------------------------------====

#ifndef __EXTERNALFUNCALL_H__
#define __EXTERNALFUNCALL_H__

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>

namespace llvm{
namespace externfuncall {

// LocalAnlysis pass
struct ExternFunCall : public ModulePass 
{
public:
    static char ID;

    // constructor
    ExternFunCall();

    // initialization method
    virtual bool doInitialization(Module &M);

    // Run on Module start
    virtual bool runOnModule(Module &M);

    // Preserve all analysis usage
    void getAnalysisUsage(AnalysisUsage &AU) const;

    void print(raw_ostream & O, const Module * M) const;

private:
    std::vector<Function *> extern_funcs;

    std::vector<Function *> extern_calls;
};


} // namespace externfuncall
} // namespace llvm


#endif
