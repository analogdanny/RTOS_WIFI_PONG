/*
 * G8RTOS_Semaphores.c
 */

/*********************************************** Dependencies and Externs *************************************************************/

#include <G8RTOS/G8RTOS_CriticalSection.h>
#include <G8RTOS/G8RTOS_Semaphores.h>
#include <G8RTOS/G8RTOS_Structures.h>
#include <stdint.h>
#include "msp.h"

/*********************************************** Dependencies and Externs *************************************************************/


/*********************************************** Public Functions *********************************************************************/

/*
 * Initializes a semaphore to a given value
 * Param "s": Pointer to semaphore
 * Param "value": Value to initialize semaphore to
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_InitSemaphore(semaphore_t *s, int32_t value)
{
    int32_t status = StartCriticalSection();     // start critical section

    // Set the value of the semaphore
    *s = value;

    EndCriticalSection(status);                  // End the critical section
}

/*
 * Waits for a semaphore to be available (value greater than 0)
 * 	- Decrements semaphore when available
 * 	- Spinlocks to wait for semaphore
 * Param "s": Pointer to semaphore to wait on
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_WaitSemaphore(semaphore_t *s)
{
    int32_t status = StartCriticalSection();     // start critical section

    //semaphore available
    (*s) = (*s) - 1;

    /* If the current semaphore is unavailable */
    if((*s) < 0)
    {
        //block thread
        CurrentlyRunningThread->blocked = s;

        EndCriticalSection(status);                  // End the critical section

        /* Setting the bit for the ICSR register of SCB for PendSV flag to start the scheduler
         * and allow for context switching, yield control */
        SCB->ICSR = BIT(28);
    }
    EndCriticalSection(status);                  // End the critical section
}

/*
 * Signals the completion of the usage of a semaphore
 * 	- Increments the semaphore value by 1
 * Param "s": Pointer to semaphore to be signalled
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_SignalSemaphore(semaphore_t *s)
{
    int32_t status = StartCriticalSection();     // start critical section
    tcb_t *pt;

    //Increment the semaphore
    (*s) = (*s) + 1;

    //Semaphore is available for use again
    if((*s) <= 0)
    {
        //Go through TCB list and unblock the first thread that is blocked
        pt = CurrentlyRunningThread->next_tcb;
        while(pt->blocked != s)
        {
            pt = pt->next_tcb;
        }

        //unblock the thread that was found
        pt->blocked = 0;
    }

    EndCriticalSection(status);                  // End the critical section
}

/*********************************************** Public Functions *********************************************************************/


