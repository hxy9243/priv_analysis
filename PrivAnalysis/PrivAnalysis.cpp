// ====---------------  PrivAnalysis.cpp --------------------====
//
// Local analysis of priv_lower calls in Function blocks.
// Find all the priv_lower calls inside each of the functions,
// and propagate all the information from callees to callers,
// and for global live analysis.
//
// ====-------------------------------------------------------====

#include <stdio.h>

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include <linux/capability.h>

#include <unordered_map>
using namespace llvm;

#define TARGET_FUNC "priv_raise"
#define CAP_SIZE (CAP_LAST_CAP + 1)

namespace {
  // Data structure for priv_lower capabilities in each function
  // Maps from InstCalls to -> Array of Capabilities
  //std::unordered_map<llvm::Function, std::array>CAPTable;

  // PrivAnalysis structure
  struct PrivAnalysis : public ModulePass {
    static char ID;
    PrivAnalysis() : ModulePass (ID) {}

    // Do initialization
    virtual bool doInitialization(Module &M){
      // Init data structure
      // TODO:

      return false;
    }

    // Run on Module start
    virtual bool runOnModule (Module &M){
      Function *F = M.getFunction (TARGET_FUNC);

      // protector
      // TODO: return instead of assert
      // assert (F != NULL && "Cannot find target function!");

      // Find all users of function in the module
      for (Value::user_iterator UI = F->user_begin(), UE = F->user_end (); UI != UE; ++UI){
      	// If it's a call Inst calling the targeted function
	CallInst *CI = dyn_cast<CallInst>(*UI);
	if (CI != NULL && CI->getCalledFunction() == F){
	  
	  // errs () << *CI << "\n";
	  // errs () << CI->getParent()->getParent()->getName();
	  
	  // Retrieve all capabilities from params of function call
	  // Note: Skip the first param of priv_lower for it's num of args
	  int numArgs = (int) CI->getNumArgOperands ();

	  errs () << "num of args is " << numArgs << "\n";
	  for (int i = 0; i < numArgs; i ++){
	    // retrieve integer value
	    Value *v = CI->getArgOperand (i);
	    // problem here
	    // ConstantInt *I = dyn_cast<ConstantInt>(*v);
	      
	    errs () << "args is " << *v << "\n";

	    // Add it to the array
	    // TODO

	  }

	  // Get the function where the Instr is in
	  Function *tf = CI->getParent ()->getParent ();
	  errs () << "Parent of the instruction is " << tf->getName() << "\n";


	  // Add to map
	  // TODO


	
	} // if (CI != NULL

      } // for ()

      return false;
    }  // virtual bool runOnModule


    // preserve all analyses
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }

  }; // endof struct PrivAnalysis
}

char PrivAnalysis::ID = 0;
static RegisterPass<PrivAnalysis> X("PrivAnalysis", "Local privilege analysis pass in each function", true, true);
