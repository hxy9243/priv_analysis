// ====-----------------------  ADT.h -----------*- C++ -*---====
//
// The ADT and configuration for the PrivAnalysis
//
// ====-------------------------------------------------------====

#include "ADT.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <array>


namespace llvm{
namespace privAnalysis{


  // Add to the FuncCAPTable, if Func exists, merge the CAPArray
  // param: CAPTable - ref to the FuncCAPTable
  //        F - the function to add
  //        CAParray - the array of capability to add to FuncCAPTable
  void AddToFuncCAPTable(FuncCAPTable_t &CAPTable,
                         Function *F, 
                         CAPArray_t CAParray) {
    // If not found in map, add to map
    if (CAPTable.find(F) == CAPTable.end()) {
      CAPTable[F] = CAParray;
    }
    // else, Union the two arrays
    else {
      for (int i = 0; i < CAP_TOTALNUM; ++ i){
        CAPTable[F][i] |= CAParray[i];
      }
    }
  }


  // Add to the BBCAPTable, if BB exists, merge the CAPArray
  // param: CAPTable - ref to the BBCAPTable
  //        B - the BasicBlock to add
  //        CAParray - the array of capability to add to FuncCAPTable
  void AddToBBCAPTable(BBCAPTable_t &CAPTable,
                       BasicBlock *B, 
                       CAPArray_t CAParray) {

    // If not found in map, add to map
    if (CAPTable.find(B) == CAPTable.end() ){
      CAPTable[B] = CAParray;
    }
    // else, Union the two arrays
    else {
      for (int i = 0; i < CAP_TOTALNUM; ++ i){
        CAPTable[B][i] |= CAParray[i];
      }
    }
  }


  // Copy CAPTable keys from src to dest
  // After this operation, dest would have all keys src
  // has, with empty CAPArrays mapping to each keys
  // param: dest - dest CAPTable
  //        src  - src CAPTable
  void CopyTableKeys(FuncCAPTable_t &dest, FuncCAPTable_t &src){
    CAPArray_t emptyArray = {0};

    for(auto MI = src.begin(), ME = src.end(); MI != ME; ++ MI){
      
      Function *tf = MI->first;

      dest[tf] = emptyArray;
    }
  }

  
  // Union two arrays, save result to dest
  // param: dest - dest CAPArray
  //        src  - src CAPArray
  // return: ischanged - if the dest's value is changed
  bool UnionCAPArrays(CAPArray_t &dest, CAPArray_t &src){
    bool ischanged = false;

    for (unsigned int i = 0; i < dest.size(); ++ i){
      // changed only when dest is 0 and src is 1
      ischanged |= (~dest[i] & src[i]);
      dest[i] |= src[i];
    }
    return ischanged;
  }


  // Diff two arrays, save result
  // param: dest - dest CAPArray
  //        A - the CAPArray to subtract from
  //        B - the CAPArray to subtract
  void DiffCAPArrays(CAPArray_t &dest, CAPArray_t &A, CAPArray_t &B) {
   
    for (unsigned int i = 0; i < A.size(); ++i){
      dest[i] = A[i] & (~B[i]);

      if (~A[i] & B[i]){
        errs() << "\nBUG in Diff CAPArrays!\n\n";

      }
    }

    return;
  } 

  // dump CAPTable for Debugging purpose
  // param: CT - the CAPTable to dump
  void dumpCAPTable(FuncCAPTable_t &CT){

    // iterate through captable, a map from func to array
    for (auto mi = CT.begin(), me = CT.end();
         mi != me;
         ++ mi){
      errs() << mi->first->getName() << " Privileges:\n";

      // iterate through cap array
      for (unsigned int i = 0; i < mi->second.size(); ++ i){
        if (mi->second[i]){
          errs() << i << "\t";
        }
      }
      errs() << "\n";
    }
    errs() << "\n";

  }


} // namespace privAnalysis
} // namepsace llvm
