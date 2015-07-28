// ====------------  DSAExternAnalysis.h ----------*- C++ -*---====
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

typedef std::unordered_map<CallSite*, std::vector<Function*> > CallSiteFunMap_t;
typedef std::unordered_map<Function*, std::vector<Function*> > FunctionMap_t;
typedef std::unordered_map<Instruction*, std::vector<Function*> > InstrFunMap_t;

struct DSAExternAnalysis : public ModulePass
{
public:
    static char ID;

    DSAExternAnalysis();

    // If a callsite to callsExternNode is complete, record it here
    // with all its callees
    CallSiteFunMap_t callsToExternNode;
    
    // FunctionMap records additional info for the callgraph, mapping callers to
    // callees
    FunctionMap_t callgraphMap;

    // instFunMap records instruction to its possible called functions
    InstrFunMap_t instFunMap;
    
    void getAnalysisUsage(AnalysisUsage &AU) const;

    virtual bool doInitialization(Module &M);

    virtual bool runOnModule(Module &M);

    void print(raw_ostream &O, const Module *M) const;

private:
    // Find out all indirect callsites, save to callsToExternNode
    void findAllCallSites(CallTargetFinder<TDDataStructures> &CTF);

    // Save information to corresponding data structures
    void saveToMappings(CallTargetFinder<TDDataStructures> &CTF);
};


} // namespace
} // namespace
