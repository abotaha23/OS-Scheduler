

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
#include <math.h>

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
    FirstFit_init();
    buddy_goBuild(0, 10, 0);
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
    processTable[newProcess->processNumber].arrival_time = newProcess->arrival_time;
    processTable[newProcess->processNumber].excutionTime = newProcess->runtime;
    // processTable[index].startTime = getClk();
    // processTable[index].lastTimeStartted = 1;
    processTable[newProcess->processNumber].lastTimeStopped = newProcess->arrival_time;
    processTable[newProcess->processNumber].pid = pid;
    processTable[newProcess->processNumber].priority = newProcess->priority;
    processTable[newProcess->processNumber].remainingTime = processTable[index].excutionTime;
    processTable[newProcess->processNumber].status = NOTSTARTED;
    processTable[newProcess->processNumber].waitingTime = 0;
    processTable[newProcess->processNumber].memSize = recProcess.process.memSize;
    processTable[newProcess->processNumber].waitingTime += getClk() - newProcess->arrival_time;
    // printf("%dzzz\n", processTable[newProcess->processNumber].memSize);
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
            short flag = false; // cannot allocate
            last = recProcess.process.processNumber;
            rec_val = msgrcv(msgq_id, &recProcess, sizeof(recProcess.process), 0, IPC_NOWAIT);
            if (recProcess.process.processNumber == last)
            {
                break; // no process arrived
            }
            //  printf("nwo time %d and now currSize %d\n ",getClk(),curSize);
            //  sleep(2);

            // we need to check wheter we have enough memory or not

            printf("nowTime %d Recieved process %d memsize is %d\n", getClk(), recProcess.process.processNumber, recProcess.process.memSize);

            processTable[recProcess.process.processNumber].memSize = recProcess.process.memSize;
            if (recProcess.process.processNumber == 3)
            {
                memoo3 = recProcess.process.memSize;
            }
            switch (memoryAlgorithm)
            {
            case FIRSTFIT:
                // try and allocate the memory for it
                // printf("Outside rec size is%d\n",recProcess.process.memSize);
                flag = FirstFit_allocateNewProcess(recProcess.process.processNumber, recProcess.process.memSize);

                break;
            case BUDDYMEMORY:
                // call the function of the allocation
                int res = buddy_allocate(0, recProcess.process.memSize, recProcess.process.processNumber);
                flag = res != -1;
                processTable[recProcess.process.processNumber].memStart = res;

                break;
            }
            // before pushing need to check wheter it was allocated or not
            if (flag)
            {
                printf("Process %d allocated memory start %d end %d\n", recProcess.process.processNumber, processTable[recProcess.process.processNumber].memStart, processTable[recProcess.process.processNumber].memStart + recProcess.process.memSize - 1);
                push(recProcess.process, Scheduler);
                Scheduler_processStart(&recProcess.process);
                kill(processTable[recProcess.process.processNumber].pid, SIGTSTP);
                // printf("%d\n",curSize);
                // printf("%d ks\n",memory[0]);
            }
            else
            {
                printf("The process %d cannt allocate memory and pushed to waitinglist\n", recProcess.process.processNumber);

                waitingProcesses[recProcess.process.processNumber] = recProcess.process;
                waitingMemoryList[recProcess.process.processNumber] = PROCESS_WAITING;
            }
            processNumbers--;
        }
    }
    else
    {
        // RR
        /*To Do : implement the same logic*/
        short flag = false;
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
            processTable[recProcess.process.processNumber].memSize = recProcess.process.memSize;
            process_par *p = malloc(sizeof(process_par));
            *p = recProcess.process;
            switch (memoryAlgorithm)
            {
            case FIRSTFIT:
                // try and allocate the memory for it
                // printf("Outside rec size is%d\n",recProcess.process.memSize);
                flag = FirstFit_allocateNewProcess(recProcess.process.processNumber, recProcess.process.memSize);

                break;
            case BUDDYMEMORY:
                // call the function of the allocation
                int res = buddy_allocate(0, recProcess.process.memSize, recProcess.process.processNumber);
                flag = res != -1;
                processTable[recProcess.process.processNumber].memStart = res;

                break;
            }
            // before pushing need to check wheter it was allocated or not
            if (flag)
            {
                printf("Process %d allocated memory start %d end %d\n", recProcess.process.processNumber, processTable[recProcess.process.processNumber].memStart, processTable[recProcess.process.processNumber].memStart + recProcess.process.memSize - 1);
                Queue_push(processQueue, p);
                Scheduler_processStart(&recProcess.process);
                kill(processTable[recProcess.process.processNumber].pid, SIGTSTP);
                // printf("%d\n",curSize);
                // printf("%d ks\n",memory[0]);
            }
            else
            {
                printf("The process %d cannt allocate memory and pushed to waitinglist\n", recProcess.process.processNumber);

                waitingProcesses[recProcess.process.processNumber] = recProcess.process;
                waitingMemoryList[recProcess.process.processNumber] = PROCESS_WAITING;
            }
        }

        processNumbers--;
    }
}

void Scheduler_processResume(int processNumber)
{
    int pid = processTable[processNumber].pid;
    kill(pid, SIGCONT);
    // To Do :Update the PCB
    processTable[processNumber].waitingTime -= processTable[processNumber].lastTimeStopped - getClk();
    processTable[processNumber].lastTimeStartted = getClk();
    Event *e = malloc(sizeof(Event));
    if (processTable[processNumber].status == NOTSTARTED)
    {
        // first time run
        processTable[processNumber].startTime = getClk();
        processTable[processNumber].lastTimeStartted = getClk();
        e->type = PROCESS_STARTED;
    }
    else
    {
        e->type = PROCESS_RESUMED;
    }
    processTable[processNumber].status = RUNNING;
    e->processNumber = processNumber;
    e->time = getClk();
    e->waitingTime = processTable[processNumber].waitingTime;
    e->remainingTime = processTable[processNumber].remainingTime;
    Queue_push(&g_eventQueue, e);
    // printf("ll\n");
}

void Scheduler_processStop(int processNumber)
{
    int pid = processTable[processNumber].pid;
    kill(pid, SIGTSTP);
    // To Do :Update The PCB
    processTable[processNumber].lastTimeStopped = getClk();
    processTable[processNumber].status = WAITING;
    if (Scheduler != SRTN)
    {
        processTable[processNumber].remainingTime -= getClk() - processTable[processNumber].lastTimeStartted;
    }
    Event *e = malloc(sizeof(Event));
    e->type = PROCESS_STOPPED;
    e->processNumber = processNumber;
    e->time = getClk();
    e->waitingTime = processTable[processNumber].waitingTime;
    e->remainingTime = processTable[processNumber].remainingTime;
    Queue_push(&g_eventQueue, e);
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
            break;
        case PROCESS_RESUMED:
            fprintf(Scheduler_file, "At time %d process %d resumed total %d remain %d wait %d\n", e.time, processNumber, processTable[processNumber].excutionTime, e.remainingTime, e.waitingTime);
            break;
        case PROCESS_FINISHED:
            int turnAround = processTable[processNumber].finishTime - processTable[processNumber].arrival_time;
            double weightedTurnAround = ((double)turnAround) / (double)processTable[processNumber].excutionTime;
            int int_part = (int)weightedTurnAround;
            int dec_part = (int)round((weightedTurnAround - int_part) * 100);
            processTable[processNumber].TA = turnAround;
            processTable[processNumber].WTA = weightedTurnAround;
            fprintf(Scheduler_file, "At time %d process %d finished total %d remain 0 wait %d TA %d WTA %d.%02d\n", e.time, processNumber, processTable[processNumber].excutionTime, e.waitingTime, turnAround, int_part, dec_part);
            break;
        }
    }

    fclose(Scheduler_file);
    // To DO: Handle scheduler.pref file
    //  Scheduler_file = fopen("Scheduler.perf", "w");
}

/*******************************************************************************
 *                    First_Fit Algorithm functuins definitions                                  *
 *******************************************************************************/
void FirstFit_init()
{

    // initialize the memory
    for (int i = 0; i < TOTAL_MEMORY_SIZE; i++)
    {
        memory[i] = NOTALLOCATED;
        waitingMemoryList[i] = PROCESS_NOTWAITING;
    }
}

short FirstFit_allocateNewProcess(int processNumber, int processSize)
{
    short flag = false;
    int i, j;
    for (i = 0, j = 0; i < TOTAL_MEMORY_SIZE; i = j + 1)
    {
        if (j < i)
            j = i;
        if (memory[i] == ALLOCATED)
        {
            continue;
        }
        while (j < TOTAL_MEMORY_SIZE && memory[j] == NOTALLOCATED)
            j++;
        // printf("%d\n",j);
        int len = j - i;
        // printf("Len is %d\n",len);
        //  printf("%d %d\n",j,i);
        if (len < processSize)
            continue;
        printf("%d %d\n ", processSize, len);
        printf("A7A ya sooree %d\n", processNumber);
        for (int it = i; it < i + processSize; it++)
        {
            memory[it] = ALLOCATED;
            // printf("hahahahaha\n");
        }
        // printf("Flag is set\n");

        flag = true;
        break;
    }
    printf("Awo");
    fflush(stdout);
    if (flag)
        processTable[processNumber].memStart = i;
    return flag;
}
short FirstFit_deAllocateProcess(int processNumber)
{
    // if(processNumber==3){
    //     processTable[processNumber].memSize=memoo3;
    // }
    int start = processTable[processNumber].memStart;
    printf("bro the meSize is %d\n", processTable[processNumber].memSize);
    // printf("%d %d \n",start,processTable[processNumber].memSize);
    for (int i = start; i < start + processTable[processNumber].memSize; i++)
    {
        //  printf("%d\n",i);
        memory[i] = NOTALLOCATED;
    }
    printf("process %d will deallocate its memory from %d to %d\n", processNumber, start, start + processTable[processNumber].memSize - 1);

    return true;
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
        flag = false;
        Scheduler_processResume(i);

        while (!flag)
            ;
        printf("ll\n");
        printf("Process Number %d finished at time %d\n", i, getClk());
        j--;
    } while (j);
}

void Scheduler_RR()
{ // 1 2 3 4 5
signal(SIGUSR1, Scheduler_processFinishHandler);
    queue left, ran;
    Queue_init(&left);
    Queue_init(&ran);

    int cnt = processNumbers;
    while (cnt)
    {

        Queue_init(&tempQ);
        Scheduler_recieveNewProcess((void *)&tempQ);
        while (!isEmpty(&tempQ))
        {
            process_par p = *((process_par *)Queue_peek(&tempQ));
            printf("%d \n", p.processNumber);
            Queue_push(&left, Queue_peek(&tempQ));
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
        int quantum = (processTable[processNumber].remainingTime > timeChunk ? timeChunk : processTable[processNumber].remainingTime);
        int pid = processTable[processNumber].pid;
        int startTime = getClk();
        Scheduler_processResume(processNumber);
        while (getClk() - clk < quantum)
            ;
        Scheduler_processStop(processNumber);
        printf("At time %d Process %d ran\n", startTime, processNumber);
        printf("At time %d Process %d stopped remaining is %d\n", getClk(), processNumber, processTable[processNumber].remainingTime);
        if (processTable[processNumber].remainingTime > 0)
        {
            Queue_push(&ran, (void *)toRun);
        }
        else
            cnt--;
    }
    Queue_destroy(&left);
    Queue_destroy(&ran);
}

void Scheduler_SRTN()
{
    short isRun = false;
    int cnt = processNumbers;
    process_par lastRun; // here this is indicating the lastRun process
    while (curSize == 0)
    {
        Scheduler_recieveNewProcess((void *)heap);
    }
    process_par p = top();
    pop(Scheduler);
    isRun = true;
    lastRun = p;
    printf("Process Number %d will run at time %d\n", lastRun.processNumber, getClk());
    Scheduler_processResume(lastRun.processNumber);

    int lastClk = getClk();

    do
    {
        if (curSize == 0 && !isRun)
        {
            Scheduler_recieveNewProcess((void *)heap);
            continue;
        }
        if (lastClk != getClk())
        {
            lastClk = getClk();
            processTable[lastRun.processNumber].remainingTime--;
        }
        Scheduler_recieveNewProcess((void *)heap);
        if (curSize != 0)
        {
            p = top();
            pop(Scheduler);
        }
        // printf("%d\n",processTable[lastRun].remainingTime);
        // printf("At time %d top is %d\n",getClk(),p.processNumber);
        if (isRun && processTable[p.processNumber].remainingTime >= processTable[lastRun.processNumber].remainingTime)
        {
            // continue running s
            //  printf("%d\n",processTable[lastRun].remainingTime);
            //  push(p,Scheduler);
            if (p.processNumber != lastRun.processNumber)
                push(p, Scheduler);
            if (processTable[lastRun.processNumber].status == FINISHED)
            {
                // pop(Scheduler);
                printf("Process Number %d finished at time %d\n", lastRun.processNumber, getClk());
                cnt--;
                isRun = false;
            }
        }
        else
        {
            // lastRun is not the min
            // need to reSchedule
            if (processTable[lastRun.processNumber].status != FINISHED)
            {
                Scheduler_processStop(lastRun.processNumber);
                printf("Process Number %d stopped at time %d\n", lastRun.processNumber, getClk());

                push(lastRun, Scheduler);
            }

            lastRun = p;
            printf("Process Number %d will run at time %d\n", lastRun.processNumber, getClk());
            Scheduler_processResume(lastRun.processNumber);
            fflush(stdout);
            printf("A&AAAAAT");
            fflush(stdout);
            isRun = true;
        }
    } while (cnt);
}

/*******************************************************************************
 *                      SIGNAL Handlers
 ********************************************************************************/

void Scheduler_processFinishHandler(int signum)
{
    printf("First");
    fflush(stdout);
    wait(&stat_loc);
    flag = true;
    // pid_t pid;
    // int status;
    int pNumber = stat_loc >> 8;
    // Handle terminated child process
    // if(!(status&0x00FF)){
    // printf("Something finished\n");
    // pNumber=status>>8;
    processTable[pNumber].status = FINISHED;
    processTable[pNumber].finishTime = getClk();
    Event *e = malloc(sizeof(Event));
    e->processNumber = pNumber;
    e->remainingTime = 0;
    e->time = getClk();
    e->type = PROCESS_FINISHED;
    e->waitingTime = processTable[pNumber].waitingTime;
    printf("ain here ");
    fflush(stdout);
    Queue_push(&g_eventQueue, e);

    // deallocate the process
    switch (memoryAlgorithm)
    {
    case FIRSTFIT:
        FirstFit_deAllocateProcess(pNumber);

        for (int i = 0; i < MAX_PROCESSNUMBER; i++)
        {
            if (waitingMemoryList[i] == PROCESS_WAITING)
            {

                if (FirstFit_allocateNewProcess(i, processTable[i].memSize))
                {
                    printf("process %d will allocate the memory and be pushed to queue from %d to %d\n", i, processTable[i].memStart, processTable[i].memStart + processTable[i].memSize - 1);
                    waitingMemoryList[i] = PROCESS_NOTWAITING;
                    if (Scheduler == HPF || Scheduler == SRTN)
                    {
                        push(waitingProcesses[i], Scheduler);
                    }
                    else
                    {
                        Queue_push(&tempQ, &waitingProcesses[i]);
                    }
                    Scheduler_processStart(&waitingProcesses[i]);
                }
            }
        }
        break;
    case BUDDYMEMORY:
        // call your deAllocation funciton

        buddy_deallocate(0, pNumber);

        // call your deAllocation funciton
        for (int i = 0; i < MAX_PROCESSNUMBER; i++)
        {
            if (waitingMemoryList[i] == PROCESS_WAITING)
            {

                if (buddy_allocate(0, waitingProcesses[i].memSize, i) != -1)
                {
                    printf("process %d will allocate the memory and be pushed to queue\n", pNumber);
                    waitingMemoryList[i] = PROCESS_NOTWAITING;
                    if (Scheduler == HPF || Scheduler == SRTN)
                    {
                        push(waitingProcesses[i], Scheduler);
                    }
                    else
                    {
                        Queue_push(&tempQ, &waitingProcesses[i]);
                    }
                    Scheduler_processStart(&waitingProcesses[i]);
                }
            }
        }

        // check after if there is a memory to allocate the waiting
    }
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
    Scheduler_generateOutputFiles();

    destroyClk(true);
}