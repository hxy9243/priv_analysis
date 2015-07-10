// ====--------------  LocalAnalysis.cpp ---------*- C++ -*---====
//
// Local analysis of priv_lower calls in Function blocks.
// Find all the priv_lower calls inside each of the functions.
//
// ====-------------------------------------------------------====


#include "ADT.h"
#include "LocalAnalysis.h"
#include "SplitBB.h"

#include <linux/capability.h>
#include <map>
#include <array>

using namespace llvm;
using namespace llvm::privAnalysis;
using namespace llvm::splitBB;
using namespace llvm::localAnalysis;


// Constructor
LocalAnalysis::LocalAnalysis() : ModulePass(ID) { }


// Do initialization
// param: Module
bool LocalAnalysis::doInitialization(Module &M)
{
    return false;
}


// RetrieveAllCAP
// Retrieve all capabilities from params of function call
// param: CI - call instruction to retrieve from
//        CAParray - the array of capability to save to
void LocalAnalysis::RetrieveAllCAP(CallInst *CI, CAPArray_t &CAPArray)
{
    int numArgs = (int) CI->getNumArgOperands();
    assert(CI != NULL && "The CallInst is NULL!\n");

    // Note: Skip the first param of priv_lower for it's num of args
    for (int i = 1; i < numArgs; ++i) {
        // retrieve integer value
        Value *v = CI->getArgOperand(i);
        ConstantInt *I = dyn_cast<ConstantInt>(v);
        unsigned int iarg = I->getZExtValue();

        // Add it to the array
        CAPArray |= 1 << iarg;
    }
}


// Run on Module start
// param: Module
bool LocalAnalysis::runOnModule(Module &M)
{
    // retrieve all data for later use
    SplitBB &SB = getAnalysis<SplitBB>();
    BBFuncTable = SB.BBFuncTable;
    ExtraJMPBB = SB.ExtraJMPBB;
  
    // find all users of targeted function
    Function *F = M.getFunction(PRIVRAISE);

    // Protector: didn't find any function TARGET_FUNC
    assert(F && "Didn't find function PRIV_RAISE function");

    // Find all user instructions of function in the module
    for (Value::user_iterator UI = F->user_begin(), UE = F->user_end();
         UI != UE; ++UI) {

        // If it's a call Inst calling the targeted function
        CallInst *CI = dyn_cast<CallInst>(*UI);
        if (CI == NULL || CI->getCalledFunction() != F) {
            continue;
        }

        // Retrieve all capabilities from params of function call
        CAPArray_t CAParray = 0;
        RetrieveAllCAP(CI, CAParray);

        // Get the function where the Instr is in
        // Add CAP to Map (Function* => array of CAPs)
        // and Map (BB * => array of CAPs)
        AddToBBCAPTable(BBCAPTable, CI->getParent(), CAParray);
        AddToFuncCAPTable(FuncCAPTable, CI->getParent()->getParent(), CAParray);
    }

    return false;
}


// getAnalysisUsage function
// preserve all analyses
// param: AU
void LocalAnalysis::getAnalysisUsage(AnalysisUsage &AU) const 
{
    // TODO: Separate SplitBB as an individual Transformation Pass?
    AU.addRequired<SplitBB>();

    AU.setPreservesAll();
}


// Print out information for debugging purposes
void LocalAnalysis::print(raw_ostream &O, const Module *M) const
{
    dumpCAPTable(FuncCAPTable);
}


// Pass registry
char LocalAnalysis::ID = 0;
static RegisterPass<LocalAnalysis> A("LocalAnalysis", "Local Privilege Analysis", 
                                     true, /* CFG only? */
                                     true /* Analysis pass? */);
