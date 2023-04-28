
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
//

int remainingtime;
int startTime;
int runTime;

/*******************************************************************************
 *                      SIGNAL Handlers                                   *
 *******************************************************************************/
void Process_resume(int signum){
    startTime=getClk();
        // printf("process will continue now %d\n",getClk());
}
void Process_stop(int signum){
    // printf("process will stop now %d\n",getClk());
    if(signum==SIGTSTP){
    runTime=remainingtime;
    raise(SIGSTOP);
    }
}

/*******************************************************************************
 *                      main                                      *
 *******************************************************************************/
//

int main(int agrc, char * argv[])
{
    initClk();
    signal(SIGCONT,Process_resume);
    signal(SIGTSTP,Process_stop);
    //TODO it needs to get the remaining time from somewhere
     runTime =atoi(argv[1]);
    printf("first is %d\n",runTime);
    startTime=getClk();
    remainingtime=runTime;
    
     while (remainingtime>0)
    {   
        int elapse=getClk()-startTime;
        remainingtime = runTime -elapse;
        printf("Remaining time is %d\n",remainingtime);
    }
    
    destroyClk(false);
    exit(1);
    return 0;
}
