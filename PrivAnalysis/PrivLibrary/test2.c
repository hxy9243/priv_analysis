/*
 * Yet another test case for priv lib
 * Testing if you still capable of chmod file after
 * lowering CAP_FOWNER capability
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "priv.h"

int main (int argc, char **argv){

  int status;
  
  status = chmod ("foo.txt", S_IRUSR|S_IRGRP|S_IROTH);

  printf ("Setting chmod before lowering priv: %s\n", status?"Fail":"Success");

  status = priv_lower (1, CAP_FOWNER);

  printf ("Setting priv %s\n", status?"Fail":"Success");

  status = chmod ("foo.txt", S_IWUSR|S_IWGRP|S_IWOTH);

  printf ("Setting chmod after lowering priv: %s\n", status?"Fail":"Success");
  

  return 0;
}