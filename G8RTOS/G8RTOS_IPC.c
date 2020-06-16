/*
 * G8RTOS_IPC.c
 *
 *  Created on: Jan 10, 2017
 *      Author: Daniel Gonzalez
 */
#include <stdint.h>
#include "msp.h"
#include "G8RTOS_IPC.h"
#include "G8RTOS_Semaphores.h"

/*********************************************** Defines ******************************************************************************/

#define FIFOSIZE 16
#define MAX_NUMBER_OF_FIFOS 4

/*********************************************** Defines ******************************************************************************/

/*********************************************** Data Structures Used *****************************************************************/

/*
 * FIFO struct will hold
 *  - buffer
 *  - head
 *  - tail
 *  - lost data
 *  - current size
 *  - mutex
 */
struct FIFO
{
    int32_t buffer[FIFOSIZE];
    volatile int32_t * head;
    volatile int32_t * tail;
    uint32_t lostData;
    semaphore_t currentSize;
    semaphore_t mutex;
};

typedef struct FIFO FIFO_t;

/* Array of FIFOS */
static FIFO_t FIFOs[4];

/*********************************************** Data Structures Used *****************************************************************/

/*
 * Initializes FIFO Struct
 */
int G8RTOS_InitFIFO(uint32_t FIFOIndex)
{
    if (FIFOIndex < MAX_NUMBER_OF_FIFOS)
    {
        FIFOs[FIFOIndex].head = &FIFOs[FIFOIndex].buffer[0];
        FIFOs[FIFOIndex].tail = &FIFOs[FIFOIndex].buffer[0];
        FIFOs[FIFOIndex].lostData = 0;
        G8RTOS_InitSemaphore(&FIFOs[FIFOIndex].currentSize, 0);
        G8RTOS_InitSemaphore(&FIFOs[FIFOIndex].mutex, 1);
        return 1;
    }
    else
        return -1;
}

/*
 * Reads FIFO
 *  - Waits until CurrentSize semaphore is greater than zero
 *  - Gets data and increments the head ptr (wraps if necessary)
 * Param: "FIFOChoice": chooses which buffer we want to read from
 * Returns: uint32_t Data from FIFO
 */
int32_t readFIFO(uint32_t FIFOChoice)
{
    G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].mutex);

    G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].currentSize);

    int32_t data;
    data = *FIFOs[FIFOChoice].head;

    FIFOs[FIFOChoice].head++;

    if (FIFOs[FIFOChoice].head == &FIFOs[FIFOChoice].buffer[FIFOSIZE - 1])
        FIFOs[FIFOChoice].head = &FIFOs[FIFOChoice].buffer[0];

    G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].mutex);
    return data;
}

/*
 * Writes to FIFO
 *  Writes data to Tail of the buffer if the buffer is not full
 *  Increments tail (wraps if necessary)
 *  Param "FIFOChoice": chooses which buffer we want to read from
 *        "Data': Data being put into FIFO
 *  Returns: error code for full buffer if unable to write
 */
int writeFIFO(uint32_t FIFOChoice, int32_t Data)
{

    if (FIFOs[FIFOChoice].currentSize == FIFOSIZE - 1)
    {
        //condition for lost data
        FIFOs[FIFOChoice].lostData++;
        return -1;
    }
    //write out data
    *FIFOs[FIFOChoice].tail = Data;

    FIFOs[FIFOChoice].tail++;

    /* wrap tail if at the end */
    if (FIFOs[FIFOChoice].tail == &FIFOs[FIFOChoice].buffer[FIFOSIZE - 1])
        FIFOs[FIFOChoice].tail = &FIFOs[FIFOChoice].buffer[0];

    G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].currentSize);
    return 1;
}

int32_t getFIFOSize(uint32_t FIFOChoice)
{
    return FIFOs[FIFOChoice].currentSize;
}

