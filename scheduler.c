

/******************************************************************************
 *
 * Module: scheduler
 *
 * File Name: scheduler.c
 *
 * Description: C file for the scheduler
 *
 *This file handles all the requirments of the scheduler (refer to the PDF)
 *
 * Author: Mohamed Samir& Hamdy Salem
 *
 *******************************************************************************/

#include "headers.h"
#include "scheduler.h"

/*******************************************************************************
 *                     global variables                                    *
 *******************************************************************************/
SCHEDULING_ALGORITHM Scheduler;
int processNumbers; /*Number of processes*/
int timeChunk;      /*special variable for the case of Round Robin time slite*/
int msgq_id;        /*msg queue to communicate between the process_generator and the scheduler*/
msgbuff recProcess;
int rec_val;
int stat_loc;
PCB processTable[MAXSIZE];
int index = 1; // current index in the processTable
/*******************************************************************************
 *                      Functions definitions
 ********************************************************************************/

void Scheduler_init(int count, SCHEDULING_ALGORITHM s, int chunk)
{
    // int x=s;
    // printf("Algorithm =%d\n",x);
    // printf("Hello im the Scheduler Process\nNumber of process1 = %d\n",count);
    Scheduler = s;
    processNumbers = count;
    timeChunk = chunk;
    key_t key_id;
    key_id = ftok("keyfile", 65);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);
    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
}
/*
//void Scheduler_processStop();
/**
 * Descritption:checks if there is a new process arrived from the process
 * generator and if so it pushes it in your data structure passed
*/

/**
 * fork the process
 * return : the pid of the forked process
 */

int Scheduler_processStart(process_par *newProcess)
{
    int pid = fork();
    if (pid < 0)
    {
        perror("Error  in forking a Process\n");
        exit(-1);
    }
    if (pid == 0)
    { // child process
        char *runTime = malloc(sizeof(char));
        snprintf(runTime, sizeof(runTime), "%d", newProcess->runtime);
        char *processFile[] = {"./process", runTime, NULL}; // arguments for execv
        execv(processFile[0], processFile);                 // execute the child process
        fprintf(stderr, "Exec failed\n");                   // execv only returns if it fails
        exit(1);
    }
    processTable[index].arrival_time = newProcess->arrival_time;
    processTable[index].excutionTime = newProcess->runtime;
    processTable[index].startTime = getClk();
    processTable[index].lastTimeStartted = processTable[index].startTime;
    processTable[index].lastTimeStopped = 0;
    processTable[index].pid = pid;
    processTable[index].priority = newProcess->priority;
    processTable[index].remainingTime = processTable[index].excutionTime;
    processTable[index].status = RUNNING;
    processTable[index].waitingTime = 0;
    index++;
    return pid;
}




void Scheduler_recieveNewProcess(void *container)
{

    if (Scheduler == HPF || Scheduler == SRTN)
    {   
        // printf("Hello from the function\n");
        int *processQueue = (int *)container;
        int last = -1;
        recProcess.process.processNumber = -1;
        while(1)
        {
            last = recProcess.process.processNumber;
            rec_val = msgrcv(msgq_id, &recProcess, sizeof(recProcess.process), 0, IPC_NOWAIT);
            if (recProcess.process.processNumber == last)
            {
                break; // no process arrived
            }
            push(recProcess.process);
            Scheduler_processStart(&recProcess.process);
            kill(processTable[index-1].pid,SIGTSTP);
            // printf("%d\n",curSize);
            processNumbers--;
        } 
    }
    else
    {
        // RR
        /*To Do : implement the same logic*/
    }
}



void Scheduler_processResume(int processNumber)
{   
    int pid=processTable[processNumber].pid;
    kill(pid, SIGCONT);
    // To Do :Update the PCB
    processTable[processNumber].waitingTime += processTable[processNumber].lastTimeStopped - getClk();
    processTable[processNumber].status = RUNNING;
    processTable[processNumber].lastTimeStartted = getClk();
}

void Scheduler_processStop( int processNumber)
{   
    int pid=processTable[processNumber].pid;
    kill(pid, SIGTSTP);
    // To Do :Update The PCB
    processTable[processNumber].lastTimeStopped = getClk();
    processTable[processNumber].status = WAITING;
    processTable[processNumber].remainingTime -= processTable[processNumber].lastTimeStartted - getClk();
}

/*******************************************************************************
 *                      Main Algorithms definition                              *
 *******************************************************************************/

void Scheduler_HPF()  
{

    // int curSize = 0;
    // process_par heap[MAXSIZE];
    process_par p;
    int j=processNumbers;
    // just checking that every thing is okey.
    do
    {   
        // printf("Hello HPF\n");
        Scheduler_recieveNewProcess((void *)heap);
        if (curSize == 0)
        {
            continue;
        }
        p = top();
        pop();
        int i=p.processNumber;
        // call the function that will execute the process here.
        printf("Process Number %d will run at time %d\n",i,getClk());// these prints are just for the sense of testing 
        Scheduler_processResume(i);
        wait(&stat_loc);
         printf("Process Number %d finished at time %d\n",i,getClk());
        // printf("%d %d %d %d\n",p.processNumber,p.arrival_time,p.runtime,p.priority);
        // call the function that will execute the process hear.
        
        // check if any process has come ans put it in the PQ
        // ofcourse its not the real implementation of the logic but
        // it's just temporary and will be replaced later.
        j--;
    } while (j);
}

/*******************************************************************************
 *                      main function
 ********************************************************************************/
int main(int argc, char *argv[])
{
    initClk();
    Scheduler_init(atoi(argv[2]), atoi(argv[1]), atoi(argv[3]));
    printf("%d\n", processNumbers);
      recProcess.process.processNumber=-1;
    // // printf("Arguments=%d\nRecieved Chunk =%d\n",argc,timeChunk);
        switch(Scheduler){
        case HPF:
        //call the function of the HPF Algorithm
        
        Scheduler_HPF();
        break;
        case SRTN:
        //call the function of SRTN Algorithm
        break;
        case RR:
        //call the function of the RR Algorithm
        break;
    }
    // process_par p;
    // p.arrival_time = 1;
    // p.priority = 1;
    // p.processNumber = 1;
    // p.runtime = 6;
    // int x = Scheduler_processStart(&p);
    // sleep(2);
    // Scheduler_processStop(x, p.processNumber);
    // sleep(2);
    // // waitpid(x,&stat_loc,0);
    // // int last =-1;
    // // int lastID=-1;
    
    // Scheduler_processResume(x, p.processNumber);
    // waitpid(x, &stat_loc, 0);
    // sleep(4);
    // Scheduler_recieveNewProcess((void*)heap);
    // printf("%d\n",curSize);
    // int nowID=-2;
    // in main program we cannot have the number of processes in scheduler so relace it
    // replace it with while process id !=-1
    // while(processNumbers){

    // rec_val = msgrcv(msgq_id, &recProcess, sizeof(recProcess.process), 0, !IPC_NOWAIT);
    //   if(last==recProcess.process.processNumber){
    //         continue;
    //     }
    // printf("nowTime is %d\n",getClk());
    // printf("Process id =%d\nProcess arrivalTime=%d\nProcess runtime=%d\nProcess priority=%d\n",recProcess.process.processNumber,recProcess.process.arrival_time,recProcess.process.runtime,recProcess.process.priority);
    // processNumbers--;
    // last =recProcess.process.processNumber;
    // }

    destroyClk(true);
}
