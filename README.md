# RTOS_WIFI_PONG
## IoT with Real-Time Operating System on TI MSP432 Launchpad
Created a 2-player pong game playable over Wi-Fi using the CC3100 Wi-Fi Booster Pack, a router or mobile hotspot, and the TI Sensors Booster Pack.
Attached additional components to the Microprocessor Applications II circuit board, supplied by the University of Florida.
Implemented LED drivers for built-in LED Array Module, touch screen LCD drivers, and FIFO structure and functionality.

*The project Yerpers on my repository is using what was created here to make a 2-D platformer game with github user digitalcamilo. The RTOS will have 
scheduling improvement, while also trying to reduce lag inbetween the host/clients connection over WiFi.*

## Multi-stage development for RTOS design and multi-threading development 
**1st Design:** Original RTOS used spin-lock algorithms with semaphores to prevent deadlocking threads.\
**2nd Design:** Improved 1st design by addings in blocking and sleeping functionality, along with aperiodic thread creation.\
**3rd Design:** 2nd design was further designed with thread priority, dynamic thread creation and destruction, blocking and sleeping thread capabilities,
            FIFOs, and aperiodic events, interfaceable with a touch screen LCD.

## Description of Packages
**BoardSupportPackage:** This board support package holds software containing TI MSP432P401R Launchpad drivers and routines.\
**CC3100SupportPackage:** This support package holds software containing SimpleLink Wi-Fi CC3100 BoosterPack drivers and routines.

## Description of Files and Folders
**G8RTOS:** The G8RTOS folder contains many files that allow the operation of a Real-Time Operation System.\
&nbsp;&nbsp;&nbsp;**-G8RTOS.h:** The G8RTOS header file contains include paths for the scheduler and semaphores.\
&nbsp;&nbsp;&nbsp;**-G8RTOS_CriticalSection.s:** This file holds all ASM functions needed for a Critical Section, saving or restoring the &nbsp;&nbsp;&nbsp;&nbsp;state of the current PRIMASK (I-bit).\
&nbsp;&nbsp;&nbsp;**-G8RTOS_CriticalSection.h:** Header file for G8RTOS_CriticalSection.s.\
&nbsp;&nbsp;&nbsp;**-G8RTOS_IPC.c:** This file holds all source code for the FIFO structure and functions(Initialize, Read, Write, Get Size).
&nbsp;&nbsp;&nbsp;**-G8RTOS_IPC.h:** Header file for G8RTOS_IPC.c.\
&nbsp;&nbsp;&nbsp;**-G8RTOS_Scheduler.c:** This file holds all source code for the RTOS scheduler, containing dependencies and\
&nbsp;&nbsp;&nbsp;&nbsp;externs, defines, data structures, private variables and functions, public variables and functions. The functions 
&nbsp;&nbsp;&nbsp;&nbsp;included are the RTOS launch, intialization, adding threads and aperiodic events, get thread ID, and killing of
&nbsp;&nbsp;&nbsp;&nbsp;threads.\
&nbsp;&nbsp;&nbsp;**-G8RTOS_Scheduler.h:** Header file for G8RTOS_Scheduler.c.\
&nbsp;&nbsp;&nbsp;**-G8RTOS_SchedulerASM.s:** This file holds all ASM functions needed for the scheduler, including context switching\
&nbsp;&nbsp;&nbsp;&nbsp;and starting the RTOS.\
&nbsp;&nbsp;&nbsp;**-G8RTOS_Semaphores.c:** This file contains functions to initialize, wait, and signal semaphores for availability.\
&nbsp;&nbsp;&nbsp;**-G8RTOS_Semaphores.h:** Header file for G8RTOS_Semaphores.c.\
&nbsp;&nbsp;&nbsp;**-G8RTOS_Structures.h:** This header file contains data structures for thread control blocks, thread ID, and periodic 
&nbsp;&nbsp;&nbsp;&nbsp;events.

**Game.c:** This file holds the pong game functionality using multi-threading for host and client connections, reading the joystick values, updating the LED array module, and drawing or moving objects.\
**Game.h:** Header file for Game.c.

**LCDLib.c:** This file contains the LCD touch-screen drivers functions and helping functions.\
**LCDLib.h:** Header file for LCDLib.c.
