/*
 * G8RTOS_Scheduler.h
 */
#include "msp.h"

#ifndef G8RTOS_SCHEDULER_H_
#define G8RTOS_SCHEDULER_H_

/*********************************************** Sizes and Limits *********************************************************************/
#define MAX_THREADS 27
#define STACKSIZE 512
#define OSINT_PRIORITY 7
#define PENDSVSET SCB_ICSR_PENDSVSET_Msk
/*********************************************** Sizes and Limits *********************************************************************/

/*********************************************** Public Variables *********************************************************************/

/* Holds the current time for the whole System */
extern uint32_t SystemTime;

/*********************************************** Public Variables *********************************************************************/

typedef uint32_t threadID_t;
/*
* Error Codes for Scheduler
*/
typedef enum
{
    NO_ERROR = 0,
    THREAD_LIMIT_REACHED = -1,
    NO_THREADS_SCHEDULED = -2,
    THREADS_INCORRECTLY_ALIVE = -3,
    THREAD_DOES_NOT_EXIST = -4,
    CANNOT_KILL_LAST_THREAD = -5,
    IRQn_INVALID = -6,
    HWI_PRIORITY_INVALID = -7

} sched_ErrCode_t;

/*********************************************** Public Functions *********************************************************************/

/*
 * Initializes variables and hardware for G8RTOS usage
 */
void G8RTOS_Init();

/*
 * Starts G8RTOS Scheduler
 * 	- Initializes Systick Timer
 * 	- Sets Context to first thread
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
int32_t G8RTOS_Launch();

/*
 * Adds threads to G8RTOS Scheduler
 * 	- Checks if there are stil available threads to insert to scheduler
 * 	- Initializes the thread control block for the provided thread
 * 	- Initializes the stack for the provided thread
 * 	- Sets up the next and previous tcb pointers in a round robin fashion
 * Param "threadToAdd": Void-Void Function to add as preemptable main thread
 * Returns: Error code for adding threads
 */
int32_t G8RTOS_AddThread(void (*threadToAdd)(void), uint8_t priority, char * name);

/*
 * Sets up the fake content in the registers for the tcb stacks
 * during the G8RTOSAddThread().
 * */
void SetInitialStack(int i);

/* Inilizae sleep count, and put the thread to sleep, yields control to allow
 * other threads to run. */
void OS_Sleep(uint32_t duration);

/* Add Periodic Threads to G8RTOS Scheduler
 *  - Initialize the period thread's structure, and handle the
 *    linked list data structure
 *  - Increment the static number of periodic threads inside of scheduler */
int32_t G8RTOS_AddPeriodicThread(void (*threadToAdd)(void), uint32_t p);

sched_ErrCode_t G8RTOS_AddAPeriodicEvent(void (*AthreadToAdd)(void), uint8_t priority, IRQn_Type IRQn);

/* Obtain the threadID of the Currently Running Thread
 * */
threadID_t G8RTOS_GetThreadId();

/* Kill a specific thread based off the threads ID, returns error code
 * */
sched_ErrCode_t G8RTOS_KillThread(threadID_t threadID);

/* The KillSelf function will simply kill the currently running thread.
 * */
sched_ErrCode_t G8RTOS_KillSelf();

/* Kill all threads except the currently running thread */
sched_ErrCode_t G8RTOS_KillAll();

/*********************************************** Public Functions *********************************************************************/

#endif /* G8RTOS_SCHEDULER_H_ */
