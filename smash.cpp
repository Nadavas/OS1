#include "commands.h"
#include "signals.h"
//#include <fcntl.h>   // For O_RDONLY
//#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>   // For open

#define MAX_LINE_SIZE 80
#define MAXARGS 20

using namespace std;
using string = std::string;


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
	if(sigaction(SIGSTOP,&sa,NULL)==-1){
		perror("smash error: sigaction failed");
	}/************************************/
	// check for current path
	if ((cur_path.empty()) && (prev_path.empty())){		
		char* temp = get_current_dir_name();
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
		std::string args[MAXARGS];
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
			if(args[0]!="&" && last_char_cmd=='&'){		//needs to run on bg
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
	return 0;
}

