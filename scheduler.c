

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
 *
 *******************************************************************************/

#include "headers.h"
#include "scheduler.h"
#include<math.h>

/*******************************************************************************
 *                      Functions definitions
 ********************************************************************************/

void Scheduler_init(int count, SCHEDULING_ALGORITHM s, int chunk)
{
    // int x=s;
    // printf("Algorithm =%d\n",x);
    // printf("Hello im the Scheduler Process\nNumber of process1 = %d\n",count);
    signal(SIGUSR1, Scheduler_processFinishHandler);
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
    Queue_init(&g_eventQueue);
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
        char *processNum = malloc(sizeof(char));
        snprintf(runTime, sizeof(runTime), "%d", newProcess->runtime);
        snprintf(processNum, sizeof(processNum), "%d", newProcess->processNumber);
        char *processFile[] = {"./process", runTime, processNum, NULL}; // arguments for execv
        execv(processFile[0], processFile);                             // execute the child process
        fprintf(stderr, "Exec failed\n");                               // execv only returns if it fails
        exit(1);
    }
    processTable[index].arrival_time = newProcess->arrival_time;
    processTable[index].excutionTime = newProcess->runtime;
    // processTable[index].startTime = getClk();
    // processTable[index].lastTimeStartted = 1;
    processTable[index].lastTimeStopped = newProcess->arrival_time;
    processTable[index].pid = pid;
    processTable[index].priority = newProcess->priority;
    processTable[index].remainingTime = processTable[index].excutionTime;
    processTable[index].status = NOTSTARTED;
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
        while (1)
        {
            last = recProcess.process.processNumber;
            rec_val = msgrcv(msgq_id, &recProcess, sizeof(recProcess.process), 0, IPC_NOWAIT);
            if (recProcess.process.processNumber == last)
            {
                break; // no process arrived
            }
            //  printf("nwo time %d and now currSize %d\n ",getClk(),curSize);
            //  sleep(2);
            push(recProcess.process,Scheduler);

            Scheduler_processStart(&recProcess.process);
            kill(processTable[index - 1].pid, SIGTSTP);
            // printf("%d\n",curSize);
            processNumbers--;
        }
       
    }
    else
    {
        // RR
        /*To Do : implement the same logic*/
        queue *processQueue = (queue *)container;
        int last = -1;
        recProcess.process.processNumber = -1;
        while (1)
        {
            last = recProcess.process.processNumber;
            rec_val = msgrcv(msgq_id, &recProcess, sizeof(recProcess.process), 0, IPC_NOWAIT);
            if (recProcess.process.processNumber == last)
            {
                break; // no process arrived
            }
            process_par *p = malloc(sizeof(process_par));
            *p = recProcess.process;
            Queue_push(processQueue, p);
            Scheduler_processStart(&recProcess.process);
            kill(processTable[index - 1].pid, SIGTSTP);
            // printf("%d\n",curSize);
            processNumbers--;
        }
    }
}

void Scheduler_processResume(int processNumber)
{
    int pid = processTable[processNumber].pid;
    kill(pid, SIGCONT);
    // To Do :Update the PCB
    processTable[processNumber].waitingTime -= processTable[processNumber].lastTimeStopped - getClk();
    processTable[processNumber].lastTimeStartted = getClk();
    Event* e=malloc(sizeof(Event));
    if(processTable[processNumber].status==NOTSTARTED){
        //first time run 
        processTable[processNumber].startTime=getClk();
        processTable[processNumber].lastTimeStartted=getClk();
        e->type=PROCESS_STARTED;
    }else{
         e->type=PROCESS_RESUMED;
    }
    processTable[processNumber].status = RUNNING;
    e->processNumber=processNumber;
    e->time=getClk();
    e->waitingTime=processTable[processNumber].waitingTime;
    e->remainingTime=processTable[processNumber].remainingTime;
    Queue_push(&g_eventQueue,e);
}

void Scheduler_processStop(int processNumber)
{
    int pid = processTable[processNumber].pid;
    kill(pid, SIGTSTP);
    // To Do :Update The PCB
    processTable[processNumber].lastTimeStopped = getClk();
    processTable[processNumber].status = WAITING;
    if(Scheduler!=SRTN){
    processTable[processNumber].remainingTime -= getClk() - processTable[processNumber].lastTimeStartted;
    }
        Event* e=malloc(sizeof(Event));
    e->type=PROCESS_STOPPED;
    e->processNumber=processNumber;
    e->time=getClk();
    e->waitingTime=processTable[processNumber].waitingTime;
    e->remainingTime=processTable[processNumber].remainingTime;
    Queue_push(&g_eventQueue,e);
}


void Scheduler_generateOutputFiles()
{
    FILE *Scheduler_file;
    Scheduler_file = fopen("Scheduler.log", "w");

    while (!isEmpty(&g_eventQueue))
    {
        Event e = *((Event *)Queue_pop(&g_eventQueue));
        int processNumber = e.processNumber;
        // TO DO: handle the total, remain and wait time after having the data needed
        switch (e.type)
        {
        case PROCESS_STARTED:
            fprintf(Scheduler_file, "At time %d process %d started total %d remain %d wait %d\n", e.time, processNumber, processTable[processNumber].excutionTime, e.remainingTime, e.waitingTime);
            break;

        case PROCESS_STOPPED:
            fprintf(Scheduler_file, "At time %d process %d stopped total %d remain %d wait %d\n", e.time, processNumber, processTable[processNumber].excutionTime, e.remainingTime, e.waitingTime);
        case PROCESS_RESUMED:
            fprintf(Scheduler_file, "At time %d process %d resumed total %d remain %d wait %d\n", e.time, processNumber, processTable[processNumber].excutionTime, e.remainingTime, e.waitingTime);
            break;
        case PROCESS_FINISHED:
            int turnAround = processTable[processNumber].finishTime - processTable[processNumber].arrival_time;
            double weightedTurnAround = ((double)turnAround) / (double)processTable[processNumber].excutionTime;
            int int_part=(int)weightedTurnAround;
            int dec_part=(int)round((weightedTurnAround-int_part)*100);
            processTable[processNumber].TA = turnAround;
            processTable[processNumber].WTA = weightedTurnAround;
            fprintf(Scheduler_file, "At time %d process %d finished total %d remain 0 wait %d TA %d WTA %d.%02d\n", e.time, processNumber, processTable[processNumber].excutionTime, e.waitingTime,turnAround, int_part,dec_part);
            break;
        }
    }

    fclose(Scheduler_file);
    // To DO: Handle scheduler.pref file
    //  Scheduler_file = fopen("Scheduler.perf", "w");
}

/*******************************************************************************
 *                      Main Algorithms definition                              *
 *******************************************************************************/

void Scheduler_HPF()
{

    // int curSize = 0;
    // process_par heap[MAXSIZE];
    process_par p;
    int j = processNumbers;
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
        pop(Scheduler);
        int i = p.processNumber;
        // call the function that will execute the process here.
        printf("Process Number %d will run at time %d\n", i, getClk()); // these prints are just for the sense of testing
        flag=false;
        Scheduler_processResume(i);
        while(!flag);
        printf("Process Number %d finished at time %d\n", i, getClk());
        j--;
    } while (j);
}

void Scheduler_RR()
{// 1 2 3 4 5
  queue left, ran;
  Queue_init(&left);
  Queue_init(&ran);

  int cnt = processNumbers;
  while (cnt)
  {
    queue tempQ;
    Queue_init(&tempQ);
    Scheduler_recieveNewProcess((void*)&tempQ);
    while (!isEmpty(&tempQ)){
      process_par p=*((process_par*)Queue_peek(&tempQ));
      printf("%d \n",p.processNumber);
      Queue_push(&left,Queue_peek(&tempQ));
      Queue_pop(&tempQ);
    }

    if (isEmpty(&left) && isEmpty(&ran))
      continue;
    else if (isEmpty(&left) && !isEmpty(&ran))
    {
      while (!isEmpty(&ran))
      {
        Queue_push(&left, Queue_peek(&ran));
        Queue_pop(&ran);
      }
    }

    process_par *toRun = (process_par *)Queue_peek(&left);
    Queue_pop(&left);
    int clk = getClk();
    int processNumber = toRun->processNumber;
    int quantum = (processTable[processNumber].remainingTime> timeChunk ? timeChunk :processTable[processNumber].remainingTime);
    int pid = processTable[processNumber].pid;
    int startTime = getClk();
    Scheduler_processResume(processNumber);
    while (getClk() - clk < quantum);
    Scheduler_processStop(processNumber);
    printf("At time %d Process %d ran\n",startTime,processNumber);
    printf("At time %d Process %d stopped remaining is %d\n",getClk(),processNumber, processTable[processNumber].remainingTime);
    if(processTable[processNumber].remainingTime>0){
      Queue_push(&ran, (void*)toRun);
    } else cnt--;
  }
}







 
void Scheduler_SRTN () {
    short isRun=false;
    int cnt = processNumbers;
    process_par lastRun;//here this is indicating the lastRun process
    while(curSize==0){
            Scheduler_recieveNewProcess((void*)heap);
    }
    process_par p=top();
    pop(Scheduler);
    isRun=true;
    lastRun=p;
    printf("Process Number %d will run at time %d\n",lastRun.processNumber,getClk());
    Scheduler_processResume(lastRun.processNumber);

    int lastClk=getClk();

do{
    if(curSize==0&&!isRun){
          Scheduler_recieveNewProcess((void*)heap);
          continue;
    }
     if(lastClk!=getClk()){
        lastClk=getClk();
        processTable[lastRun.processNumber].remainingTime--;
     }
    Scheduler_recieveNewProcess((void*)heap);
    if(curSize!=0){
    p=top();
    pop(Scheduler);
    }
    // printf("%d\n",processTable[lastRun].remainingTime);
    // printf("At time %d top is %d\n",getClk(),p.processNumber);
    if(isRun&&processTable[p.processNumber].remainingTime>=processTable[lastRun.processNumber].remainingTime){
        //continue running s
            // printf("%d\n",processTable[lastRun].remainingTime);
            // push(p,Scheduler);
            if(p.processNumber!=lastRun.processNumber)
            push(p,Scheduler);
        if(processTable[lastRun.processNumber].status==FINISHED){
            // pop(Scheduler);
            printf("Process Number %d finished at time %d\n",lastRun.processNumber,getClk());
            cnt--;
            isRun=false;
            
        }
       
    }
    else{
        //lastRun is not the min 
        //need to reSchedule
        if(processTable[lastRun.processNumber].status!=FINISHED){
        Scheduler_processStop(lastRun.processNumber);
         printf("Process Number %d stopped at time %d\n",lastRun.processNumber,getClk());
         push(lastRun,Scheduler);
        }
        
         lastRun=p;
         printf("Process Number %d will run at time %d\n",lastRun.processNumber,getClk());
        Scheduler_processResume(lastRun.processNumber);
        isRun=true;

    }
    } while (cnt);
}


/*******************************************************************************
 *                      SIGNAL Handlers
 ********************************************************************************/

void Scheduler_processFinishHandler(int signum)
{   
    wait(&stat_loc);
    flag =true;
    // pid_t pid;
    // int status;
    int pNumber=stat_loc>>8;
        // Handle terminated child process
        // if(!(status&0x00FF)){
            // printf("Something finished\n");
            // pNumber=status>>8;
            processTable[pNumber].status=FINISHED;
            processTable[pNumber].finishTime=getClk();
            Event* e =malloc(sizeof(Event));
            e->processNumber=pNumber;
            e->remainingTime=0;
            e->time=getClk();
            e->type=PROCESS_FINISHED;
            e->waitingTime=processTable[pNumber].waitingTime;
            Queue_push(&g_eventQueue,e);
}
        

/*******************************************************************************
 *                      main function
 ********************************************************************************/
int main(int argc, char *argv[])
{
    initClk();
    Scheduler_init(atoi(argv[2]), atoi(argv[1]), atoi(argv[3]));
    printf("%d\n", processNumbers);
    printf("%d\n", atoi(argv[3]));
    recProcess.process.processNumber = -1;
    // // // printf("Arguments=%d\nRecieved Chunk =%d\n",argc,timeChunk);
    switch (Scheduler)
    {
    case HPF:
        // call the function of the HPF Algorithm

        Scheduler_HPF();
        break;
    case SRTN:
        // call the function of SRTN Algorithm
        Scheduler_SRTN();
        break;
    case RR:
        // call the function of the RR Algorithm
        Scheduler_RR();
        break;
    }

    destroyClk(true);
}