
 /******************************************************************************
 *
 * Module: process
 *
 * File Name: process.c
 *
 * Description: C file for the process
 * 
 *This file handles the process definition 
 *
 * Author: Mohamed Samir
 *
 *******************************************************************************/

#include"headers.h"

/*******************************************************************************
 *                      global variabels                                    *
 *******************************************************************************/

int remainingtime;
int startTime;
int runTime;
int processNumber;

/*******************************************************************************
 *                      SIGNAL Handlers                                   *
 *******************************************************************************/

void Process_resume(int signum){
    startTime = getClk();
}

void Process_stop(int signum){
    if (signum == SIGTSTP){
        runTime = remainingtime;
        raise(SIGSTOP);
    }
}

/*******************************************************************************
 *                      main                                      *
 *******************************************************************************/

int main(int argc, char * argv[])
{
    initClk();
    signal(SIGCONT,Process_resume);
    signal(SIGTSTP,Process_stop);

    runTime = atoi(argv[1]);
    processNumber = atoi(argv[2]);
    startTime = getClk();
    remainingtime = runTime;

    while (remainingtime > 0)
    {
        int elapse = getClk()- startTime;
        remainingtime = runTime - elapse;
    }

    destroyClk(false);
    kill(getppid(),SIGUSR1);
    exit(processNumber);
}



