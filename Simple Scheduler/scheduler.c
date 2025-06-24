// INCLUDING THE REQUIRED HEADER FILES FOR THE PROGRAM

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>

// // EXTERNAL FUNCTIONS & VARIABLES REQUIRED FROM FILE scheduler.c

extern char *readyQueue[];
extern char **commandingArgs(char *command);
extern int ready_index;
extern int nCPU, tSlice;

// PRE-DEFINED SIZE VARIABLE

#define MAX_READY_SIZE 10000

/*
GLOBAL ARRAYS:
processPIDs: this array keeps the temporary values of PIDs of different processes.
printPIDs: this array keeps the permanent values of PIDs of different processes.
comp: this array stores the completion time of the processes.
waitTime: this array stores the waiting time of the processes.
*/

int processPIDs[MAX_READY_SIZE];  
int printPIDs[MAX_READY_SIZE];  
int comp[MAX_READY_SIZE];  
int waitTime[MAX_READY_SIZE]; 

// GLOBAL VARIABLE: keeps track of the time for which the scheduler is running.

int globalTime = 0;

/*
printSchedulerDetails() function prints the details of the executed processes once the program is
terminated using Ctrl+c Interrupt. This function prints the command name, its PID, its completion
time (in ms) as well as its waiting time (in ms).
*/

void printSchedulerDetails() {
    for (int i=0; i<ready_index; i++) {
        printf("Command %d: %s\n", i+1, readyQueue[i]);
        printf("PID: %d\n", printPIDs[i]);
        printf("Completion Time: %d\n", comp[i]);
        printf("Waiting Time: %d\n\n", waitTime[i]);
    }
}

// is_executable function checks if a file exists
int is_executable(const char *job) {
    if (access(job, F_OK) != 0) {
        printf("Error: File %s does not exist.\n", job);
        return 0;
    }
    return 1;
}

/*
SimpleScheduler_main() function is called from the shell file. It executes the Round-Robin
due to which nCPU processes are run simultaneously for tSlice time then the set of processes
are changed. This proceeds till all the processes are completely executed.The completion time 
& the waiting time of the processes are stored into comp & waitTime arrays respectively.
*/

int SimpleScheduler_main(int argc, char *argv[]) {

    printf("SimpleScheduler is now running with %d CPUs and %d ms time slice.\n\n", nCPU, tSlice);

    int running_count = 0;  // number of processes which run in this round.
    int process_index = 0;  // the process_index from which the processes start running.
    // jobs_left is the variable to store the number of processes which still need are not completely executed.
    int jobs_remaining = ready_index;

    // processPIDs, completion time & waiting time initialized to -1, 0 & 0 for all the processes.
    for (int i = 0; i < ready_index; i++) {
        processPIDs[i] = -1;  
        comp[i] = 0;  
        waitTime[i] = 0;  
    }

    while (jobs_remaining > 0) {
        
        globalTime += tSlice;
        // printf("%d\n", globalTime);
        running_count = 0;
        int start_index = process_index; // stores the process_index from which the processes start running.
        int i = 0;

        for (running_count = 0; i < ready_index && running_count < nCPU && process_index < ready_index; i++) {

            // this condition checks if the process is completely executed.
            if (processPIDs[process_index] == 0) {
                process_index = (process_index + 1) % ready_index;
                continue;
            }

            // Check if the process exists and is executable before forking
            if (!is_executable(readyQueue[process_index])) {
                processPIDs[process_index] = 0; // Mark as completed to skip further attempts
                jobs_remaining--;
                process_index = (process_index + 1) % ready_index;
                continue;
            }

            // this condition creates a process using fork().
            if (processPIDs[process_index] == -1) {
                pid_t pid = fork(); // process creation

                if (pid == 0) {

                    // printf("Starting job: %s\n", readyQueue[process_index]);
                    comp[process_index] += tSlice;
                    char **args = commandingArgs(readyQueue[process_index]);
                    
                    // process execution using exevp
                    if (execvp(args[0], args) == -1) {
                        perror("Error executing job"); // Error handling for exec
                        exit(EXIT_FAILURE);
                    }

                } else if (pid > 0) {

                    processPIDs[process_index] = pid;
                    printPIDs[process_index] = pid;
                    // printf("Forked job %s with PID %d\n", readyQueue[process_index], pid);
                    running_count++;

                } else {
                    perror("Fork failed"); // error handling
                }
            } 

            // this condition starts the process with this PID to run for next tSlice ms
            else if (processPIDs[process_index] > 0) {
                kill(processPIDs[process_index], SIGCONT);
                // printf("Resumed job %s (PID: %d)\n", readyQueue[process_index], processPIDs[process_index]);
                running_count++;
            }

            process_index = (process_index + 1) % ready_index;
        }

        // usleep() stops the scheduler for working for next tSlice ms & run the currently running processes.
        usleep(tSlice * 1000);  

        // this for loop stops the currently running processes and correctly increments the completion time.
        for (int i = start_index; i < start_index + running_count; i++) {

            // printf("start index: %d\n", start_index);
            int temp = i;
            if (temp >= ready_index) temp %= ready_index;
            
            if (processPIDs[temp] > 0) {  
                kill(processPIDs[temp], SIGSTOP);  
                comp[i] += tSlice;
                // printf("Stopped job: %s (PID: %d)\n", readyQueue[i], processPIDs[i]);
            }

        }

        // this for loop keeps a check of all the processes which have been completely executed.
        int active_processes = 0;
        for (int i = 0; i < ready_index; i++) {

            if (processPIDs[i] > 0) {  

                int status;
                pid_t result = waitpid(processPIDs[i], &status, WNOHANG); 

                if (result == 0) active_processes++;
                else if (result == -1) perror("Error waiting for process"); // Error handling
                else if (WIFEXITED(status)) {
                    // the wait time is stored once the process is completely executed.
                    printf("Job %s (PID: %d) completed with exit status %d.\n", readyQueue[i], processPIDs[i], WEXITSTATUS(status));
                    waitTime[i] += globalTime - comp[i];
                    processPIDs[i] = 0;  
                    jobs_remaining--;  
                }
            }
        }

        if (process_index >= ready_index) process_index = 0;
        
        // printf("process_index: %d\n", process_index);
    }

    printf("All jobs in the readyQueue have been executed.\n\n");
    return 0;
}

int dummy_main(int argc, char **argv) {
    int ret = SimpleScheduler_main(argc, argv);  // Call your actual scheduler
    return ret;  // Return the result of the scheduler execution
}