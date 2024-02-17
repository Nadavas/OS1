/*	smash.cpp
main file. This file contains the main function of smash
*******************************************************************/
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "commands.h"
#include "signals.h"
#define MAX_LINE_SIZE 80
#define MAXARGS 20

using namespace std;
using string = std::string;
//global data
int last_job_id=0;	
string prev_path = NULL;	// string describing the last path 
string cur_path = NULL;	// string describing the current path 
std::map<int, job, std::less<int>> jobs_list;	// jobs list in ascending order based on job id
Fg_job fg_job;	//check later

//**************************************************************************************
// function name: main
// Description: main function of smash. get command from user and calls command functions
//**************************************************************************************
int main(int argc, char *argv[])
{  
	
	//signal declaretions
	//NOTE: the signal handlers and the function/s that sets the handler should be found in siganls.c
	 /* add your code here */
	struct sigaction sa;	// decalring the struct handler for sigaction
	/************************************/
	//NOTE: the signal handlers and the function/s that sets the handler should be found in siganls.c
	//set your signal handlers here
	/* add your code here */
	sa.sa_handler = &sig_handler;	// for the proper routine
	sa.sa_flags = 0;
	if(sigaction(SIGINT,&sa,NULL)==-1){
		perror("smash error: sigaction failed");
	}
	if(sigaction(SIGSTP,&sa,NULL)==-1){
		perror("smash error: sigaction failed");
	}/************************************/
	// check for current path
	if (cur_path!=NULL && prev_path!=NULL){		
		string temp = get_current_dir_name();
		if (temp==NULL){
			perror("smash error: getcwd failed");
			return -1;
		}
		cur_path = temp;	// setting the cur_path to the proper value
	}
	/************************************/

    	while (1)
    	{
	 	printf("smash > ");
		update_job_list();		// have to do that between 2 commands
		string input,command;
		std::getline(std::cin,input);	//getting the cmd from terminal
		command = input;	
		string args[MAXARGS];
		int args_count = break_cmd_to_args(input,args);		// args - array of cmd arguments
		if (check_if_built_in_cmd(args[0])){
			// In case this is a built in command
			char last_char_cmd = args[args_count][args[args_count].length()-1];
			if (last_char_cmd=='&'){	//needs to run on bg
				args[args_count].pop_back();	//removes &
				if(args[args_count].empty()){
					args_count--;
				}
			}
			ExeCmd(args,args_count,command);
		}
		else{
			// In case of external command
			char last_char_cmd = args[args_count][args[args_count].length()-1];
			if(args[0]!='&' && last_char_cmd=='&'){		//needs to run on bg
				args[args_count].pop_back();
				if(args[args_count].empty()){
					args_count--;
				}
			BgCmd(args,args_count,command);		//runs it on bg
			}
			else{	// needs to run on fg
				ExeExternal(args,args_count,command);
			}
		}
	
		fg_clean();
		}
	free(prev_path);
	free(cur_path);
	return 0;
}

