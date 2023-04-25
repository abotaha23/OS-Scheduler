

 /******************************************************************************
 *
 * Module: process_generator
 *
 * File Name: process_generator.c
 *
 * Description: C file for the process generator
 *
 *This file handles all the requirments of the process generator (refer to the PDF)
 *
 * Author: Mohamed Samir& Hamdy Salem
 *
 *******************************************************************************/

#include "headers.h"

#include"process_generator.h"


/*******************************************************************************
 *                      Functions definitions                                    *
 *******************************************************************************/
/*
 * Description : Function to clear all resources

 */

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
	 msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);
	 //To Do : call the Destructor of the process queue
	 Queue_destroy(&process_queue);
}

/*******************************************************************************
 *                      main                                      *
 *******************************************************************************/
//
int main(int argc, char * argv[])
{

	//initialization of the msg queue
	 key_t key_id;
	 int send_val;
	 key_id = ftok("keyfile", 65);
	 msgq_id = msgget(key_id, 0666 | IPC_CREAT);

	    if (msgq_id == -1)
	    {
	        perror("Error in create");
	        exit(-1);
	    }



	int stat_loc;
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    FILE *input_file;

    input_file = fopen("processes.txt","r");
    if (!input_file) {
        printf("couldn't open the input file \"processes.txt\"\n");
        exit(-1);
    }

    // skipping the first line
    fscanf(input_file,"%*[^\n]\n");

     process_par p;

    processNumbers=0;
            // printf("%d %d %d %d",p.process_id,p.arrival_time,p.runtime,p.priority);
    while (fscanf(input_file, "%d %d %d %d", &p.process_id, &p.arrival_time, &p.runtime, &p.priority) == 4){
        process_par *ptr=malloc(sizeof(process_par));
        *ptr=p;
        Queue_push(&process_queue,(void*)ptr);
        printf("\n");
        processNumbers++;
    }

    fclose(input_file);



    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    printf("Choose one of the following algorithms to use:\n"
           "0.Non-preemptive Highest Priority First (HPF).\n"
           "1.Shortest Remaining time Next (SRTN).\n"
           "2.Round Robin (RR).\n");

    SCHEDULING_ALGORITHM scheduling_algorithm = 0;
    int x;
    scanf("%d",&x);
    scheduling_algorithm=x;

    int time_chunk = -1;
    if (scheduling_algorithm == RR) {
        printf("Enter the time chunk for each process to run RR algorithm:");
        scanf("%d",&time_chunk);
    }
    // 3. Initiate and create the scheduler and clock processes.
    printf("Number of proccess =%d\nAlgorithm :%d\n",processNumbers,scheduling_algorithm);
    int Scheduler_pid=fork();
    int Clock_pid=fork();
    while (Clock_pid < 0) { // error occurred
    	Clock_pid=fork();
           }
		if (Clock_pid == 0) { // child process
       char *Clock_file[] = {"./clk", NULL}; // arguments for execv
       execv(Clock_file[0], Clock_file); // execute the child process
       fprintf(stderr, "Exec failed\n"); // execv only returns if it fails
       exit(1);
   }

    while (Scheduler_pid < 0) { // error occurred
    	Scheduler_pid=fork();
       }
       	if (Scheduler_pid == 0) { // child process
            char* buff=malloc(sizeof(char));//we need to free these memory
            char* len=malloc(sizeof(char));
            char* timeChunk=malloc(sizeof(char));
            int x=scheduling_algorithm;
            snprintf(buff,sizeof(buff),"%d",x);
            snprintf(len,sizeof(buff),"%d",processNumbers);
            snprintf(timeChunk,sizeof(buff),"%d",time_chunk);
           char *Scheduler_file[] = {"./scheduler",buff,len,timeChunk,NULL}; // arguments for execv
            
           execv(Scheduler_file[0], Scheduler_file); // execute the child process
           fprintf(stderr, "Exec failed\n"); // execv only returns if it fails
           exit(1);
        }
    

	
    // 4. Use this function after creating the clock process to initialize clock
    initClk();

    // To get time use this
    // int x = getClk();
    // printf("current time is %d\n", x);

    // TODO Generation Main Loop

    while (!isEmpty(&process_queue)) {
       p = *((process_par*)Queue_peek(&process_queue));
       int cur_time = getClk();
//    6. Send the information to the scheduler at the appropriate time.
       if (cur_time >= p.arrival_time) {
        //    send process to scheduler
//            // use message queue don't forget the flag !IPC_NOWAIT
       	msgbuff sentProcess;
       	sentProcess.process=p;
       	sentProcess.mtype = 7; /* arbitrary value */
       	send_val = msgsnd(msgq_id, &sentProcess, sizeof(sentProcess.process), !IPC_NOWAIT);
        if (send_val == -1)
        perror("Errror in send");
        Queue_pop(&process_queue);
       }
   }

    // 7. Clear clock resources

    /*Don't uncomment the destroyclk there is an exception thrown*/
    /*To Do: need to hndle the Exception */
    // destroyClk(true);



    //TODO clear the resources

  
}























