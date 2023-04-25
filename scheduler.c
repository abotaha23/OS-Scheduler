

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
int processNumbers;/*Number of processes*/
int timeChunk;/*special variable for the case of Round Robin time slite*/
int msgq_id;/*msg queue to communicate between the process_generator and the scheduler*/
/*******************************************************************************
 *                      Functions definitions                                    
********************************************************************************/

void Scheduler_init(int count,SCHEDULING_ALGORITHM s,int chunk){
// int x=s;
// printf("Algorithm =%d\n",x);
// printf("Hello im the Scheduler Process\nNumber of process1 = %d\n",count);
    Scheduler=s;
    processNumbers=count;
    timeChunk=chunk;
     key_t key_id;
    key_id = ftok("keyfile", 65);
     msgq_id = msgget(key_id, 0666 | IPC_CREAT);
      if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }

}





/*******************************************************************************
 *                      main function                                    
********************************************************************************/
int main(int argc, char *argv[])
{   
    int rec_val;
    initClk();
    Scheduler_init(atoi(argv[2]),atoi(argv[1]),atoi(argv[3]));
    // printf("Arguments=%d\nRecieved Chunk =%d\n",argc,timeChunk);
        switch(Scheduler){
        case HPF:
        //call the function of the HPF Algorithm
        break;
        case SRTN:
        //call the function of SRTN Algorithm
        break;
        case RR:
        //call the function of the RR Algorithm 
        break;
    }   
    msgbuff recProcess;
    recProcess.process.process_id=-1;
    int last =-1;
    // int lastID=-1;
    // int nowID=-2;
// in main program we cannot have the number of processes in scheduler so relace it 
//replace it with while process id !=-1
    while(processNumbers){  
    rec_val = msgrcv(msgq_id, &recProcess, sizeof(recProcess.process), 7, !IPC_NOWAIT);
    if(recProcess.process.process_id!=last){
    printf("nowTime is %d\n",getClk());
    printf("Process id =%d\nProcess arrivalTime=%d\nProcess runtime=%d\nProcess priority=%d\n",recProcess.process.process_id,recProcess.process.arrival_time,recProcess.process.runtime,recProcess.process.priority);
    processNumbers--;
    last =recProcess.process.process_id;
    }
    }
    destroyClk(true);
}