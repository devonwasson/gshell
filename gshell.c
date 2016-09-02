// Devon Wasson
// csci 315
// lab 12
// gshell.c

/*

features
	: exiting by typing exit or ctrl-d
        : cd [directory] works as expected but requires an argument
        : help gives help with the new features
        : history shows all commands entered while in the shell
        : last shows the last command. entering last again shows the next last, ect..
        : execute executes the command that last has showed. If last was never used,
                nothing happens.

*/

#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termio.h>
#include <termios.h>
#include "dlist.h"
#include "dnode.h"

#define MAX_TOKEN_SIZE 128
#define SHELL_BUFFER_SIZE 256
#define ASCII_SPACE 32
#define ASCII_TAB 9
#define ASCII_NEWLINE 10
#define MAX_TOK_SIZE 128

char lastCommandBuf[SHELL_BUFFER_SIZE];

struct dlist *history;

void programRunner(char *input);

int entercount = 0;

void lastCommand() {
	if (dlist_iter_has_next(history)) {
			memset(lastCommandBuf, '\0', SHELL_BUFFER_SIZE);
			strcpy(lastCommandBuf, dlist_iter_next(history)->data);
			printf("%s\n", lastCommandBuf);
	}
	else {
			if (history->iter == NULL) {
				printf("No last command!\n");
			}
			else {
				memset(lastCommandBuf, '\0', SHELL_BUFFER_SIZE);
				strcpy(lastCommandBuf, history->iter->data);
				printf("%s\n", history->iter->data);
				dlist_iter_next(history);
			}
	}
}

void historycommand() {
	printf("[most recent]\n");
	int count = 0;
	while (dlist_iter_has_next(history)) {
		printf(" %d: %s\n",++count, dlist_iter_next(history)->data);
	}
	printf(" %d: %s\n", ++count, history->iter->data);
	dlist_iter_begin(history);
	printf("[least recent]\n");
}


int main(int argc, char *argv[]){
	history = dlist_create();

	char *input = calloc(SHELL_BUFFER_SIZE, 1);
	struct termios first_tio, second_tio;
	unsigned char c;
	int index = 0;
	int tabcount = 0;
	while(1){
		memset(input, '\0', SHELL_BUFFER_SIZE);
		printf("gshell> ");
		fflush(stdout);
		fflush(stdin);

		//get terminal settings
		tcgetattr(STDIN_FILENO, &first_tio);

		//want to keep old settings
		second_tio = first_tio;

		second_tio.c_lflag &=(~ICANON & ~ECHO);

		//set the new settings
		tcsetattr(STDIN_FILENO, TCSANOW, &second_tio);

		//read in the new commands, paying attention to tabs
		index = 0;
		tabcount = 0;
		int temp;
		do {

			temp = fgetc(stdin);
			c = (unsigned char)temp;
			if((c != '\t')){
				input[index] = c;
				index++;
			}
			if(temp == 4){
				tcsetattr(STDIN_FILENO, TCSANOW, &first_tio);
				printf("Exiting.\n");
				exit(0);
			}
			if(temp == 10){
				entercount++;
				if(entercount == 2){
					tabcount = 2;
					entercount = 0;
					break;
				}
			}
			if('\t' == c){
				tabcount++;
				if(tabcount == 2){break;}
			}
			else{
				printf("%c", c);
				tabcount = 0;
			}
		} while(c!= '\n');

		//add null terminator
		input[index-1] = '\0';

		//reset to old settings
		tcsetattr(STDIN_FILENO, TCSANOW, &first_tio);

		if((tabcount == 2) && ((input[0] == '\n') || (input[0] == '\0') || (input[0] == '\t'))){
			printf("\n");
			fflush(stdout);
			input[0] = 'l';
			input[1] = 's';
			input[2] = '\n';
			input[3] = '\0';
		}

		else if (tabcount == 2){
     			printf("\n");
		}

		if((input[0] == ASCII_SPACE) || (input[0] == ASCII_TAB) || (input[0] == ASCII_NEWLINE)){
			printf("Not a valid command.\n");
			fflush(stdout);
			continue;
		}

		//split input based on semi colons

		int numSemi = 1;
		int d;
		for (d = 0; d < SHELL_BUFFER_SIZE; d++){
			if(input[d] == ';'){
				numSemi++;
			}
			if(input[d] == '\0'){break;} //more effecient
		}

		char **commands = calloc(numSemi + 1, sizeof(SHELL_BUFFER_SIZE));

		commands[0] = strtok(input, ";");
		int f = 1;
		while((commands[f] = strtok(NULL, ";")) != NULL){
			numSemi++;
			f++;
		}
		int a;
		for(a = 0; a < numSemi; a++){
			programRunner(commands[a]);
		}
	}
}


void programRunner(char *input){
	dlist_add_front(history, input);

	int numTokens = 0;
	int j;
	int stat;
	int status;
	char *in;
	char *out;
	FILE *ptr;
	pid_t pid;
	int fd[2];

	if(input == NULL){return;}

	for (j = 1; input[j] != '\0'; j++){
		if(((int) input[j] == ASCII_SPACE) || ((int) input[j] == ASCII_TAB) || ((int) input[j] == ASCII_NEWLINE)){
			numTokens++;

			if(((int) input[j-1] == ASCII_SPACE) || ((int) input[j-1] == ASCII_TAB) || ((int) input[j-1] == ASCII_NEWLINE)){
				numTokens--;
			}
		}
	}

	//HOLDS THE COMPONENTS OF THIS COMMAND IN A LIST
	char *argument[numTokens];
	int count; //ADDS NULLs TO THE ENTIRE LIST
	int pip = -1; //CHANGES TO INDEX OF PIPE
	for (count = 0; count < numTokens; count++){
		argument[count] = NULL;
	}
	//ALLOCATE MEMORY TO STORE COMMANDS
	*argument = calloc(numTokens, MAX_TOKEN_SIZE);
	//TOKENIZES THE COMMAND INTO ARGUMENT LIST
	argument[0] = strtok(input, " \t\0");
	j = 1;
	while((argument[j] = strtok(NULL, " \t\0")) != NULL){
		if (strcmp(argument[j],"|")==0) {pip = j;}
			j++;
	}

	if(pip != -1){ //if we have a pipe'd process
		char *argument1[numTokens];
		char *argument2[numTokens];
		argument[pip] = NULL; //remove the pipe from list of args
		*argument1 = calloc(pip, MAX_TOKEN_SIZE);
		*argument2 = calloc(numTokens - pip, MAX_TOKEN_SIZE);

		/*store arguments in arrays*/
		int ii = 0;
		while(ii < pip){
			argument1[ii] = argument[ii];
			ii++;
		}
		int iii = 0;
		ii++;
		while(ii < numTokens + 1){
			argument2[iii] = argument[ii];
			ii++;
			iii++;
		}

		if ((pid = fork()) < 0){
			perror("fork error");
			exit(-1);
		}
		else if (pid == 0){ //child process
			pid_t childPid;
			pipe(fd);
			if ((childPid = fork()) == -1){
				perror("fork error");
				exit(-1);
			}
			else if (childPid == 0){
				close(fd[0]);
				close(1);
				dup2(fd[1], 1);
				execvp(argument1[0], argument1);
			}
			else{ //parent process
				wait(&childPid);
				close(fd[1]);
				close(0);
				dup2(fd[0], 0);
				execvp(argument2[0], argument2);
			}
		}
		else{ //parent process
			waitpid(pid, &status, 0);
			return; //did everything we have to do, so we're done here
			printf("program terminated successfully!\n");
		}

	}

	int num_features = 6;
        char *features[] = {
	"exiting by typing exit or ctrl-d.",
        "cd [directory] works as expected but requires an argument.",
        "help gives help with the new features.",
        "history shows all commands entered while in the shell. \n\tPrograms that failed to execute are not remembered in history.",
        "last shows the last command. entering last again shows the next last, ect..",
        "executeLast executes the command that last has showed. \n\tIf last was never used, nothing happens."
        };

	entercount = 0;

	//added feature to exit from the shell by typing exit
	if(strncmp("exit\0", argument[0], 5) == 0){
		printf("Exiting.\n");
		fflush(stdout);
		exit(0);
	}

	if (strncmp("executeLast\0", argument[0], 12) == 0) {
		if(lastCommandBuf[0] == '\0'){
			printf("The last command needs to be executed first.\n");
			return;
		}
		programRunner(lastCommandBuf);
		return;
	}

	if (strncmp("history\0", argument[0], 8) == 0) {
		historycommand();
		return;
	}

	if (strcmp(argument[0], "last") == 0) {
		lastCommand();
		return;
	}

	if(strncmp("cd\0", argument[0], 3) == 0){
		if (argument[1] == NULL){
			dlist_remove_front(history);
			printf("format cd [directory]\n");
			return;
		}
		int changeDir = chdir(argument[1]);
		if(changeDir == 0){
			printf("program terminated successfully!\n");
		}
		else{
			dlist_remove_front(history);
			printf("program terminated abnormally.\n");
		}
		return;
	}

	if(strcmp(argument[0],"help") == 0) {
                printf("-------------------- GSHELL FEATURES --------------------\n");
                printf("THESE FEATURES GIVE ADDITIONAL FUNCTIONALITY TO THE SHELL\n");
                int indexer;
                for (indexer = 0; indexer < num_features; indexer++) {
                        printf("<>  %s\n", features[indexer]);
                }
                printf("\n");
                return;
        }

	in = malloc(128);
	out = malloc(128);
	strcpy(in, "which ");
	strcat(in, argument[0]);

	ptr = popen(in, "r");
	fgets(out, 128, ptr);
	pclose(ptr);

	argument[0] = out;

	int i;
	int h;
	for(i = 0; argument[i] != NULL; i++){
		for (h = 0; h < strlen(argument[i]); h++){
			if(argument[i][h] == ASCII_NEWLINE){
				argument[i][h] = '\0';
			}
		}
	}

	if ((pid = fork()) < 0){
		perror("fork error");
		exit(-1);
	}
	//child
	else if (pid == 0){
		stat = execv(argument[0], argument);
		if(stat < 0){
			perror("execv error");
			exit(-1);
		}
	}
	//parent
	else{
		waitpid(pid, &status, 0);
		if(status != 0){
			dlist_remove_front(history);
			printf("program terminated abnormally: %d.\n", status);
		}
		else{
			printf("program terminated successfully!\n");
		}
	}

}
