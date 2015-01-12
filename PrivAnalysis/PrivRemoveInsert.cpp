// ====----------  PrivRemoveInsert.h ------------*- C++ -*---====
//
// Based on the information from GlobalLiveAnalysis, insert 
// privRemove calls at the end of BasicBlocks, to remove 
// unnecessary privileges. 
//
// ====-------------------------------------------------------====

#include "PrivRemoveInsert.h"
#include "GlobalLiveAnalysis.h"

using namespace llvm;
using namespace llvm::globalLiveAnalysis;
using namespace llvm::privremoveinsert;


// PrivRemoveInsert constructor
PrivRemoveInsert::PrivRemoveInsert() : ModulePass(ID)
{

}


// Initialization
bool PrivRemoveInsert::doInitialization(Module &M)
{

    return false;
}


// Preserve analysis usage
void PrivRemoveInsert::getAnalysisUsage(AnalysisUsage &AU) const
{



}


// Run on Module
bool PrivRemoveInsert::runOnModule(Module &M)
{



    return true;
}


// register pass
char PrivRemoveInsert::ID = 0;
static RegisterPass<PrivRemoveInsert> I("PrivRemoveInsert", "Insert PrivRemove calls", true, true);

