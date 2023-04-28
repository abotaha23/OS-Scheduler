 /******************************************************************************
 *
 * Module: scheduler
 *
 * File Name: scheduler.h
 *
 * Description: header file for the scheduler
 *
 * Author: Mohamed Samir
 *
 *******************************************************************************/

#ifndef scheduler
#define scheduler
#include "headers.h"
#define MAXSIZE 200
/*******************************************************************************
 *                         Types Declaration                                   *
 *******************************************************************************/
typedef enum{RUNNING,WAITING,FINISHED}PROCESS_STATUS;


typedef struct {
int pid;//process pid that returned from the fork
PROCESS_STATUS status;
int arrival_time;
int excutionTime;
int remainingTime;
int waitingTime;
int startTime;
int finishTime;
int priority;
int lastTimeStopped;//indicator to calculate the waiting time for each process
int lastTimeStartted;
}PCB;

// 2 4 1 0 5 
//7 8 10 11
//Queue<processPar> 





int curSize = 0;
process_par heap[MAXSIZE];


// priority queue functions



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

// 1--> 1 > 2
// 0--> 1 == 2
// -1--> 1 < 2
static int compare(int idx1, int idx2)
{
    int val1 = heap[idx1].priority;
    int val2 = heap[idx2].priority;

    if (val1 > val2) return 1;
    if (val1 == val2) return 0;
    if (val1 < val2) return -1;
}

static void heapify(int root)
{
    int l = left(root), r = right(root);
    int mn = root;
    if (l < curSize && compare(l, root) == -1) mn = l;
    if (r < curSize && compare(r, root) == -1) mn = r;

    if (mn != root) {
        process_par tmp = heap[root];
        heap[root] = heap[mn];
        heap[mn] = tmp;
        heapify(mn);
    }
}

// main functions

process_par top()
{
    return heap[0];
}

void pop()
{
    if (curSize == 1) {
        curSize--;
        return;
    }
    heap[0] = heap[curSize-1];
    curSize--;
    heapify(0);
}

void push(process_par newP)
{
    curSize++;
    heap[curSize-1] = newP;
    int pos = curSize-1;
    while(pos > 0 && compare(pos, parent(pos)) == -1) {
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

*/
//void Scheduler_processStop();
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

/*******************************************************************************
 *                      Main Algorithms                                   *
 *******************************************************************************/

void Scheduler_HPF();
// void SRTN();
// void RR(int chunck);



#endif


