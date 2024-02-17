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
#define MAXARGS 20
using namespace std;
using string = std::string;
#define MAX_ARG 20


//global data
extern int last_job_id;	
extern string prev_path;	// string describing the last path 
extern string cur_path;	// string describing the current path 


// Job class - to represent job data type
class Job{
	public:
		pid_t pid;
		string cmd;
		bool is_stopped;
		time_t begining_time;
		Job();
		Job(pid_t pidx, std::string cmdx, bool is_stoppedx, time_t beginning_timex);
class Fg_job{
	public:
		int fg_pid;	// holds the process id of the job in fg, init to -1
		std:: string fg_cmd;
		int fg_jid;	// holds holds the job id of the job in fg, init to -1
		Fg_job() : fg_pid(-1), fg_cmd(""), fg_jid(-1) {};
		Fg_job(int fg_pid,std::string fg_cmd,int fg_jid);
};

//extern std::map<int, Job, std::less<int>> jobs_list;
void BgCmd(string args[MAX_ARG], int args_count, string command);
void ExeExternal(string args[MAX_ARG], int args_count, string command);
int ExeCmd(string args[MAX_ARG], int args_count, string cmd);
void update_job_list();
int break_cmd_to_args(string line, string(&args)[MAXARGS], string delimiters=" \t\n");
bool check_if_built_in_cmd(std::string command);
void fg_clean();
bool is_number(std::string& str);
int search_job(std::string &arg);
void fg_replace(pid_t pid, std::string command, int jid=-1);
int find_stopped_job();
bool insert_job(pid_t pID, std::string cmd, bool is_stopped=false, int jid=-1);
bool fg_empty();
#endif

