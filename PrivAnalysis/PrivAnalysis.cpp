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

#define TARGET_FUNC "priv_lower"
#define CAP_SIZE (CAP_LAST_CAP + 1)

namespace {
  // Data structure for priv_lower capabilities in each function
  // Maps from InstCalls to -> Array of Capabilities
  std::unordered_map<llvm::Function, std::array>CAPTable;

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
      Function *F = M.getFunction (StringRef (TARGET_FUNC));

      // protector
      assert (F != NULL && "Cannot find target function!");
      // Find all users of function in the module
      for (User *U : F->users ()){

      	// If it's a call Inst calling the targeted function
	if (CallInst *CI = dyn_cast<CallInst>(U)){
      	   if  (CI->getCalledFunction () == F) {

      	    fprintf (stderr, "function is %p\n", F);
      	    fprintf (stderr, "get function is %p\n", CI->getCalledFunction ());

	    // Retrieve all capabilities from params of function call
	    // Note: Skip the first param of priv_lower for it's num of args
	    int numArgs = (int) CI->getNumArgOperands ();
	    for (int i = 1; i < numArgs; i ++){
	      // retrieve integer value
	      Value *v = CI->getArgOperand (i);
	      APInt *vi = dyn_cast<ConstantInt>(v)->getValue();
	      unsigned val = (unsigned) vi->getZExtValue ();
	      
	      // Add it to the array

	    }

	    // Get the function where the Instr is in
	    Function *tf = CI->getParent ()->getParent ();

	    // Add to map



	   } // if (CI->getCalledFunction() == F)
	} // if (CallInst *CI = dyn_cast<CallInst>(U))

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
