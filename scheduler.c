

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
int indexG = 1;         // current index in the processTable
short lastProcessFlag; // a flag that indicate the last process that is to be recieved
// it is updated in the scheduler_recieveNewProcess
// when the scheduler recieves a process with id = -1 just an indicator

queue g_eventQueue;

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
  processTable[indexG].arrival_time = newProcess->arrival_time;
  processTable[indexG].excutionTime = newProcess->runtime;
  processTable[indexG].startTime = getClk();
  processTable[indexG].lastTimeStartted = processTable[indexG].startTime;
  processTable[indexG].lastTimeStopped = 0;
  processTable[indexG].pid = pid;
  processTable[indexG].priority = newProcess->priority;
  processTable[indexG].remainingTime = processTable[indexG].excutionTime;
  processTable[indexG].status = RUNNING;
  processTable[indexG].waitingTime = 0;
  indexG++;
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
      push(recProcess.process);
      Scheduler_processStart(&recProcess.process);
      kill(processTable[indexG - 1].pid, SIGTSTP);
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
      process_par* p=malloc(sizeof(process_par));
      *p=recProcess.process;
      Queue_push(processQueue, p);
      Scheduler_processStart(&recProcess.process);
      kill(processTable[indexG - 1].pid, SIGTSTP);
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
  processTable[processNumber].status = RUNNING;
  processTable[processNumber].lastTimeStartted = getClk();
}

void Scheduler_processStop(int processNumber)
{
  int pid = processTable[processNumber].pid;
  kill(pid, SIGTSTP);
  // To Do :Update The PCB
  processTable[processNumber].lastTimeStopped = getClk();
  processTable[processNumber].status = WAITING;
  processTable[processNumber].remainingTime -= getClk()-processTable[processNumber].lastTimeStartted;
}

// void Scheduler_generateOutputFiles()
// {
//     FILE *Scheduler_file;
//     Scheduler_file = fopen("Scheduler.log", "w");

//     while (!isEmpty(&g_eventQueue))
//     {
//         Event e = *((Event *)Queue_pop(&g_eventQueue));
//         int processNumber = e.processNumber;
//         // TO DO: handle the total, remain and wait time after having the data needed
//         switch (e.type)
//         {
//         case PROCESS_STARTED:
//             fprintf(Scheduler_file, "At time %d process %d started total %d remain %d wait %d\n", e.time, processNumber, processTable[processNumber].excutionTime, e.remainingTime, e.waitingTime);
//             break;

//         case PROCESS_STOPPED:
//             fprintf(Scheduler_file, "At time %d process %d stopped total %d remain %d wait %d\n", e.time, processNumber, processTable[processNumber].excutionTime, e.remainingTime, e.waitingTime);
//         case PROCESS_RESUMED:
//             fprintf(Scheduler_file, "At time %d process %d resumed total %d remain %d wait %d\n", e.time, processNumber, processTable[processNumber].excutionTime, e.remainingTime, e.waitingTime);
//             break;
//         case PROCESS_FINISHED:
//             int turnAround = processTable[processNumber].finishTime - processTable[processNumber].arrival_time;
//             float weightedTurnAround = ((float)turnAround) / processTable[processNumber].excutionTime;
//             float near = round(weightedTurnAround * 100) / 100;
//             processTable[processNumber].TA = turnAround;
//             processTable[processNumber].WTA = weightedTurnAround;
//             fprintf(Scheduler_file, "At time %d process %d finished total %d remain 0 wait %d TA %d WTA %d\n", e.time, processNumber, processTable[processNumber].excutionTime, e.waitingTime, near);
//             break;
//         }

//     }

//     fclose(Scheduler_file);
//     // To DO: Handle scheduler.pref file
//     //  Scheduler_file = fopen("Scheduler.perf", "w");
// }

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
    pop();
    int i = p.processNumber;
    // call the function that will execute the process here.
    printf("Process Number %d will run at time %d\n", i, getClk()); // these prints are just for the sense of testing
    Scheduler_processResume(i);
    wait(&stat_loc);
    printf("Process Number %d finished at time %d\n", i, getClk());
    j--;
  } while (j);
}

void Scheduler_RR()
{// 1 2 3 4 5
  queue left, ran;
  Queue_init(&left);
  Queue_init(&ran);

  while (true)
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
    printf("Now will run process %d\n",toRun->processNumber);
    Queue_pop(&left);
    int clk = getClk();
    int processNumber = toRun->processNumber;
    int quantum = (processTable[processNumber].remainingTime> timeChunk ? timeChunk :processTable[processNumber].remainingTime);
    int pid = processTable[processNumber].pid;
    printf("At time %d Process %d ran remaining is %d\n",getClk(),processNumber,processTable[processNumber].remainingTime);
    Scheduler_processResume(processNumber);
    while (getClk() - clk < quantum);
    Scheduler_processStop(processNumber);
    printf("stopped at %d",getClk());
    if(processTable[processNumber].remainingTime>0){
      Queue_push(&ran, (void*)toRun);
    }
  }
}

/*******************************************************************************
 *                      main function
 ********************************************************************************/
int main(int argc, char *argv[])
{
  initClk();
  Scheduler_init(atoi(argv[2]), atoi(argv[1]), atoi(argv[3]));
  printf("Number: %d\n", processNumbers);
  printf("%d\n",atoi(argv[3]));
  recProcess.process.processNumber = -1;
  // // printf("Arguments=%d\nRecieved Chunk =%d\n",argc,timeChunk);
  switch (Scheduler)
  {
    case HPF:
      // call the function of the HPF Algorithm

      Scheduler_HPF();
      break;
    case SRTN:
      // call the function of SRTN Algorithm
      break;
    case RR:
      // call the function of the RR Algorithm
      Scheduler_RR();
      break;
  }

// queue q;
// Queue_init(&q);
// for(int i=0;i<5;i++){
//     int* val = malloc(sizeof(int));
//     *val = i;
//     Queue_push(&q, val);
// }

// for (int i=0;i<5;i++){
//     printf("%d\n",* ((int*)Queue_peek(&q)));
//     free(Queue_pop(&q));
// }
// Queue_destroy(&q);

  // testing the RR recieve new process
  // queue q;
  // Queue_init(&q);
  // sleep(4);
  // Scheduler_recieveNewProcess((void*)(&q));
  // struct node* ptr;
  // ptr=q.front;
  // int cnt=0;
  // while(ptr!=q.rear){
  //     cnt++;
  //     ptr++;
  // }
  // printf("Recieved processes aree %d\n",cnt);
  // Queue_destroy(&q);

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
