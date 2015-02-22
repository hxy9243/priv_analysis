// ====------------------- DynCount.cpp ---------*- C++ -*----====
//
// Dynamically count the lines of IR for each privilege set
//
// ====-------------------------------------------------------====

#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

#include <map>

#include "ADT.h"
#include "GlobalLiveAnalysis.h"


using namespace llvm::privAnalysis;

namespace llvm {
namespace dynCount {


struct DynCount : public ModulePass
{
public:
    static char ID;

    typedef std::map<CAPArray_t, int> CAPMap;

    DynCount();

    // Initialization
    virtual bool doInitialization(Module &M);

    // Run on Module Start
    virtual bool runOnModule(Module &M);

    // Preserve analysis usage
    void getAnalysisUsage(AnalysisUsage &AU) const;

    // Print out information for debugging purposes
    void print(raw_ostream &O, const Module *M) const;
private:
    
    Function *
};



} // namespace dynCount
} // namespace namespace
