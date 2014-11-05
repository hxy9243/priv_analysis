// ====---------------  PrivAnalysis.cpp ---------*- C++ -*---====
//
// Local analysis of priv_lower calls in Function blocks.
// Find all the priv_lower calls inside each of the functions,
// and propagate all the information from callees to callers,
// and for global live analysis.
//
// ====-------------------------------------------------------====

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include <linux/capability.h>
#include <map>
#include <array>

using namespace llvm;

#define TARGET_FUNC "priv_raise"
#define CAP_TOTALNUM (CAP_LAST_CAP + 1)

namespace {
  // PrivAnalysis structure
  struct PrivAnalysis : public ModulePass {
    static char ID;
    // Data structure for priv_lower capabilities in each function
    // Maps from InstCalls to -> Array of Capabilities
    std::map<Function *, std::array<bool, CAP_TOTALNUM> >CAPTable;

    PrivAnalysis() : ModulePass(ID) {}

    //
    // Do initialization
    //
    virtual bool doInitialization(Module &M){
      // Init data structure
      // TODO:

      return false;
    }

    //
    // Retrieve all capabilities from params of function call
    //
    void RetrieveAllCAP(CallInst *CI, std::array<bool, CAP_TOTALNUM> &CAParray){
      int numArgs = (int) CI->getNumArgOperands();

      // Note: Skip the first param of priv_lower for it's num of args
      for (int i = 1; i < numArgs; i ++){
        // retrieve integer value
        Value *v = CI->getArgOperand(i);
        ConstantInt *I = dyn_cast<ConstantInt>(v);
        unsigned int iarg = I->getZExtValue();

        // Add it to the array
        CAParray[iarg] = 1;
      }
    }

    //
    // Get the function where the CallInst is in, add to map
    //
    void AddFuncToMap(Function *tf, std::array<bool, CAP_TOTALNUM> CAParray){
      std::map<Function *, std::array<bool, CAP_TOTALNUM> >&pCAPTable
        = this->CAPTable;

      // DEBUG
      errs() << "Parent of the instruction is " << tf->getName() << "\n";

      // If new, add to map, else, Union the two arrays
      if (pCAPTable.find(tf) == pCAPTable.end() ){
        pCAPTable[tf] = CAParray;
      }
      else {
        for (int i = 0; i < CAP_TOTALNUM; ++ i){
          pCAPTable[tf][i] |= CAParray[i];
        }
      }
    }

    //
    // Run on Module start
    //
    virtual bool runOnModule(Module &M){
      Function *F = M.getFunction(TARGET_FUNC);

      // Protector: didn't find any function TARGET_FUNC
      if (F == NULL){
        errs() << "Didn't find function " << TARGET_FUNC << "\n";
        return false;
      }

      // Find all user instructions of function in the module
      for (Value::user_iterator UI = F->user_begin(), UE = F->user_end();
           UI != UE;
           ++UI){
        // If it's a call Inst calling the targeted function
        CallInst *CI = dyn_cast<CallInst>(*UI);
        if (CI == NULL || CI->getCalledFunction() != F){
          continue;
        }

        // Retrieve all capabilities from params of function call
        std::array<bool, CAP_TOTALNUM>CAParray = {0};
        RetrieveAllCAP (CI, CAParray);

        // Get the function where the Instr is in
        // Add CAP to Map (Function* => array of CAPs)
        Function *tf = CI->getParent()->getParent();
        AddFuncToMap(tf, CAParray);

      } // for (Value::use_iterator

      // DEBUG purpose: dump map table
      // ---------------------//
      for (auto mi = CAPTable.begin(), me = CAPTable.end(); 
           mi != me;
           ++ mi){
        errs() << mi->first->getName() << ":\n";
        for (auto ai = mi->second.begin(), ae = mi->second.end();
             ai != ae;
             ++ ai){
          errs() << *ai << "\t";
        }
        errs() << "\n";
      }
      errs() << "\n";
      // ----------------------//

      return false;
    }  // virtual bool runOnModule

    //
    // getAnalysisUsage function
    // preserve all analyses
    //
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }

  }; // endof struct PrivAnalysis
}

char PrivAnalysis::ID = 0;
static RegisterPass<PrivAnalysis> X("PrivAnalysis", "Local privilege analysis pass in each function", true, true);
