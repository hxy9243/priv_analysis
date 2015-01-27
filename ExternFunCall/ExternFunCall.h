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
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"


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

private:

};


} // namespace externfuncall
} // namespace llvm


#endif
