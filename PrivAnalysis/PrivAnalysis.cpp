// ====---------------  PrivAnalysis.cpp --------------------====
//
// Local analysis of priv_lower calls in Function blocks.
// Find all the priv_lower calls inside each of the functions,
// and propagate all the information from callees to callers,
// and for global live analysis.
//
// ====-------------------------------------------------------====

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include <linux/capability.h>

#include <unordered_map>

#define TARGET_FUNC "priv_lower"
#define CAP_SIZE (CAP_LAST_CAP + 1)

using namespace llvm;

namespace {
  // Data structure for priv_lower capabilities in each function
  // Maps from InstCalls to -> Array of Capabilities
  unordered_map <std::string, int[CAP_SIZE]>CAPTable;

  // PrivAnalysis structure
  struct PrivAnalysis : public FunctionPass {
    static char ID;
    PrivAnalysis() : ModulePass (ID) {}

    // Do initialization
    virtual bool doInitialization(Module &M){
      // Init data structure
      // TODO:


    }

    // Run on Module start
    virtual bool runOnModule (Module &M){
      Function *F = getFunction (StringRef (TARGET_FUNC));

      // Find all users of function in the module
      for (User *U : F->users()){
	// If it's a call Inst calling the targeted function
	if (CallInst *CI = dyn_cast<CallInst>(U) 
	    && CI->getCalledFunction () == F){

	  // Retrieve all capabilities from params of function call
	  // Note: Skip the first param of priv_lower for it's redundant
	  unsigned numArgs = CI->getNumArgOperands ();
	  for (int i = 1; i < numArgs; i ++){
	    // retrieve integer value
	    Value *v = CI->getArgOperand (i);
	    APInt *vi = dyn_cast<ConstantInt>(v)->getValue();
	    unsigned val = (unsigned) vi->getZExtValue ();


	  }

	  // Get the function where the Instr is in
	  Function *tf = CI->getParent ()->getParent ();
	  // Add to map


	} // if (CallInst *CI = dyn_cast<CallInst>(U))

      } // for (user *U : F->users ())
      return false;
    }  // virtual bool runOnModule
  }; // endof struct PrivAnalysis
}

char PrivAnalysis::ID = 0;
static RegisterPass<PrivAnalysis> X("PrivAnalysis", "Local privilege analysis pass in each function");
