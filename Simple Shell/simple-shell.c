// INCLUDING THE REQUIRED HEADER FILES FOR THE PROGRAM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

// PRE-DEFINED SIZE VARIABLES

#define MAX_COMMAND_LENGTH 1024
#define MAX_HISTORY_SIZE 10000
#define MAX_ARGS_SIZE 10000

/*
GLOBAL ARRAYS:
history: Stores all the commands which have been entered by the user to the shell.
pidArray: Stores the Process ID (PID) of the respective commands entered by the user.
startArray: Stores the start time of the execution of the respective commands entered by the user.
endArray: Stores the total duration of the execution of the respective commands entered by the user.
*/

char *history[MAX_HISTORY_SIZE];
int history_index = 0;              // initializing the index to 0

int pidArray[MAX_HISTORY_SIZE];
int pid_index = 0;                  // initializing the index to 0

long int startArray[MAX_HISTORY_SIZE];
int start_index = 0;                // initializing the index to 0

long int endArray[MAX_HISTORY_SIZE];
int end_index = 0;                  // initializing the index to 0

// read_UserInput function reads the user input and removes the '\n' character from the input.

char *read_userInput() {
    char *input = malloc(MAX_COMMAND_LENGTH * sizeof(char));
    if (!input) {
        fprintf(stderr, "Memory allocation error.\n");
        exit(EXIT_FAILURE);
    }

    if (fgets(input, MAX_COMMAND_LENGTH, stdin) == NULL) {
        perror("Error in fgets");
        exit(EXIT_FAILURE);
    }

    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }

    return input;
}

/*
commandingArgs takes the user entered command and stores them in the space separated form.
This helps us get the system command and the passed argument separated.
*/

char **commandingArgs(char *command) {
    char **args = malloc(MAX_ARGS_SIZE * sizeof(char *));
    if (!args) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    char *ptr = strtok(command, " ");
    int index = 0;

    while (ptr != NULL && index < MAX_ARGS_SIZE - 1) {
        args[index++] = ptr;
        ptr = strtok(NULL, " ");
    }
    args[index] = NULL;                       // Null-terminating the argument list

    return args;
}

// addStartTime function adds the start time (microseconds) of the command to the startArray.

void addStartTime(int start) {
    if (start_index < MAX_HISTORY_SIZE) {
        startArray[start_index++] = start;
    } 
    else {
        for (int i = 1; i < MAX_HISTORY_SIZE; i++) {
            startArray[i-1] = startArray[i];
        }
        startArray[MAX_HISTORY_SIZE - 1] = start;
    }
}

/*
addEndTime function adds the total time execution 
time (microseconds) of the command to the endArray.
*/

void addEndTime(int end) {
    if (end_index < MAX_HISTORY_SIZE) {
        endArray[end_index++] = end;
    } 
    else {
        for (int i = 1; i < MAX_HISTORY_SIZE; i++) {
            endArray[i-1] = startArray[i];
        }
        endArray[MAX_HISTORY_SIZE - 1] = end;
    }
}

// addPid function adds the PID of the command to the pidArray.

void addPid(int pid) {
    if (pid_index < MAX_HISTORY_SIZE) {
        pidArray[pid_index++] = pid;
    } 
    else {
        // free(pidArray[0]);
        for (int i = 1; i < MAX_HISTORY_SIZE; i++) {
            pidArray[i - 1] = pidArray[i];
        }
        pidArray[MAX_HISTORY_SIZE - 1] = pid;
    }
}

// addHistory function adds the command to the history list of commands.

void addHistory(char *command) {
    if (history_index < MAX_HISTORY_SIZE) {
        history[history_index++] = strdup(command);
    } else {
        free(history[0]);
        for (int i = 1; i < MAX_HISTORY_SIZE; i++) {
            history[i - 1] = history[i];
        }
        history[MAX_HISTORY_SIZE - 1] = strdup(command);
    }
}

/*
printHistory function prints the history of all the commands entered by the user till now.
It specifically prints the following details about the command:
1.The Serial Number
2.The PID of the process
3.The user input entered as the command.
4.The start time of the process.
5.The total execution time of the process.
*/

void printHistory() {
    for (int i = 0; i < history_index; i++) {
        printf("%d. PID: %d, Command: %s\n", i + 1, pidArray[i], history[i]);
        printf("Start Time: %ld microseconds\n", startArray[i]);
        printf("Total Duration: %ld microseconds\n", endArray[i]);
    }
}

void print_time_duration(struct timeval start, struct timeval end) {
    long sec = end.tv_usec - start.tv_usec;
    printf("Execution Time: %ld microseconds\n", sec);
}

/*
the launch function takes the command and creates child processes out of parent processes
using fork() and asks the parent to wait for the completion of the child process.  
*/

int launch(char **args) {
    pid_t pid;
    int status;
    int flag=0;
    int i=0;

    // IMPLEMENTATION OF "&" IN THE COMMANDS (THE BONUS PART OF THE ASSIGNMENT)
    while (args[i] != NULL){
        if(strcmp(args[i],"&") == 0){
            flag = 1;
            for (int j = i; args[j] != NULL; j++){
                args[j] = args[j+1];
            }
            i--;
        }
        i++;
    }
    
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);   // stores the current time into start_time
    addStartTime(start_time.tv_usec);

    pid = fork();                      // stores the PID of child process into variable pid
    if (pid == 0) {
        // It is a Child Process
        if (execvp(args[0], args) == -1) {
            perror("Error in execvp");
            exit(EXIT_FAILURE);
        }
    } 
    else if (pid < 0) {
        // Fork Failure ("Error Handling")
        perror("Fork error");
        return -1;
    } 
    else {
        // It is a Parent Process
        if (!flag) {
            waitpid(pid, &status, 0);      // waits till the child process completes
            gettimeofday(&end_time, NULL); // stores the current time into end_time
            addEndTime(end_time.tv_usec - start_time.tv_usec);
        }
        else printf("Process running in background. \n");
        
        addPid(pid);
        
    }

    return 1;
}

// ifPipe function checks if the command entered implements pipe.

int ifPipe(char *command) {
    return strchr(command, '|') != NULL;
}

// Implements the pipe for the execution of the command.

void ifPipe_Execute(char *command) {
    char *c1 = strtok(command, "|");
    char *c2 = strtok(NULL, "|");

    if (c1 == NULL || c2 == NULL) {
        fprintf(stderr, "Invalid pipe command\n");
        return;
    }

    int fd[2];
    if (pipe(fd) == -1) {
        perror("Pipe failed");
        return;
    }

    pid_t pid;
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);    // stores the current time into start_time
    addStartTime(start_time.tv_usec);

    pid = fork();                       // stores the PID of child process into variable pid
    if (pid == 0) {
        // First Child (the command before the pipe)
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);

        char **args1 = commandingArgs(c1);
        if (execvp(args1[0], args1) == -1) {
            perror("Error in execvp c1");
            exit(EXIT_FAILURE);
        }
    }

    pid = fork();                      // stores the PID of child process into variable pid
    if (pid == 0) {
        // Second Child (the command after the pipe)
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);

        char **args2 = commandingArgs(c2);
        if (execvp(args2[0], args2) == -1) {
            perror("Error in execvp c2");
            exit(EXIT_FAILURE);
        }
    }

    // Closing the File Descriptors after the reading/writing into the file is done.
    close(fd[0]);
    close(fd[1]);

    // wait till the completion of the child processes
    wait(NULL);

    gettimeofday(&end_time, NULL);       // stores the current time into end_time
    addEndTime(end_time.tv_usec - start_time.tv_usec); 
    addPid(pid);
}

/*
sys_call handles the Ctrl+C interrupt signal. 
It terminates the program and prints the history of commands.
*/

void sys_call(int sig) {
    printf("\nInterrupt signal (SIGINT) received. Exiting shell and displaying history.\n");
    printHistory();                         // prints the history of commands
    exit(0);  
}

/*
loop_shell() function is the main function which calls an infinite do-while loop & constantly
asks for the user input to execute the entered commands. It adds the entered command into the 
history list and checks if the command needs the use of pipes or can be simply launched
to be executed.
The infinite do-while loop can be broken by giving Ctrl+c as the input for the command. Once it
is called, it gives the signal SIGINT to the parent process it terminates the process and prints 
the command history. The function also frees all the memory allocated to the command.
*/

void loop_shell() {

    signal(SIGINT, sys_call);

    char *command;
    char **args;

    while (1) {
        printf("pvtshell:~$ ");
        command = read_userInput();

        if (strlen(command) == 0) {
            free(command);
            continue;
        }

        addHistory(command);

        if (strcmp(command, "history") == 0) {
            printHistory();
            free(command);
            continue;
        }

        if (ifPipe(command)) {
            ifPipe_Execute(command);
        } 
        else {
            args = commandingArgs(command);
            launch(args);
            free(args);
        }

        free(command);
    }
}

int main() {
    loop_shell();
    return 0;
}