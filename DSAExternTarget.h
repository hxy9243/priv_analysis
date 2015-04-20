// ====------------  DSAExternTarget.h ----------*- C++ -*---====
//
// Find information about call sites from DSA analysis
//
// callsToExternNode will contain calls to calls external node
// in the LLVM callgraph, which is complete in DSA
//
// ====-------------------------------------------------------====


#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Analysis/CallGraph.h"

#include "LocalAnalysis.h"
#include "dsa/DataStructure.h"
#include "dsa/DSGraph.h"
#include "dsa/CallTargets.h"

#include <vector>


namespace llvm {
namespace dsaexterntarget {

using namespace dsa;

struct DSAExternTarget : public ModulePass
{
public:
    static char ID;

    DSAExternTarget();

    typedef map<Function*, vector<Function*> > FunctionMap;

    // If a callsite to callsExternNode is complete, record it here
    // with all its callees
    FunctionMap callsToExternNode;
    
    void getAnalysisUsage(AnalysisUsage &AU) const;

    virtual bool doInitialization(Module &M);

    virtual bool runOnModule(Module &M);

    void print(raw_ostream &O, const Module *M) const;
private:

};


} // namespace
} // namespace
