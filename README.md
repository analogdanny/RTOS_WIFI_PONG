# RTOS_WIFI_PONG - IoT with Real-Time Operating System on TI MSP432 Launchpad

Created a 2-player pong game playable over Wi-Fi using the CC3100 Wi-Fi Booster Pack, a router or mobile hotspot, and the TI Sensors Booster Pack. 
Attached additional components to the Microprocessor Applications II circuit board, supplied by the University of Florida.

## Multi-stage development for RTOS design and multi-threading development. 
1st Design: Original RTOS used spin-lock algorithms with semaphores to prevent deadlocking threads. 
2nd Design: Improved 1st design by addings in blocking and sleeping functionality, along with aperiodic thread creation.
3rd Design: 2nd design was further designed with thread priority, dynamic thread creation and destruction, blocking and sleeping thread capabilities,
            FIFOs, and aperiodic events, interfaceable with a touch screen LCD.

Implemented LED drivers for built-in LED Array Module, touch screen LCD drivers, and FIFO structure and functionality
using semaphores.

The project Yerpers on my repository is using what was created here to make a 2-D platformer game. The RTOS will have scheduling improvement, while also 
trying to reduce lag inbetween the host/clients connection over WiFi.
