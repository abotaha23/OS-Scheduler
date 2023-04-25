 /******************************************************************************
 *
 * Module: process_generator
 *
 * File Name: process_generator.h
 *
 * Description: header file for the process generator
 *
 * Author: Mohamed Samir& Hamdy Salem
 *
 *******************************************************************************/
#ifndef process_generator
#define process_generator
/*******************************************************************************
 *                     global variables                                    *
 *******************************************************************************/
int msgq_id;
queue process_queue;  
int processNumbers;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description : Function to clear all resources

 */

void clearResources(int);

#endif

