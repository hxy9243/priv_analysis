/*
 * Dynamic Counting library for LLVM Privilege Analysis pass
 * 
 * author: Kevin Hu <hxy9243@gmail.com>
 *
 */


#ifndef __DYNCOUNT_H__
#define __DYNCOUNT_H__


#if defined(__cplusplus)
extern "C" {
#endif


#include <stdint.h>


/* init the counting data structure */
int initCount();


/* add LOC to the CAPArray data structure 
 * This function is to be inserted into every Basic Block for 
 * counting the LOC of capability set of the Basic Block
 * param: LOC - the LOC of the capability set
 *        CAPArray - the capability set represented in uint64_t
 */
int addCount(int LOC, uint64_t CAPArray);


/* report the counting data structure */
int reportCount();


 
#if defined(__cplusplus)
}
#endif


#endif
