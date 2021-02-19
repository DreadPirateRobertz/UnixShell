#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "dispatcher.h"
#include "shell_builtins.h"
#include "parser.h"
//tododeleteme

/**
 * dispatch_external_command() - run a pipeline of commands
 *
 * @pipeline:   A "struct command" pointer representing one or more
 *              commands chained together in a pipeline.  See the
 *              documentation in parser.h for the layout of this data
 *              structure.  It is also recommended that you use the
 *              "parseview" demo program included in this project to
 *              observe the layout of this structure for a variety of
 * 
           inputs.
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static int uno_commando(struct command *pipeline){

	bool outFlag = 0;
	bool inFlag = 0;
	int fdOut = STDOUT_FILENO;
	int fdIn = STDIN_FILENO;
  	
	pid_t forkey;
    int status = 0;
	
		//Let's check for file Redirection; wanted to do this as a switch but it wanted me to put down every
		//output type
		if(pipeline->output_type == COMMAND_OUTPUT_FILE_TRUNCATE){
			outFlag = true; 
			fdOut = open(pipeline->output_filename, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
			if(fdOut == -1) perror("");
		}
		if(pipeline->output_type == COMMAND_OUTPUT_FILE_APPEND){
			outFlag = true; 
			fdOut = open(pipeline->output_filename, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
			if(fdOut == -1) perror("");
		}
		if(pipeline->input_filename != NULL){
			inFlag = true; 
			fdIn = open(pipeline->input_filename, O_RDONLY);
			if(fdIn == -1) perror("");
		}

	forkey = fork();//forkey == pid
	if(forkey<0) perror("");
	else if(forkey == 0) { //Child Process
		if(outFlag){
			if(dup2(fdOut, STDOUT_FILENO) == -1){
				perror("");
				exit(errno);
			}
		}
		if(inFlag)	{
			if(dup2(fdIn, STDIN_FILENO) == -1){
				perror("");
				exit(errno);
			}
		}
		if((execvp(pipeline->argv[0], pipeline->argv)) == -1) perror(""); //Prints out Appropriate error 	
		exit(errno);
	}
	//Parent -> Close Files and wait 	
	 if(outFlag){
		 if(close(fdOut) == -1) perror("");
	 }
	  if(inFlag){
		 if(close(fdIn) == -1) perror("");
	 }
	 if(wait(&status) != forkey) {//Wait for your CHILD!	
		perror("");
	 } 
	 printf("Uno Commando Exit Status: %d\n", WEXITSTATUS(status));
	 return status;
}
 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static int terminal_command(struct command *pipeline, int fd[]){
	
	bool outFlag = 0;
	bool inFlag = 0;
	int fdOut = STDOUT_FILENO;
	int fdIn = STDIN_FILENO;
  	
	pid_t forkey;
    int status = 0;
	
		//Let's check for file Redirection; wanted to do this as a switch but it wanted me to put down every
		//output type
		if(pipeline->output_type == COMMAND_OUTPUT_FILE_TRUNCATE){
			outFlag = true; 
			fdOut = open(pipeline->output_filename, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
			if(fdOut == -1) perror("");
		}
		if(pipeline->output_type == COMMAND_OUTPUT_FILE_APPEND){
			outFlag = true; 
			fdOut = open(pipeline->output_filename, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
			if(fdOut == -1) perror("");
		}
		if(pipeline->input_filename != NULL){
			inFlag = true; 
			fdIn = open(pipeline->input_filename, O_RDONLY);
			if(fdIn == -1) perror("");
		}

	forkey = fork();//forkey == pid
	if(forkey<0) perror("");

	else if(forkey == 0) { //Child Process
		if(outFlag){
			if(dup2(fdOut, STDOUT_FILENO) == -1){
				perror("");
				exit(errno);
			}
		}
		if(inFlag)	{
			if(dup2(fdIn, STDIN_FILENO) == -1){
				perror("");
				exit(errno);
			}
		}
		if(dup2(fd[0], STDIN_FILENO) == -1) perror("");
		if((execvp(pipeline->argv[0], pipeline->argv)) == -1) {
				perror("Test 2: Child Exec Fail -> Terminal Command"); //Error handling and executing the child	
				exit(errno);
		}
	}
	 //Parent -> 
	 if(close(fd[0]) == -1) perror(""); //Closing read	
	 if(wait(&status) != forkey) {//Wait for your CHILD!	
		perror("");
	 } 
	 
	 printf("Terminal Command Exit Status: %d\n", WEXITSTATUS(status));
	 
	 return status;	
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static int down_the_pipe(struct command *pipeline){
	pid_t forkey; 
    int status = 0;;
	int fd[2]; //File Descriptor Array fd[0] = read fd[1] = write
	int fdIn = STDIN_FILENO;
	bool inFlag = 0;
		
	if(pipe(fd) == -1) perror("");//Pipe is open and takes care of errors

	if(pipeline->input_filename != NULL){
		inFlag = true; 
		fdIn = open(pipeline->input_filename, O_RDONLY);
		if(fdIn == -1) perror("");
		}

	forkey = fork(); //forkey == pid
	if(forkey<0) perror("");

		else if(forkey == 0) { //Child Process
			if(close(fd[0])==-1) perror(""); //Closing Read
			if(dup2(fd[1], STDOUT_FILENO)==-1) perror("");//dup2(source, destination) //Writing to the pipe
			if(inFlag)	{
				if(dup2(fdIn, STDIN_FILENO) == -1){
					perror("");
					exit(errno);
			}
		}
			if((execvp(pipeline->argv[0], pipeline->argv)) == -1) {
				 perror("Test 1: Child Exec Fail -> Down The Pipe"); //Performing child process thru execvp
				 exit(errno);
			}
			}
		//Parent Below ->
		if(inFlag){
			if(close(fdIn) == -1) perror("");
		}

		if(close(fd[1]) == -1) perror(""); //Closing write  
	 	if(wait(&status) != forkey) perror(""); //Wait for your CHILD!
		pipeline = pipeline->pipe_to;  //Makes it easier

	 	if(pipeline->output_type == COMMAND_OUTPUT_PIPE){ //Are there more pipeS??? :)
			if((forkey = fork())==-1) {//Fork Numero Dos
				perror(""); 
			}
			else if (forkey == 0){ //CHILD
			if(dup2(fd[0], STDIN_FILENO) == -1) perror(""); // -> Pass the output down the pipe
					
			down_the_pipe(pipeline); //Recurse
			exit(0);
			}
			else if (forkey > 0){	 
				while (wait(NULL) != -1 || errno != ECHILD) {  //Code Snippet from StackOverflow
					//Chill out and wait for your ALL damn children
				}	  
		 }}
		  printf("Down The Pipe Exit Status: %d\n", WEXITSTATUS(status));
		//Tried the wait statement right here -> somehow the down the pipe status is superceding terminal status ~> SHIT :)
		 if(pipeline->output_type != COMMAND_OUTPUT_PIPE){ //Seems not to matter if I set either of these 2 functions equal to status
			  terminal_command(pipeline, fd);  //TODO: GET PROGRAM TO RETURN THIS GODDAMN STATUS -> RIGHT NOW RETURNING :) with three+ bad commands I wonder if another waiting issue
				
			  }
		 
		 return status;	//Tried different variables to return terminal directly, tried terminal_command to return at bottom of dispatch_external
		 				//Tried calling terminal command afterd down the pipe in external command and passing fd -> none worked for some reason
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static int dispatch_external_command(struct command *pipeline)
{

	int status = 0;

	 if(pipeline->output_type == COMMAND_OUTPUT_PIPE){ //Is there a pipe? Let's go, Mario!
		status= down_the_pipe(pipeline); 
	 }
	 else{		
		status = uno_commando(pipeline);

	 }
	 return status;	
}


/**
 * dispatch_parsed_command() - run a command after it has been parsed
 *
 * @cmd:                The parsed command.
 * @last_rv:            The return code of the previously executed
 *                      command.
 * @shell_should_exit:  Output parameter which is set to true when the
 *                      shell is intended to exit.
 *
 * Return: the return status of the command.
 */
static int dispatch_parsed_command(struct command *cmd, int last_rv,
				   bool *shell_should_exit)
{
	/* First, try to see if it's a builtin. */
	for (size_t i = 0; builtin_commands[i].name; i++) {
		if (!strcmp(builtin_commands[i].name, cmd->argv[0])) {
			/* We found a match!  Run it. */
			return builtin_commands[i].handler(
				(const char *const *)cmd->argv, last_rv,
				shell_should_exit);
		}
	}

	/* Otherwise, it's an external command. */
	return dispatch_external_command(cmd);
}

int shell_command_dispatcher(const char *input, int last_rv,
			     bool *shell_should_exit)
{
	int rv;
	struct command *parse_result;
	enum parse_error parse_error = parse_input(input, &parse_result);

	if (parse_error) {
		fprintf(stderr, "Input parse error: %s\n",
			parse_error_str[parse_error]);
		return -1;
	}

	/* Empty line */
	if (!parse_result)
		return last_rv;

	rv = dispatch_parsed_command(parse_result, last_rv, shell_should_exit);
	free_parse_result(parse_result);
	return rv;
}