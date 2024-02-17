#ifndef _COMMANDS_H
#define _COMMANDS_H
#include <unistd.h> 
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <cstring>
#include <map>
#include <algorithm>    // std::find
#include <cerrno>
#include <cstdio>
#include <iostream>

#define MAX_LINE_SIZE 80
#define MAX_ARG 20
using namespace std;
using string = std::string;

// Job class - to represent job data type
class Job{
	public:
		pid_t pid;
		string cmd;
		bool is_stopped;
		time_t begining_time;
		Job();
		Job(int pid, string cmd, bool is_stopped, time_t begining_time);
}

class Fg_job{
	public:
		int fg_pid;	// holds the process id of the job in fg, init to -1
		std:: string fg_cmd;
		int fg_jid;	// holds holds the job id of the job in fg, init to -1
		Fg_job() : fg_pid(-1), fg_cmd(""), fg_jid(-1) {};
		Fg_job(int fg_pid,std::string fg_cmd,int fg_jid);
}

typedef enum { FALSE , TRUE } bool;
void BgCmd(string args[MAX_ARG], int args_count, string command);
void ExeExternal(string args[MAX_ARG], int args_count, string command)
void ExeExternal(char *args[MAX_ARG], char* cmdString);
#endif

