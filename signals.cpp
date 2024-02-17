// signals.cpp
// contains signal handler funtions
// contains the function/s that set the signal handlers

/*******************************************/
/* Name: handler_cntlc
   Synopsis: handle the Control-C */
#include "signals.h"
#include "commands.h"
#define MAX_LINE_SIZE 80
#define MAXARGS 20

// linking the globals form smash.cpp
extern Fg_job fg_job;


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


