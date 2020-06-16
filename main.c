#include "msp.h"
#include "Game.h"
#include "cc3100_usage.h"
#include "BSP.h"
#include <driverlib.h>
#include "G8RTOS.h"
#include "LCDLib.h"
/**
 * main.c
 */

void main(void)
{
    /* Initialize port directions and out registers */
    P2->DIR |= 0x03;
    P1->DIR |= 0x01;
    P2->OUT = 0x00;
    P1->OUT = 0x00;

    G8RTOS_Init();

    if(myPlayer == Host)
        G8RTOS_AddThread(&CreateGame, 1, "Create Game");
    else if(myPlayer == Client)
        G8RTOS_AddThread(&JoinGame, 1, "Join Game");

    G8RTOS_InitSemaphore(&CC3100Semaphore, 1);
    G8RTOS_InitSemaphore(&LCDSemaphore, 1);
    G8RTOS_InitSemaphore(&I2CSemaphore, 1);

    LCD_Init(false);
    G8RTOS_Launch();
}
