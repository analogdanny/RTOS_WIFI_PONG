/*
 * G8RTOS_Structure.h
 *
 *  Created on: Jan 12, 2017
 *      Author: Raz Aloni
 */

#ifndef G8RTOS_STRUCTURES_H_
#define G8RTOS_STRUCTURES_H_

#include "../G8RTOS/G8RTOS.h"

/*********************************************** Data Structure Definitions ***********************************************************/

/*
 *  Thread Control Block:
 *      - Every thread has a Thread Control Block
 *      - The Thread Control Block holds information about the Thread Such as the Stack Pointer, Priority Level, and Blocked Status
 *      - For Lab 2 the TCB will only hold the Stack Pointer, next TCB and the previous TCB (for Round Robin Scheduling)
 */

//Typedef bool so the value of true/false can be used and threadID_t
typedef uint32_t threadID_t;
typedef bool;

#define true 1
#define false 0
#define alive 1
#define dead 0
#define MAX_NAME_LENGTH 16

struct tcb
{
    int32_t * stack_pointer;
    struct tcb * next_tcb;
    struct tcb * previous_tcb;
    threadID_t threadID;
    char threadName[MAX_NAME_LENGTH];
    bool isAlive;
    uint8_t priority;
    bool asleep;
    uint32_t sleepCount;
    semaphore_t blocked;
};

typedef struct tcb tcb_t;

/* Periodic Event:
 *  - Holds information regarding the event's state, similar to tcb
 *  - Function pointer to period event handler
 *  - Period
 *  - Execution Time
 *  - Pointer to the previous periodic event
 *  - Pointer to the next periodic event
 *  MAX number of period events is defined by the OS  */

struct periodic_event{
    void (*Handler)(void);
    uint32_t period;
    uint32_t execute_time;
    uint32_t current_time;
    struct periodic_event * previous_pe;
    struct periodic_event * next_pe;
};

typedef struct periodic_event pe_t;


/*********************************************** Data Structure Definitions ***********************************************************/


/*********************************************** Public Variables *********************************************************************/

tcb_t * CurrentlyRunningThread;
tcb_t * tempNextThread;

/*********************************************** Public Variables *********************************************************************/




#endif /* G8RTOS_STRUCTURES_H_ */
