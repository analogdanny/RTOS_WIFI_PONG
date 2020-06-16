/*
 * G8RTOS_Scheduler.c
 */

/*********************************************** Dependencies and Externs *************************************************************/

#include <G8RTOS/G8RTOS.h>
#include <G8RTOS/G8RTOS_CriticalSection.h>
#include <G8RTOS/G8RTOS_Scheduler.h>
#include <G8RTOS/G8RTOS_Structures.h>
#include <stdint.h>
#include "msp.h"
#include "ClockSys.h"
#include "BSP.h"
#include "systick.h"

/*
 * G8RTOS_Start and PendSV_Handler exists in asm
 */
extern void G8RTOS_Start();
extern void PendSV_Handler();

/* System Core Clock From system_msp432p401r.c */
extern uint32_t SystemCoreClock;

/* Pointer to the currently running Thread Control Block */
extern tcb_t * CurrentlyRunningThread;

/*********************************************** Dependencies and Externs *************************************************************/

/*********************************************** Defines ******************************************************************************/

/* Status Register with the Thumb-bit Set */
#define THUMBBIT 0x01000000

/*********************************************** Defines ******************************************************************************/

/*********************************************** Data Structures Used *****************************************************************/

/* Thread Control Blocks
 *	- An array of thread control blocks to hold pertinent information for each thread
 */
static tcb_t threadControlBlocks[MAX_THREADS];

/* Thread Stacks
 *	- An array of arrays that will act as individual stacks for each thread
 */
static int32_t threadStacks[MAX_THREADS][STACKSIZE];

/* Thread Control Blocks
 *  - An array of thread control blocks to hold pertinent information for each thread
 */
static pe_t periodicEventThreads[MAX_THREADS];

/*********************************************** Data Structures Used *****************************************************************/

/*********************************************** Private Variables ********************************************************************/

/*
 * Current Number of Threads currently in the scheduler
 */
static uint32_t NumberOfThreads;
/*
 * Current Number of Periodic Threads currently in the scheduler
 */
static uint32_t NumberOfPeriodicThreads;
/*
 * Current Number of Threads with an ID
 */
static uint16_t IDCounter;

/*********************************************** Private Variables ********************************************************************/

/*********************************************** Private Functions ********************************************************************/

/*
 * Initializes the Systick and Systick Interrupt
 * The Systick interrupt will be responsible for starting a context switch between threads
 * Param "numCycles": Number of cycles for each systick interrupt
 */
static void InitSysTick(float time)
{
    uint32_t freq = ClockSys_GetSysFreq();
    uint32_t ticks = (uint32_t) (time * freq);

    SysTick_Config(ticks);
    SysTick_enableInterrupt();
}

/*
 * Chooses the next thread to run.
 * Lab 2 Scheduling Algorithm:
 * 	- Simple Round Robin: Choose the next running thread by selecting the currently running thread's next pointer
 */
void G8RTOS_Scheduler()
{
    /* Priority of potential next thread to run */
    uint8_t nextThreadPriority = UINT8_MAX;

    /* Pointer to the Next Thread in the Control Block that isn't asleep/blocked */
    tcb_t * tempNextThread;
    tempNextThread = CurrentlyRunningThread;

    for (int i = 0; i < NumberOfThreads; i++)
    {
        if (!tempNextThread->asleep && !tempNextThread->blocked && tempNextThread->isAlive)
        {
            if (tempNextThread->priority < nextThreadPriority)
            {
                CurrentlyRunningThread = tempNextThread;
                nextThreadPriority = CurrentlyRunningThread->priority;
            }
        }
        tempNextThread = tempNextThread->next_tcb;
    }
}

/*
 * SysTick Handler
 * Currently the Systick Handler will only increment the system time
 * and set the PendSV flag to start the scheduler
 *
 * In the future, this function will also be responsible for sleeping threads and periodic threads
 */
void SysTick_Handler()
{
    /* Increment System Time */
    SystemTime++;

    // Create a pointer and set it to the first periodic event thread
    pe_t * Pptr;
    Pptr = &periodicEventThreads[0];

    // Check if there is a periodic thread that needs to execute
    for (int i = 0; i < NumberOfPeriodicThreads; i++)
    {
        if (Pptr->execute_time == SystemTime)
        {
            Pptr->execute_time = Pptr->period + SystemTime;
            (*Pptr->Handler)(); //Look into calling periodic threads
        }

        Pptr = Pptr->next_pe;

    }

    tcb_t * ptr;
    ptr = CurrentlyRunningThread;

    // Now check for a thread to be asleep, and if we need to wake it up
    for (int i = 0; i < MAX_THREADS; i++)
    {
        //If threads sleep count is equal to the system time, wake thread
        if (ptr->asleep == true && ptr->sleepCount == SystemTime)
        {
            ptr->asleep = false;
        }
        ptr = ptr->next_tcb;
    }

    /* Setting the bit for the ICSR register of SCB for PendSV flag to start the scheduler
     * and allow for context switching, yield control */
    SCB->ICSR = BIT(28);
}

/*********************************************** Private Functions ********************************************************************/

/*********************************************** Public Variables *********************************************************************/

/* Holds the current time for the whole System */
uint32_t SystemTime;

/*********************************************** Public Variables *********************************************************************/

/*********************************************** Public Functions *********************************************************************/

/*
 * Sets variables to an initial state (system time and number of threads)
 * Enables board for highest speed clock and disables watchdog
 */
void G8RTOS_Init()
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    BSP_InitBoard();

    SystemTime = 0;         // Initialize the time to zero
    NumberOfThreads = 0;    // Initialize the threads to zero
    IDCounter = 0;

    uint32_t newVTORTable = 0x20000000;
    memcpy((uint32_t *) newVTORTable, (uint32_t *) SCB->VTOR, 57 * 4); // 57 interrupt vectors to copy
    SCB->VTOR = newVTORTable;
}

/*
 * Starts G8RTOS Scheduler
 * 	- Initializes the Systick
 * 	- Sets Context to first thread
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
int G8RTOS_Launch()
{
    InitSysTick(0.001);     //Initialize SysTick Timer to 1 ms

    /* Set SysTick_Handler and PendSV_Handler to the lowest priority. */
    NVIC_SetPriority(SysTick_IRQn, OSINT_PRIORITY);
    NVIC_SetPriority(PendSV_IRQn, OSINT_PRIORITY);

    uint8_t nextThreadPriority = UINT8_MAX;

    for (int i = 0; i < NumberOfThreads; i++)
    {
        if (threadControlBlocks[i].priority < nextThreadPriority)
        {
            CurrentlyRunningThread = &threadControlBlocks[i];
            nextThreadPriority = CurrentlyRunningThread->priority;
        }
    }

    if (NumberOfThreads == 0)
        return -1;  //failed start

    G8RTOS_Start();     //Start the Scheduler
    return 1;
}

/*
 * Adds threads to G8RTOS Scheduler
 * 	- Checks if there are stil available threads to insert to scheduler
 * 	- Initializes the thread control block for the provided thread
 * 	- Initializes the stack for the provided thread to hold a "fake context"
 * 	- Sets stack tcb stack pointer to top of thread stack
 * 	- Sets up the next and previous tcb pointers in a round robin fashion
 * Param "threadToAdd": Void-Void Function to add as preemptable main thread
 * Returns: Error code for adding threads
 */
int G8RTOS_AddThread(void (*threadToAdd)(void), uint8_t priority, char * name)
{
    int32_t status = StartCriticalSection();       // start critical section
    //index for thread to add, along with a flag for dead threads
    int32_t i = 0;

    if (NumberOfThreads == MAX_THREADS - 1)
    {
        EndCriticalSection(status);                  // End the critical section
        return THREAD_LIMIT_REACHED;
    }


    /*
     *
     *  You were thinking that when adding a thread,
     *  i < NumberOfThreads should somehow start at the CRT
     *  and iterate MAX_THREAD times
     *
     * */

    //Search for a dead thread
    while (i < NumberOfThreads)
    {
        if (threadControlBlocks[i].isAlive == dead)
        {
            break;
        }
        i++;
    }

    //If thread is not dead, add threads links
    if (NumberOfThreads == 0) // Only one Thread
    {
        threadControlBlocks[0].previous_tcb = &threadControlBlocks[0];
        threadControlBlocks[0].next_tcb = &threadControlBlocks[0];
    }
    else
    {
        if (i == 0)
        {
            //THIS IS WHERE YOU LEFT OFF AND WAS ADDED
            for(int j = 1; i < MAX_THREADS; j++)
            {
                if(threadControlBlocks[j].isAlive)
                {
                    threadControlBlocks[i].next_tcb =
                                        &threadControlBlocks[j];
                    threadControlBlocks[i].previous_tcb =
                                        threadControlBlocks[j].previous_tcb;
                    break;
                }
            }
        }
        else
        {
            threadControlBlocks[i].previous_tcb = &threadControlBlocks[i - 1];
            threadControlBlocks[i].next_tcb =
                    threadControlBlocks[i].previous_tcb->next_tcb; //Added threads next ptr set to the previous alive threads next thread
        }
        threadControlBlocks[i].next_tcb->previous_tcb = &threadControlBlocks[i]; //Make the thread ahead point back to the added thread
        threadControlBlocks[i].previous_tcb->next_tcb = &threadControlBlocks[i]; //Make the previous thread point to the added thread
    }

    SetInitialStack(i);
    threadStacks[i][STACKSIZE - 2] = (int32_t) (threadToAdd);
    threadControlBlocks[i].sleepCount = 0;
    threadControlBlocks[i].asleep = false;
    threadControlBlocks[i].blocked = 0;
    threadControlBlocks[i].isAlive = alive;
    threadControlBlocks[i].priority = priority;
    threadControlBlocks[i].threadID = ((IDCounter++) << 16) | i;

    //Character array location
    uint32_t character = 0;

    //Assign name to character array
    while (*name != 0)
    {
        threadControlBlocks[i].threadName[character] = *name++;
        character++;
    }

    NumberOfThreads++;  // increment the number of threads currently running
    EndCriticalSection(status);                      // End the critical section
    return NO_ERROR;
}

/* Generates the initial stack for a new thread from the G8RTOS_AddThread()
 * and fills each register with "fake content."
 *
 * */
void SetInitialStack(int i)
{
    threadControlBlocks[i].stack_pointer = &threadStacks[i][STACKSIZE - 16]; // thread stack pointer
    threadStacks[i][STACKSIZE - 1] = 0x01000000; // Thumb bit
    threadStacks[i][STACKSIZE - 3] = 0x14141414; // R14
    threadStacks[i][STACKSIZE - 4] = 0x12121212; // R12
    threadStacks[i][STACKSIZE - 5] = 0x03030303; // R3
    threadStacks[i][STACKSIZE - 6] = 0x02020202; // R2
    threadStacks[i][STACKSIZE - 7] = 0x01010101; // R1
    threadStacks[i][STACKSIZE - 8] = 0x00000000; // R0
    threadStacks[i][STACKSIZE - 9] = 0x11111111; // R11
    threadStacks[i][STACKSIZE - 10] = 0x10101010; // R10
    threadStacks[i][STACKSIZE - 11] = 0x09090909; // R9
    threadStacks[i][STACKSIZE - 12] = 0x08080808; // R8
    threadStacks[i][STACKSIZE - 13] = 0x07070707; // R7
    threadStacks[i][STACKSIZE - 14] = 0x06060606; // R6
    threadStacks[i][STACKSIZE - 15] = 0x05050505; // R5
    threadStacks[i][STACKSIZE - 16] = 0x04040404; // R4
}

/* Inilizae sleep count, and put the thread to sleep, yields control to allow
 * other threads to run. */
void OS_Sleep(uint32_t duration)
{
    //Initialize the currently running thread's sleep count
    CurrentlyRunningThread->sleepCount = duration + SystemTime;

    //Put thread to sleep
    CurrentlyRunningThread->asleep = true;

    /* Setting the bit for the ICSR register of SCB for PendSV flag to start the scheduler
     * and allow for context switching, yield control */
    SCB->ICSR = BIT(28);
}

/* Add Periodic Threads to G8RTOS Scheduler
 *  - Initialize the period thread's structure, and handle the
 *    linked list data structure
 *  - Increment the static number of periodic threads inside of scheduler */
int32_t G8RTOS_AddPeriodicThread(void (*threadToAdd)(void), uint32_t p)
{

    int32_t status = StartCriticalSection();       // start critical section

    if (NumberOfPeriodicThreads < MAX_THREADS)
    {
        NumberOfPeriodicThreads++; // increment the number of threads currently running

        /* This for loop will ensure that threads are ran as a round robin
         * doubly linked list.
         * */
        for (int i = 0; i < NumberOfPeriodicThreads; i++)
        {
            /* If-Else tree to ensure round robin works for all Number of Threads added from 1-6 */
            if (NumberOfPeriodicThreads == 1)
            {
                periodicEventThreads[i].next_pe = &periodicEventThreads[0];
                periodicEventThreads[i].previous_pe = &periodicEventThreads[0];
            }
            else if (i == 0)
            {
                periodicEventThreads[i].next_pe = &periodicEventThreads[i + 1];
                periodicEventThreads[i].previous_pe =
                        &periodicEventThreads[NumberOfPeriodicThreads - 1];
            }
            else if (i + 1 == NumberOfPeriodicThreads)
            {
                periodicEventThreads[i].next_pe = &periodicEventThreads[0];
                periodicEventThreads[i].previous_pe = &periodicEventThreads[i
                        - 1];
            }
            else
            {
                periodicEventThreads[i].next_pe = &periodicEventThreads[i + 1];
                periodicEventThreads[i].previous_pe = &periodicEventThreads[i
                        - 1];
            }
        }

        /* Initialize the recently add periodic event threads structure */
        periodicEventThreads[NumberOfPeriodicThreads - 1].Handler = threadToAdd;
        periodicEventThreads[NumberOfPeriodicThreads - 1].current_time = 0;
        periodicEventThreads[NumberOfPeriodicThreads - 1].period = p;
        periodicEventThreads[NumberOfPeriodicThreads - 1].execute_time =
                NumberOfPeriodicThreads;

        EndCriticalSection(status);                  // End the critical section
        return 1;                                      // Return 1 if successful
    }
    else
    {
        EndCriticalSection(status);                  // End the critical section
        return -1;   //else return -1 if unsuccessful
    }
}

sched_ErrCode_t G8RTOS_AddAPeriodicEvent(void (*AthreadToAdd)(void),
                                         uint8_t priority, IRQn_Type IRQn)
{
    int32_t status = StartCriticalSection();

    if (IRQn < PSS_IRQn || IRQn > PORT6_IRQn)
        return IRQn_INVALID;

    if (priority > 6)
        return HWI_PRIORITY_INVALID;

    __NVIC_SetVector(IRQn, AthreadToAdd);
    __NVIC_SetPriority(IRQn, priority);
    NVIC_EnableIRQ(IRQn);

    EndCriticalSection(status);

    return NO_ERROR;
}

/* Obtain the threadID of the Currently Running Thread
 * */
threadID_t G8RTOS_GetThreadId()
{
    return CurrentlyRunningThread->threadID;
}

sched_ErrCode_t G8RTOS_KillThread(threadID_t threadID)
{
    int32_t status = StartCriticalSection();


    // Checks to see if there is only one thread
    if (NumberOfThreads == 1)
    {
        EndCriticalSection(status);
        return CANNOT_KILL_LAST_THREAD;
    }

    int i = 0;

    // Find the thread with the ID sent in
    while (i < MAX_THREADS)
    {
        if (threadControlBlocks[i].threadID == threadID && threadControlBlocks[i].isAlive)
            break;
        i++;
    }

    //if this condition is true, the thread doesn't exist.
    if (i == MAX_THREADS)
    {
        EndCriticalSection(status);
        return THREAD_DOES_NOT_EXIST;
    }

    // Kill
    threadControlBlocks[i].priority = 255;
    threadControlBlocks[i].asleep = false;
    threadControlBlocks[i].sleepCount = 0;
    threadControlBlocks[i].threadID = 0;
    threadControlBlocks[i].isAlive = dead;

    // Decrements the number of threads
    NumberOfThreads--;

    //re-adjust pointers for linked list
    threadControlBlocks[i].previous_tcb->next_tcb =
            threadControlBlocks[i].next_tcb;
    threadControlBlocks[i].next_tcb->previous_tcb =
            threadControlBlocks[i].previous_tcb;

    // Checks to see if the thread being killed is currently running
    if (CurrentlyRunningThread == &threadControlBlocks[i])
    {
        //clear pipelines
        __DSB();
        __ISB();
        //start context switch
        EndCriticalSection(status);
        SCB->ICSR = PENDSVSET;
        return NO_ERROR;
    }

    EndCriticalSection(status);     // End critical section

    return NO_ERROR;
}

/* The KillSelf function will simply kill the currently running thread.
 * */
sched_ErrCode_t G8RTOS_KillSelf()
{
    int32_t status = StartCriticalSection();

    if (NumberOfThreads == 1)
    {
        EndCriticalSection(status);
        return CANNOT_KILL_LAST_THREAD;
    }

    //Adjust pointers
    CurrentlyRunningThread->isAlive = dead;

    CurrentlyRunningThread->previous_tcb->next_tcb =
            CurrentlyRunningThread->next_tcb;
    CurrentlyRunningThread->next_tcb->previous_tcb =
            CurrentlyRunningThread->previous_tcb;

    //Decrement threads and perform context switch
    NumberOfThreads--;

    __DSB();
    __ISB();
    EndCriticalSection(status);
    SCB->ICSR = PENDSVSET;

    return NO_ERROR;
}

sched_ErrCode_t G8RTOS_KillAll()
{
    int32_t status = StartCriticalSection();

//    if (NumberOfThreads == 1)
//    {
//        EndCriticalSection(status);
//        return CANNOT_KILL_LAST_THREAD;
//    }

    tcb_t ptr = *CurrentlyRunningThread;

    for(int i = 0; i < MAX_THREADS; i++)
    {
        if(ptr.threadID != threadControlBlocks[i].threadID && threadControlBlocks[i].isAlive)
            G8RTOS_KillThread(threadControlBlocks[i].threadID);
    }

//    for(int i = 1; i < NumberOfThreads; i++)
//    {
//        if(ptr != CurrentlyRunningThread)
//        {
//            ptr->isAlive = dead;
//            ptr->asleep = false;
//            ptr->blocked = 0;
//            ptr->sleepCount = 0;
//        }
//        ptr = ptr->next_tcb;
//    }
//
//    NumberOfThreads = 1;
//    //CurrentlyRunningThread->next_tcb = CurrentlyRunningThread;
//    //CurrentlyRunningThread->previous_tcb = CurrentlyRunningThread;

    EndCriticalSection(status);
    return NO_ERROR;
}
/*********************************************** Public Functions *********************************************************************/
