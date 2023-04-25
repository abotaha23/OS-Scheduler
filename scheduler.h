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

/*******************************************************************************
 *                         Types Declaration                                   *
 *******************************************************************************/
typedef sturct{



}PCB;


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
 * 1- Process first time excuted we will fork 
 * 2- Process already exists but is stopped we will resume it 
 * 
 * 
*/

//void Scheduler_processExcute();

/*


*/
//void Scheduler_processStop();
// void HPF();
// void SRTN();
// void RR(int chunck);



#endif


