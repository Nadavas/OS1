// signals.cpp
// contains signal handler funtions
// contains the function/s that set the signal handlers

/*******************************************/
/* Name: handler_cntlc
   Synopsis: handle the Control-C */
#include "signals.h"
#include "commands.h"
#include <sys/wait.h>
using string = std::string;
using namespace std;
using string = std::string;
// linking the globals form smash.cpp
extern Fg_job fg_job;
#define MAX_LINE_SIZE 80
#define MAXARGS 20
///////////////////////////////////////////
//////////////////////////////////////////
	// HELP FUNCTIONS SECTION //
void update_job_list(){
	int stat;
	pid_t child_pid;
	auto it = jobs_list.begin();
	while (it!=jobs_list.end()){
		stat=0;
		child_pid = waitpid((it->second).pid,&stat, WNOHANG|WUNTRACED|WCONTINUED);
		if(child_pid==-1){
			perror("smash error: waitpid failed");
			return;
		}
		if(WIFSTOPPED(stat)){
			(it->second).is_stopped = true;
		}
		else if(WIFCONTINUED(stat)){
			(it->second).is_stopped = false;
		}
		else if(child_pid == (it->second).pid){
			//process not running, erase it
			jobs_list.erase(it);
		}
	it++;
	}
	if (jobs_list.empty()){
		last_job_id=0;
	}
	else{
		last_job_id=jobs_list.rbegin()->first;
	}

}

int break_cmd_to_args(string line, string(&args)[MAXARGS], string delimiters){

	char* iter = strtok(const_cast<char*>(line.c_str()), const_cast<char*>(delimiters.c_str()));
	int i = 0, num_arg = 0;
	if (iter == NULL)
		return 0;
	args[0] = iter; // arg[0] is the cmd
	for (i = 1; i < MAX_ARG; i++)
	{
		// arg[i] is the arguments by order
		iter = strtok(NULL, const_cast<char*>(delimiters.c_str()));
		if (iter != NULL) {
			args[i] = iter;
			num_arg++;  // number of arguments of cmd
		}

	}
	return num_arg;
}

bool check_if_built_in_cmd(string command){
	const std::string BUILT_IN_CMD[] = 
	{ "showpid" ,"pwd","cd","jobs","kill","fg","bg","quit","diff" };
	if (command[command.length()-1]=='&'){
		command.pop_back();		//remove &
		if(command.empty()){
			return false;
		}
	}
	return (std::find(std::begin(BUILT_IN_CMD), std::end(BUILT_IN_CMD), command) != std::end(BUILT_IN_CMD));
}

void fg_clean(){
	fg_job.fg_pid=-1;
	fg_job.fg_cmd.clear();
	fg_job.fg_jid=-1;
}

bool is_number(std::string& str){
    for (char const &c : str) {
        // using the std::isdigit() function
        if (std::isdigit(c) == 0)
          return false;
    }
    return true;
}

int search_job(string &arg){
	int jid = stoi(arg);	//convert to int
	if(jobs_list.find(jid)==jobs_list.end())
		return -1;
	return jid;
}

void fg_replace(pid_t pid, string command, int jid){
	fg_job.fg_pid=pid;
	fg_job.fg_cmd=command;
	fg_job.fg_jid=jid;
}

int find_stopped_job(){
	bool flag;
	int highest_jid_stopped;
	auto it=jobs_list.begin();
	while(it!=jobs_list.end()){
		if(it->second.is_stopped){
			highest_jid_stopped = it->first;
			flag=true;
		}
		it++;
	}
	if (!flag)
		return -1;
	return highest_jid_stopped;
}

bool insert_job(pid_t pID, string cmd, bool is_stopped, int jid){
	update_job_list();
	time_t present_time(time(NULL));
	if(present_time==-1){
		perror("smash error: time failed");
		return false;
	}
	Job newjob(pID,cmd,is_stopped,present_time);
	int fin_job = last_job_id+1;
	if(jid!=-1){
		fin_job = jid;
	}
	bool res = jobs_list.insert(std::pair<int,Job>(fin_job,newjob)).second;
	return res;
}

bool fg_empty(){
	if((fg_job.fg_pid!=-1) && (!fg_job.fg_cmd.empty())){
		return true;
	}
	return false;
}

void sig_handler(int sig_number){
	if(sig_number==SIGINT){
		cout << "smash: caught ctrl-C" << endl;
		if(!fg_empty()){		//checking if there is a job on fg
			if(kill(fg_job.fg_pid,SIGKILL)==-1){
				perror("smash error: kill failed");
				return;
			}
			cout << "smash: process " << fg_job.fg_pid << " was killed" << endl;
			fg_clean();
		}
		else{
			printf("smash > ");
		}
	}
	else if(sig_number==SIGSTOP){
		cout << "smash: caught ctrl-Z" << endl;
		if(!fg_empty()){		//checking if there is a job on fg
			if(insert_job(fg_job.fg_pid,fg_job.fg_cmd,true,fg_job.fg_jid)){
				if(kill(fg_job.fg_pid,SIGKILL)==-1){
					perror("smash error: kill failed");
					return;
				}
				else{
					cout << "smash: process " << fg_job.fg_pid << " was stopped" << endl;
					fg_clean();
				}
			}
			else{
				perror("smash error: insert_job failed\n");
			}
		}
		else{
			printf("smash > ");
		}	

	}	
}


