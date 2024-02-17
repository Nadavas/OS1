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

//global data
extern int last_job_id;	
extern string prev_path;	// string describing the last path 
extern string cur_path;	// string describing the current path 
extern std::map<int, Job, std::less<int>> jobs_list;

// Job class - to represent job data type
class Job{
	public:
		pid_t pid;
		string cmd;
		bool is_stopped;
		time_t begining_time;
		Job();
		Job(int pid, string cmd, bool is_stopped, time_t begining_time);
};

class Fg_job{
	public:
		int fg_pid;	// holds the process id of the job in fg, init to -1
		std:: string fg_cmd;
		int fg_jid;	// holds holds the job id of the job in fg, init to -1
		Fg_job() : fg_pid(-1), fg_cmd(""), fg_jid(-1) {};
		Fg_job(int fg_pid,std::string fg_cmd,int fg_jid);
};

void BgCmd(string args[MAX_ARG], int args_count, string command);
void ExeExternal(string args[MAX_ARG], int args_count, string command);
int ExeCmd(string args[MAX_ARG], int args_count, string cmd);
#endif

