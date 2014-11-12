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

  // Copy CAPTable keys from src to dest
  // After this operation, dest would have all keys src
  // has, with empty CAPArrays mapping to each keys
  // param: dest - dest CAPTable
  //        src  - src CAPTable
  void CopyTableKeys(CAPTable_t &dest, CAPTable_t &src){
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


  // dump CAPTable for Debugging purpose
  void dumpCAPTable(CAPTable_t &CT){

    // iterate through captable, a map from func to array
    for (auto mi = CT.begin(), me = CT.end();
         mi != me;
         ++ mi){
      errs() << mi->first->getName() << ":\n";

      // iterate through cap array
      for (auto ai = mi->second.begin(), ae = mi->second.end();
           ai != ae;
           ++ ai){
        errs() << *ai << "\t";
      }
      errs() << "\n";
    }
    errs() << "\n";

  }


} // namespace privAnalysis
} // namepsace llvm
