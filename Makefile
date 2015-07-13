#===--------- Makefile - Common make rules for LLVM --------*- Makefile -*--===#
#
#                        The Privilege Analysis Project
#
# Author: Kevin Hu <hxy9243#gmail.com>
#
#===------------------------------------------------------------------------===#


include Makefile.common

DSA_BUILD = /home/kevin/LocalWorkspace/DSA/build/projects/poolalloc/Release+Asserts/lib
DSA_LIB   = $(DSA_BUILD)/LLVMDataStructure.a

SRC      = ADT.cpp FindExternNodes.cpp LocalAnalysis.cpp PropagateAnalysis.cpp \
           DynCount.cpp  GlobalLiveAnalysis.cpp  PrivRemoveInsert.cpp  SplitBB.cpp \
           DSAExternAnalysis.cpp

OBJ      = $(SRC:.cpp=.o)


all: LLVMPrivAnalysis.so

LLVMPrivAnalysis.so:  $(OBJ) $(DSA_LIB)
	$(CXX) $(LDFLAGS) -o $@ $^ 

.cpp.o:
	$(CXX) $(CPPFLAGS) -o $@ $^

clean:
	-rm -f *.o *.so
