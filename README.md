Privilege Analysis LLVM pass
====

# Introduction

This is an LLVM pass for inter-procedural whole program analysis of Linux program
capabilities. Currently it requires that your programs properly instrumented with
privilege library calls in ```PrivLibrary``` directory.

See [here](http://linux.die.net/man/7/capabilities) for Linux capabilities.

This pass is able to infer from your instrumentations where certain capabilities
are live in the program, and ergo where in the program you can savely remove
capabilities. Based on this information, it could automatically insert calls
from ```PrivLibrary``` to remove capabilities, enforcing the Principle of Minimal
Capabilities.


# Installation

So far compiling requires putting code inside the LLVM source tree. Standalone
compilation coming on the way.

1. Go to ```${LLVM_SRC_DIR}/lib/Transform/```, backup your ```LLVMBuild.txt```,
    ```CMakeLists.txt``` and ```Makefile``` files.
2. Clone this repo.
3. Copy all its contents to the ```lib/Transform``` directory of the LLVM source tree.
    Use the following script to finish the whole process. Before that, make sure you
    have all your Makefile related files backed up.
    
    ```
    git clone https://github.com/hxy9243/priv_analysis.git -b printGlobal
    
    cp -R priv_analysis/* .
    ```
4. In your build directory of LLVM, go to ```${LLVM_BUILD_DIR}/lib/Transform/```, 
    edit the Makefile, add the following to the Makefile field ```PARALLEL_DIRS:```
    ```PrivAnalysis ExternFunCall```.
5. Type ```make``` to make them all.
6. The library shall now be compiled and moved in your
    ```${LLVM_BUILD_DIR}/${Release_Or_Debug}/lib/``` directory.
    ```${Release_Or_Debug}``` could be ```Release```, ```Debug```, ```Release+Asserts```
    or ```Debug+Asserts```, depending on your LLVM compile options.

    Right now you should to able to see the ```LLVMPrivAnalysis.so```
    and ```LLVMExternFunCall.so``` shared libraries in the lib folder.


# Privilege Bracketing

To use the pass, your code needs to be properly instrumented with privilege bracketing.
That is, all privileged external function calls need to be bracketed. Privilege will be
raised in the EFFECTIVE set right before the call, and lowered right after the call.

This repo provides with a simple interface to Linux capability library for privilege bracketing.

The related functions are:

* ```int priv_lowerall()```: Lower all privilges in the EFFECTIVE set.

* ```int priv_raise(int count, ...)```: Raise one to a few capabilities in the EFFECTIVE set.

    parameter count -- the number of capabilities to operate on,

    parameter ...   -- the name of the capabilities.

    See [here](http://linux.die.net/man/7/capabilities) for more on capabilities.

* ```int priv_lower(int count, ...)```: Lower one to a few capabilites in the EFFECTIVE set.
parameter same as ```priv_raise```.

* ```int priv_remove(int count, ...)```: Remove one to a few capabilities from the PERMITTED set,
making them never usable in the same process. Parameter same as ```priv_raise```. This is to be
inserted automatically by the pass transformation.

The shared library pass presumes the following for the pass to be properly working:

* All external function calls are properly bracketed with ```priv_raise``` and ```priv_lower```.

    For example:
    
    ```
    priv_raise(1, CAP_SETUID);
    
    setuid(some_other_uid_that_requires_capability_to_change_into);
    
    priv_lower(1, CAP_SETUID);

    ....

    priv_raise(1, CAP_CHROOT);
    
    chroot("some_path");
    
    priv_lower(1, CAP_CHROOT);
    ```

* Paired ```priv_raise``` and ```priv_lower``` must be in the same Basic Block. There are no
control flow related instructions inside the bracketing.

    Therefore the following code:

    ```
    if (setuid(id)) {
        ...
    }
    ```

    needs to be transformed into the following for correct and fine-grained bracketing.


    ```
    priv_raise(1, CAP_SETUID);
    int setuid\_return = setuid(id);
    priv\_lower(1, CAP\_SETUID);
    
    if (setuid\_return) {

        ...
        
    }
    ```

# Shared Library Usage

Use opt inside the build directory to use the shared library.
(Known issue: using opt built from other sources, e.g. system-wise installed
opt from package manager, may result in errors.)

Use the compiled library with opt:

```${LLVM_BUILD_DIR}/${Release_Or_Debug}/bin/opt -load ${LLVM_BUILD_DIR}/${Release_Or_Debug}/lib/${SHARED_LIBRARY} -${PASS_NAME} ${OPTION} bitcode.bc > bitcode.opt.bc ```

```SHARED_LIBRARY``` is the shared library we obtained, which is
```LLVMPrivAnalysis.so``` or ```LLVMExternFunCall.so```,
```PASS_NAME``` is the name of the pass inside each shared library, explained in detail below.

So for example, if you want to insert remove capability instructions, run with the following command,
and get transformed bitcode in bitcode.opt.bc:

```${LLVM_BUILD_DIR}/Release+Asserts/bin/opt -load ${LLVM_BUILD_DIR}/Release+Asserts/lib/LLVMPrivAnalysis.so -PrivRemoveInsert bitcode.bc > bitcode.opt.bc```


## Shared Library LLVMExternFunCall.so

* __ExternFunCall pass__: Print the external function calls from the input bitcode source,
e.g. the standard C library, or external library whose source is not in the bitcode file.

    Run with: ```--analyze``` option to see the output.
    
    This is an assist pass for figuring out which function calls needs privilege bracketing.

## Shared Library LLVMPrivAnalysis.so

* __SplitBB pass__: Internal pass for splitting the Basic Blocks for easier analysis. 

* __LocalAnalysis pass__: Internal pass for inferring bracketed privileged calls.
Depends on __SplitBB__ pass.

* __PropagateAnalysis pass__: Propagate information along in Call Graph.
Depends on __LocalAnalysis__ pass. 

* __GlobalLiveAnalysis pass__: Infer live information depending on Call Graph and Control Flow
Graphs from all functions. Depends on __Propagate Analysis__ pass and __UnifyExitNode__ pass
(built pass in LLVM).

    Run with ```--analyze``` to see the unique set of capabilities for the whole source program.

* __PrivRemoveInsert pass__: Insert ```priv_remove``` calls to proper locations where
capabilities are no more live. Depends on __GlobalLiveAnalysis__.


# LICENSE

[GPLv3 License](http://www.gnu.org/copyleft/gpl.html)

The GNU General Public License is a free, copyleft license for software and other kinds of works.

