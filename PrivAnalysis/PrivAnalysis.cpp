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

#incude <unordered_map>

#define TARGET_FUNC "priv_lower"

using namespace llvm;

namespace {
  // Data structure for priv_lower capabilities in each function
  // Maps function name -> Array of capabilities
  unordered_map <std::string, int[MAX_CAP_NUM]>PrivTable;

  // PrivAnalysis structure
  struct PrivAnalysis : public FunctionPass {
    static char ID;
    PrivAnalysis() : ModulePass (ID) {}

    // Do initialization
    virtual bool doInitialization(Module &M){
      // Init data structure
      // TODO:
    }

    // Run on Each Function
    virtual bool runOnModule (Module &M){

      // find function object
      Function F = getfunction (TARGET_FUNC);

      // find all ocurring uses
      
      
      

      return false;
    }
  }; // endof struct PrivAnalysis
}

char PrivAnalysis::ID = 0;
static RegisterPass<PrivAnalysis> X("PrivAnalysis", "Local privilege analysis pass in each function");
