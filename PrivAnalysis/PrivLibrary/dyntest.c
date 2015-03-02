/*
 * Dynamic Counting library for LLVM Privilege Analysis pass
 *
 * Testing code for dyncount library
 * 
 * author: Kevin Hu <hxy9243@gmail.com>
 *
 */


#include "dyncount.h"
#include "stdio.h"


int main() 
{

    initCount();

    int i;

    for (i = 0; i < 10; ++i) {

        addCount(3, (uint64_t)(0x00001000));

        addCount(3, (uint64_t)(0x00001A0B));

    }

    addCount(4, (uint64_t)(0x00001000));


    reportCount();
}
