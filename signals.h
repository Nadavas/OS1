#ifndef _SIGS_H
#define _SIGS_H
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <string>
#include "commands.h"
#define MAX_LINE_SIZE 80
#define MAXARGS 20
//using namespace std;

void sig_handler(int sig_number);


#endif

