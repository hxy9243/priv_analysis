// ====------------------- DynCount.cpp ---------*- C++ -*----====
//
// Dynamically count the lines of IR for each privilege set
//
// ====-------------------------------------------------------====

#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"


#include <map>
#include <cstdint>


#include "ADT.h"
#include "LocalAnalysis.h"
#include "PropagateAnalysis.h"
#include "GlobalLiveAnalysis.h"


// The dynamic counting library API:
// void initCount();
// void addCount(int LOC, uint64_t CAPArray);
// void reportCount();
#define INIT_COUNT_FUNC "initCount"     
#define ADD_COUNT_FUNC "addCount"       
#define REPORT_COUNT_FUNC "reportCount" 


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
    Function *getInitCountFunc(Module &M);

    Function *getAddCountFunc(Module &M);

    Function *getAtExitFunc(Module &M);

    void getAddCountArgs(std::vector<Value *>& Args, unsigned int LOC, 
                         const CAPArray_t &CAPArray);

    template<typename T>
    bool findVector(std::vector<T> V, T elem);
};

// template<typename T>
// bool findVector(std::vector<T> V, T elem);

    

} // namespace dynCount
} // namespace namespace
