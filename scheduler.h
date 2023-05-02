
 /******************************************************************************
 *
 * Module: scheduler
 *
 * File Name: scheduler.h
 *
 * Description: header file for the scheduler
 *
 *******************************************************************************/

#ifndef scheduler
#define scheduler
#include "headers.h"
#define MAXSIZE 200
#define NOMOREPROCESS -1

/*******************************************************************************
 *                         Types Declaration                                   *
 *******************************************************************************/

typedef enum {RUNNING, WAITING, FINISHED, NOTSTARTED} PROCESS_STATUS;
typedef enum {PROCESS_STARTED, PROCESS_STOPPED, PROCESS_RESUMED, PROCESS_FINISHED} EVENT_TYPE;

typedef struct {
    int pid;
    PROCESS_STATUS status;
    int arrival_time;
    int excutionTime;
    int remainingTime;
    int waitingTime;
    int startTime;
    int finishTime;
    int priority;
    int lastTimeStopped; //indicator to calculate the waiting time for each process
    int lastTimeStartted;
    int TA;
    int WTA;
} PCB;

typedef struct {
    int processNumber;
    EVENT_TYPE type;
    int waitingTime;
    int remainingTime;
    int time;
} Event;

int curSize = 0;
process_par heap[MAXSIZE];

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
short flag=false;
int index = 1;         // current index in the processTable

short lastProcessFlag; // a flag that indicate the last process that is to be received
// it is updated in the scheduler_receiveNewProcess
// when the scheduler receives a process with id = -1 just an indicator
queue g_eventQueue;

/*******************************************************************************
 *                     priority Queue implementation                                     *
 *******************************************************************************/

// auxiliary fucnctions

static int parent(int i) 
{
    return (i-1)/2;
}

static int left(int i)
{
    return 2*i+1;
}

static int right(int i)
{
    return 2*i+2;
}

//  1 --> 1 >  2
//  0 --> 1 == 2
// -1 --> 1 <  2
static int compare(int idx1, int idx2, SCHEDULING_ALGORITHM s)
{
    int val1 = 0, val2 = 0;

    if (s == HPF) {
        val1 = heap[idx1].priority;
        val2 = heap[idx2].priority;
    } else if (s == SRTN) {
        val1 = processTable[idx1].remainingTime;
        val2 = processTable[idx2].remainingTime;
    }

    if (val1 > val2) return 1;
    if (val1 == val2) return 0;
    if (val1 < val2) return -1;
}

static void heapify(int root, SCHEDULING_ALGORITHM s)
{
    int l = left(root), r = right(root);
    int mn = root;
    if (l < curSize && compare(l, mn, s) == -1) mn = l;
    if (r < curSize && compare(r, mn, s) == -1) mn = r;

    if (mn != root) {
        process_par tmp = heap[root];
        heap[root] = heap[mn];
        heap[mn] = tmp;
        heapify(mn, s);
    }
}

// main PQ functions
process_par top()
{
    return heap[0];
}

void pop(SCHEDULING_ALGORITHM s)
{
    if (curSize == 1) {
        curSize--;
        return;
    }
    heap[0] = heap[curSize-1];
    curSize--;
    heapify(0, s);
}

void push(process_par newP, SCHEDULING_ALGORITHM s)
{
    curSize++;
    heap[curSize-1] = newP;
    int pos = curSize-1;
    while(pos > 0 && compare(pos, parent(pos), s) == -1) {
        process_par tmp = heap[parent(pos)];
        heap[parent(pos)] = heap[pos];
        heap[pos] = tmp;
        pos = parent(pos);
    }
}

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description : Function to initalize the Scheduler 
 inputs: 1-Number of processes
         2-Scheduling algorithm 
 */
void Scheduler_init(int count,SCHEDULING_ALGORITHM s,int chunk);

/**
 * Descritption:checks if there is a new process arrived from the process 
 * generator and if so it pushes it in your data structure passed 
*/
void Scheduler_recieveNewProcess(void * container);

/**
 * Fork a new process and give its parameters 
 * called when we run the proecss for the first time
*/
int Scheduler_processStart(process_par* newProcess);

/**
 * Resume a stopped Process
 * inpts : the pid of the process  (pid is the id that returnd when forking a process)
 * don't conflict between the pid and the id of the text file
*/
void Scheduler_processResume(int processNumber);

/**
 * stop a running Process
 * inpts : the pid of the process  (pid is the id that returnd when forking a process)
 * don't conflict between the pid and the id of the text file
*/
void Scheduler_processStop(int processNumber);

void Scheduler_generateOutputFiles();

void Scheduler_processFinishHandler(int signum);

/*******************************************************************************
 *                      Main Algorithms                                   *
 *******************************************************************************/

void Scheduler_HPF();
void Scheduler_SRTN();
void Scheduler_RR();

#endif

