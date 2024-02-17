//		commands.cpp
//********************************************
#include "commands.h"
#include "signals.h"
using std::cout;
using std::endl;
using std::map;
using std::string;
#include <fcntl.h>   // For O_RDONLY
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>   // For open

//global data
int last_job_id=0;	
string prev_path = NULL;	// string describing the last path 
string cur_path = NULL;	// string describing the current path 
std::map<int, Job, std::less<int>> jobs_list;	// jobs list in ascending order based on job id
Fg_job fg_job;	//check later

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


//********************************************
// function name: ExeCmd
// Description: interperts and executes built-in commands
// Parameters: pointer to jobs, command string
// Returns: 0 - success,1 - failure
//**************************************************************************************
int ExeCmd(string args[MAX_ARG], int args_count, string cmd)
{
	bool error_in_cmd = false;
	
/*************************************************/
// Built in Commands PLEASE NOTE NOT ALL REQUIRED
// ARE IN THIS CHAIN OF IF COMMANDS. PLEASE ADD
// MORE IF STATEMENTS AS REQUIRED
/*************************************************/
	if (!strcmp(cmd.c_str(), "cd") ) 
	{
		
		if(args_count>1)	// too many arguments
			perror("smash error: cd: too many arguments\n");
		else if (args[1]=="-"){	
		// cd to prev path
				if(prev_path.empty()){	
					// in case of no previous path
					perror("smash error: cd: OLDPWD not set\n");
					error_in_cmd = true;
				}
				else if(chdir(prev_path.c_str())){
					perror("smash error: chdir failed");
					error_in_cmd = true;
				}
				else{
					// chdir success - need to update relevant paths
					string temp = cur_path;
					cur_path = prev_path;
					prev_path = temp;
				}
		}
		else{
		// cd to a specific dir
			
			if(chdir(const_cast<char*>(args[1].c_str()))){
				perror("smash error: chdir failed");
			}
			else{
				// chdir success - need to update relevant paths
				string temp = get_current_dir_name();
				if(temp.empty()){
					perror("smash error: getcwd failed");
					return -1;
				}
				// updating relevent pathways
				prev_path = cur_path;
				cur_path = temp;
			}
		}
	} 
	/*************************************************/
	else if (!strcmp(cmd.c_str(), "pwd")) 
	{
		
		if(cur_path.empty())	//if cur_path is initialized
			cout << cur_path <<endl;
		else{
			char* temp = get_current_dir_name();
			if(temp==NULL){
				perror("smash error: getcwd failed");
				return -1;
			}
			else{	//print to screen and update cur_path
				cout << temp <<endl;
				cur_path = temp;
			}
		}
	
	}
	/*************************************************/
	else if (!strcmp(cmd.c_str(), "jobs")) 
	{
		
		update_job_list();		// have to do that every time we call jobs
		time_t present_time(time(NULL));
		if(present_time==-1){
			perror("smash error: time failed");
			return -1;
		}
		auto it= jobs_list.begin();
		double elapsed_time;
		while(it!=jobs_list.end()){
			elapsed_time = difftime(present_time,(it->second).begining_time);	//calc runtime
			cout << "[" << it->first << "]" << (it->second).cmd << " : "
			<< (it->second).pid << " " << elapsed_time << " secs";
			if((it->second).is_stopped){
				cout << " (stopped)";
			}
			cout << endl;
			it++;
		} 			
	}

	/*************************************************/
	else if (!strcmp(cmd.c_str(), "showpid")) 
	{
		cout << "smash pid is " << getpid() << endl;
	}
	/*************************************************/
	else if (!strcmp(cmd.c_str(), "fg")) 
	{
		if(args_count>1){
			// too many arguments
			perror("smash error: fg: invalid arguments\n");
			error_in_cmd = true;
		}
		else if(args_count == 0 && jobs_list.empty()){
			// nothing to move to fg
			perror("smash error: fg: jobs list is empty/n");
		}
		else if(args_count!=0 && !is_number(args[1])){
			// arg not a number
			perror("smash error: fg: invalid arguments\n");
			error_in_cmd = true;
		}
		else if(args_count!=0 && (search_job(args[1])==-1)){
			// job not found
			cerr << "smash error: fg: job-id " << args[1] << " does not exist"<< endl;
		}
		else{
			// No errors - need to move to fg
			int waiting_fg_jid;
			if (args_count)	{
				// no args given, take job with highest jid
				waiting_fg_jid = jobs_list.rbegin()->first;
			}
			else{
				// find the proper jid in map
				waiting_fg_jid = search_job(args[1]);	//returns jid
			}
			// update fg data
			fg_clean();
			fg_replace(jobs_list[waiting_fg_jid].pid,
					   jobs_list[waiting_fg_jid].cmd,
					   waiting_fg_jid);
			cout << jobs_list[waiting_fg_jid].cmd << " : "
			<< jobs_list[waiting_fg_jid].pid << endl;
			jobs_list.erase(waiting_fg_jid);
			// now need to send signal to run
			if(kill(fg_job.fg_pid,SIGCONT)){
				perror("smash error: kill failed");
				return -1;
			}
			// need to wait for fg to finished
			if(waitpid(fg_job.fg_pid,NULL,WUNTRACED)==-1){
				perror("smash error: waitpid failed");
				return -1;
			}
			fg_clean();
		}
	} 
	/*************************************************/
	else if (!strcmp(cmd.c_str(), "bg")) 
	{
		if(args_count>1){
			// too many arguments
			perror("smash error: fg: invalid arguments\n");
			error_in_cmd = true;
		}
		else if(args_count==0 && (find_stopped_job()==-1)){
			// didn't find any stopped jobs to run in bg
			perror("smash error: bg: there are no stopped jobs to resume\n");
			error_in_cmd = true;
		}
  		else if(args_count!=0 && (!is_number(args[1]))){
			  // arg not a number
			  perror("smash error: bg: invalid arguments\n");
			  error_in_cmd = true;
		 }
		else if(args_count!=0 && (search_job(args[1])==-1)){
			// job not found
			cerr << "smash error: bg: job-id " << args[1] << " does not exist" << endl;
			error_in_cmd = true;
		}
		else{
			// all is good - execute
			int waiting_bg_jid;
			if(args_count){
				// no args, take the highest jid stopped
				waiting_bg_jid = find_stopped_job();
			}
			else{
				// first, convert to a number and than assign
				waiting_bg_jid = stoi(args[1]);
			}
			// now we have the job to run on bg - execute
			if(!jobs_list[waiting_bg_jid].is_stopped){
				// the job is already running
				cerr << "smash error: bg: job-id " << waiting_bg_jid 
				<< " is already running in the background" << endl;
			}
			else{
				cout << jobs_list[waiting_bg_jid].cmd << " : " 
				<< jobs_list[waiting_bg_jid].pid << endl;
				// now send the signal to continue
				if (kill(jobs_list[waiting_bg_jid].pid,SIGCONT)){
					perror("smash error: kill failed");
					return -1;
				}
				// update the job's state
				jobs_list[waiting_bg_jid].is_stopped=false;
			}




		}
	}
	/*************************************************/
	else if (!strcmp(cmd.c_str(), "quit"))
	{
   		// loop over jobs_list and terminate processes
		auto it= jobs_list.begin();
		pid_t test_proc;
		while(it!=jobs_list.end()){
			cout << "[" << it->first << "] " << it->second.cmd 
			<< " - " << "sending SIGTERM...";
			// sending the signal
			if(kill(it->second.pid, SIGTERM)){
				perror("smash error: kill failed");
				return -1;
			}
			// waiting 5 seconds to check if terminated
			sleep(5);
			test_proc = waitpid(it->second.pid,NULL,WNOHANG);
			if(test_proc==-1){
				perror("smash error: waitpid failed");
				return -1;
			}
			if(test_proc!=it->second.pid){
				// process not terminated
				cout << "(5 sec passed) sending SIGKILL...";
				if(kill(it->second.pid, SIGKILL)){
				perror("smash error: kill failed");
				return -1;
				}
			}
			cout << "Done." << endl;
			it++;
		}
	} 
	/*************************************************/
	else if (!strcmp(cmd.c_str(), "kill"))
	{
   		if(args_count!=2){
			   perror("smash error: kill: invalid arguments\n");
			   error_in_cmd=true;
		   }
		else{
			string sig_num(args[1]);
			string rel_jid(args[2]);
			// checking arg[0] is '-' like asked
			if(sig_num[0]!='-'){
				perror("smash error: kill: invalid arguments\n");
			   error_in_cmd=true;
			}
			else{
				sig_num.erase(0,1);	//remove the "-" in beginning
			}
			if (!error_in_cmd && (!is_number(sig_num) || is_number(rel_jid))){
				perror("smash error: kill: invalid arguments\n");
			    error_in_cmd=true;
			}
			else if(!error_in_cmd){
				// all is fine with command
				int rel_jid_int = stoi(rel_jid);
				int sig_num_int = stoi(sig_num);
				if(jobs_list.find(rel_jid_int)==jobs_list.end()){
					// means job not found
					cout << "smash error: kill: job-id " << rel_jid<<
					" does not exist" << endl;
					error_in_cmd=true;
				}
				else{
					// job found - send sig_num
					if(kill(jobs_list[rel_jid_int].pid,sig_num_int)){
						perror("smash error: kill failed");
						error_in_cmd=true;
					}
					cout << "signal number " << sig_num_int << " was sent to pid "
					 << jobs_list[rel_jid_int].pid << endl;
				}

			}
		}
	} 
	/*************************************************/	
	else if (!strcmp(cmd.c_str(), "diff"))
	{
        if (args_count != 2) {
            std::cerr << "smash error: diff: invalid arguments\n";
            error_in_cmd = true;
        } 
		else{
            int f1, f2;
            f1 = open(args[1].c_str(), O_RDONLY);
            if (f1 == -1) {
                perror("smash error: open failed");
                return -1;
            }
            f2 = open(args[2].c_str(), O_RDONLY);
            if (f2 == -1) {
                perror("smash error: open failed");
                close(f1);	
                return -1;
            }
            char c1, c2;
            bool diff = false;
            // Checks every byte in the files 
            while (read(f1, &c1, 1) > 0 || read(f2, &c2, 1) > 0) {
                if (c1 != c2) {
                    diff = true;
                    break;
                }
            }

            if (diff) { // different
                std::cout << "1" << std::endl;
            } 
			else { // identical
                std::cout << "0" << std::endl;
            }

            close(f1);
            close(f2);
        }
    }

	/*************************************************/	
	if (error_in_cmd == true)
	{
		return -1;
	}
    return 0;
}
//**************************************************************************************
// function name: ExeExternal
// Description: executes external command
// Parameters: external command arguments, external command string
// Returns: void
//**************************************************************************************
void ExeExternal(string args[MAX_ARG], int args_count, string command)
{
	int pID;
    	switch(pID = fork()) 
	{
    		case -1: 
					perror("smash error: fork failed");
					return;
        	case 0 :
               		// Child Process
               		setpgrp();
					char *cmd_args[MAX_ARG];
					for(int i=0; i<args_count+1;i++){	//going through args and parses them
						cmd_args[i] = const_cast<char*>(args[i].c_str());
					}
					cmd_args[args_count+1]=NULL;
					execvp(cmd_args[0],cmd_args);
					// execv returns only if there is an error
					perror("smash error: execv failed");
					exit(1);
			
			default:
                	// replace fg
					fg_clean();
					fg_replace(pID,command);	// insert the cmd to fg
					// father waits for the child process
					if (waitpid(pID,NULL,WUNTRACED) == -1){
						perror("smash error: waitpid failed");
						return;
					}
					// now child process finished so we can clean the fg
					fg_clean();
					return;
	}
	return;
}
//**************************************************************************************
// function name: BgCmd
// Description: if command is in background, insert the command to jobs
// Parameters: command string, pointer to jobs
// Returns: 0- BG command -1- if not
//**************************************************************************************
void BgCmd(string args[MAX_ARG], int args_count, string command)
{
	pid_t pID;
	switch(pID = fork()){ 
	
    		case -1: 
					perror("smash error: fork failed");
					return;
        	case 0 :
                	// Child Process
               		setpgrp();
					char *cmd_args[MAX_ARG];
					for(int i=0; i<args_count+1;i++){	//going through args and parses them
						cmd_args[i] = const_cast<char*>(args[i].c_str());
					}
					cmd_args[args_count+1]=NULL;
					execvp(cmd_args[0],cmd_args);
					// execv returns only if there is an error
					perror("smash error: execv failed");
					exit(1);
			default:
                	// Parent Process
					// not in fg anymore - add to job list
					if(!insert_job(pID,command)){
						return;
					}
					return;
	}
	return;
}

