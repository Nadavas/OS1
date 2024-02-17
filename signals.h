#ifndef _SIGS_H
#define _SIGS_H
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#define MAX_LINE_SIZE 80
#define MAXARGS 20
using namespace std;
using string = std::string;

void update_job_list();
int break_cmd_to_args(string line, std::string args[MAXARGS], std::string delimiters=" \t\n");
bool check_if_built_in_cmd(std::string command);
void fg_clean();
bool is_number(std::string& str);
int search_job(std::string &arg);
void fg_replace(pid_t pid, std::string command, int jid=-1);
int find_stopped_job();
bool insert_job(pid_t pID, std::string cmd, bool is_stopped=false, int jid=-1);
void sig_handler(int sig_number);


#endif

