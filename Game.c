/*
 * Game.c
 *
 *  Created on: Mar 15, 2020
 *      Author: Danny
 */

#include "Game.h"
#include <stdio.h>
#include "G8RTOS_IPC.h"
#include <driverlib.h>
#include "stdlib.h"
#include "time.h"
#include "RGBLeds.h"

GameState_t HostToClient, ClientToHost;
uint8_t packetHost[sizeof(HostToClient)], packetClient[sizeof(HostToClient)],
        str_client[2], str_host[2], buttonFlag = 0;

/*********************** WiFi Packet Funcitons ***********************/

/* Function to fill in a packet to be sent over through WiFi */
static inline void fillPacket(GameState_t * gs, _u8 * buffer)
{
    _u8 * ptr;
    ptr = gs;

    for (int i = 0; i < sizeof(*gs); i++)
    {
        buffer[i] = *ptr;
        ptr++;
    }
}
;

/* Function to empty a packet after being sent over through WiFi */
static inline void emptyPacket(GameState_t * gs, _u8 * buffer)
{
    _u8 * ptr;
    ptr = gs;

    for (int i = 0; i < sizeof(*gs); i++)
    {
        *ptr = buffer[i];
        ptr++;
    }
}
;

/*********************** WiFi Packet Funcitons ***********************/

/*********************** Common Threads ***********************/

void DrawObjects()
{
    PrevBall_t prevball[MAX_NUM_OF_BALLS];
    PrevPlayer_t prevplayer[MAX_NUM_OF_PLAYERS];

    //start the pervious tracking centers
    prevplayer[0].Center = HostToClient.players[0].currentCenter;
    prevplayer[1].Center = HostToClient.players[1].currentCenter;

    for (int i = 0; i < MAX_NUM_OF_BALLS; i++)
    {
        prevball[i].CenterX = HostToClient.balls[i].currentCenterX;
        prevball[i].CenterY = HostToClient.balls[i].currentCenterY;
    }

    while (1)
    {
        /***********************************************************************************/
        /****************************** DRAWING BALLS **************************************/
        /***********************************************************************************/
        for (int i = 0; i < MAX_NUM_OF_BALLS; i++)
        {
            if (HostToClient.balls[i].alive == 1
                    && (prevball[i].CenterX
                            != HostToClient.balls[i].currentCenterX
                            || prevball[i].CenterY
                                    != HostToClient.balls[i].currentCenterY)
                    && (HostToClient.balls[i].currentCenterY > ARENA_MIN_Y
                            && HostToClient.balls[i].currentCenterY
                                    < ARENA_MAX_Y))
            {
                G8RTOS_WaitSemaphore(&LCDSemaphore);

                LCD_DrawRectangle(prevball[i].CenterX - BALL_SIZE_D2,
                                  prevball[i].CenterX + BALL_SIZE_D2,
                                  prevball[i].CenterY - BALL_SIZE_D2,
                                  prevball[i].CenterY + BALL_SIZE_D2,
                                  LCD_BLACK);

                LCD_DrawRectangle(
                        HostToClient.balls[i].currentCenterX - BALL_SIZE_D2,
                        HostToClient.balls[i].currentCenterX + BALL_SIZE_D2,
                        HostToClient.balls[i].currentCenterY - BALL_SIZE_D2,
                        HostToClient.balls[i].currentCenterY + BALL_SIZE_D2,
                        HostToClient.balls[i].color);

                G8RTOS_SignalSemaphore(&LCDSemaphore);

                /* update the previous ball*/
                prevball[i].CenterX = HostToClient.balls[i].currentCenterX;
                prevball[i].CenterY = HostToClient.balls[i].currentCenterY;
            }

            if (HostToClient.balls[i].killed == true
                    && HostToClient.balls[i].alive == false)
            {
                HostToClient.balls[i].killed = false;

                G8RTOS_WaitSemaphore(&LCDSemaphore);

                //erase the balls previous and current location
                LCD_DrawRectangle(prevball[i].CenterX - BALL_SIZE_D2,
                                  prevball[i].CenterX + BALL_SIZE_D2,
                                  prevball[i].CenterY - BALL_SIZE_D2,
                                  prevball[i].CenterY + BALL_SIZE_D2,
                                  LCD_BLACK);

                G8RTOS_SignalSemaphore(&LCDSemaphore);

            }
        }

        /***********************************************************************************/
        /****************************** DRAWING PADDLES/PLAYERS ****************************/
        /***********************************************************************************/

        /* The next two if-else if statements handle the paddles being drawn/handled out of bounds of the arena */
        if (HostToClient.players[0].currentCenter + PADDLE_LEN_D2
                >= ARENA_MAX_X - 1)
            HostToClient.players[0].currentCenter = ARENA_MAX_X - 1
                    - PADDLE_LEN_D2;
        else if (HostToClient.players[0].currentCenter - PADDLE_LEN_D2
                <= ARENA_MIN_X + 1)
            HostToClient.players[0].currentCenter = ARENA_MIN_X + 1
                    + PADDLE_LEN_D2;

        if (HostToClient.players[1].currentCenter + PADDLE_LEN_D2
                >= ARENA_MAX_X - 1)
            HostToClient.players[1].currentCenter = ARENA_MAX_X - 1
                    - PADDLE_LEN_D2;
        else if (HostToClient.players[1].currentCenter - PADDLE_LEN_D2
                <= ARENA_MIN_X + 1)
            HostToClient.players[1].currentCenter = ARENA_MIN_X + 1
                    + PADDLE_LEN_D2;

        //If the paddles center for host has changed
        if (prevplayer[0].Center < HostToClient.players[0].currentCenter)
        {
            //moving right

            G8RTOS_WaitSemaphore(&LCDSemaphore);

            //erase the difference
            if (prevplayer[0].Center - PADDLE_LEN_D2 - PRINT_OFFSET
                    > HORIZ_CENTER_MIN_BALL)
                LCD_DrawRectangle(
                        prevplayer[0].Center - PADDLE_LEN_D2 - PRINT_OFFSET,
                        HostToClient.players[0].currentCenter - PADDLE_LEN_D2,
                        ARENA_MAX_Y - PADDLE_WID,
                        ARENA_MAX_Y, LCD_BLACK);
            else
                LCD_DrawRectangle(
                        prevplayer[0].Center - PADDLE_LEN_D2,
                        HostToClient.players[0].currentCenter - PADDLE_LEN_D2,
                        ARENA_MAX_Y - PADDLE_WID,
                        ARENA_MAX_Y, LCD_BLACK);

            //draw the difference
            LCD_DrawRectangle(
                    prevplayer[0].Center + PADDLE_LEN_D2,
                    HostToClient.players[0].currentCenter + PADDLE_LEN_D2,
                    ARENA_MAX_Y - PADDLE_WID,
                    ARENA_MAX_Y, HostToClient.players[0].color);

            G8RTOS_SignalSemaphore(&LCDSemaphore);
        }
        else if (prevplayer[0].Center > HostToClient.players[0].currentCenter)
        {
            //moving left

            G8RTOS_WaitSemaphore(&LCDSemaphore);

            //erase the difference
            if (prevplayer[0].Center + PADDLE_LEN_D2 + PRINT_OFFSET
                    < HORIZ_CENTER_MAX_BALL)
                LCD_DrawRectangle(
                        HostToClient.players[0].currentCenter + PADDLE_LEN_D2,
                        prevplayer[0].Center + PADDLE_LEN_D2 + PRINT_OFFSET,
                        ARENA_MAX_Y - PADDLE_WID,
                        ARENA_MAX_Y, LCD_BLACK);
            else
                LCD_DrawRectangle(
                        HostToClient.players[0].currentCenter + PADDLE_LEN_D2,
                        prevplayer[0].Center + PADDLE_LEN_D2,
                        ARENA_MAX_Y - PADDLE_WID,
                        ARENA_MAX_Y, LCD_BLACK);

            //draw the difference
            LCD_DrawRectangle(
                    HostToClient.players[0].currentCenter - PADDLE_LEN_D2,
                    prevplayer[0].Center - PADDLE_LEN_D2,
                    ARENA_MAX_Y - PADDLE_WID,
                    ARENA_MAX_Y, HostToClient.players[0].color);

            G8RTOS_SignalSemaphore(&LCDSemaphore);
        }

        //If the paddles center for client has changed
        if (prevplayer[1].Center < HostToClient.players[1].currentCenter)
        {
            //moving right

            G8RTOS_WaitSemaphore(&LCDSemaphore);

            //erase the difference
            if (prevplayer[1].Center - PADDLE_LEN_D2 - PRINT_OFFSET
                    > HORIZ_CENTER_MIN_BALL)
                LCD_DrawRectangle(
                        prevplayer[1].Center - PADDLE_LEN_D2 - PRINT_OFFSET,
                        HostToClient.players[1].currentCenter - PADDLE_LEN_D2,
                        ARENA_MIN_Y,
                        ARENA_MIN_Y + PADDLE_WID, LCD_BLACK);
            else
                LCD_DrawRectangle(
                        prevplayer[1].Center - PADDLE_LEN_D2,
                        HostToClient.players[1].currentCenter - PADDLE_LEN_D2,
                        ARENA_MIN_Y,
                        ARENA_MIN_Y + PADDLE_WID, LCD_BLACK);

            //draw the difference
            LCD_DrawRectangle(
                    prevplayer[1].Center + PADDLE_LEN_D2,
                    HostToClient.players[1].currentCenter + PADDLE_LEN_D2,
                    ARENA_MIN_Y,
                    ARENA_MIN_Y + PADDLE_WID, HostToClient.players[1].color);

            G8RTOS_SignalSemaphore(&LCDSemaphore);
        }
        else if (prevplayer[1].Center > HostToClient.players[1].currentCenter)
        {
            //moving left

            G8RTOS_WaitSemaphore(&LCDSemaphore);

            //erase the difference
            if (prevplayer[1].Center + PADDLE_LEN_D2 + PRINT_OFFSET
                    < HORIZ_CENTER_MAX_BALL)
                LCD_DrawRectangle(
                        HostToClient.players[1].currentCenter + PADDLE_LEN_D2,
                        prevplayer[1].Center + PADDLE_LEN_D2 + PRINT_OFFSET,
                        ARENA_MIN_Y,
                        ARENA_MIN_Y + PADDLE_WID, LCD_BLACK);
            else
                LCD_DrawRectangle(
                        HostToClient.players[1].currentCenter + PADDLE_LEN_D2,
                        prevplayer[1].Center + PADDLE_LEN_D2,
                        ARENA_MIN_Y,
                        ARENA_MIN_Y + PADDLE_WID, LCD_BLACK);

            //draw the difference
            LCD_DrawRectangle(
                    HostToClient.players[1].currentCenter - PADDLE_LEN_D2,
                    prevplayer[1].Center - PADDLE_LEN_D2,
                    ARENA_MIN_Y,
                    ARENA_MIN_Y + PADDLE_WID, HostToClient.players[1].color);

            G8RTOS_SignalSemaphore(&LCDSemaphore);
        }

        //update the previous positions
        prevplayer[0].Center = HostToClient.players[0].currentCenter;
        prevplayer[1].Center = HostToClient.players[1].currentCenter;

        OS_Sleep(20);
    }
}

/*
 * Thread to update LEDs based on score
 */
void MoveLEDs()
{
    uint8_t prevScores[2];
    uint16_t hostLEDs = 0, clientLEDs = 0;

    prevScores[0] = HostToClient.LEDScores[0];
    prevScores[1] = HostToClient.LEDScores[1];

    while (1)
    {
        if (myPlayer == Host && HostToClient.LEDScores[0] != prevScores[0])
        {
            //change the LED variable to increment based on the difference
            for (int i = 0; i < abs(HostToClient.LEDScores[0] - prevScores[0]);
                    i++)
                hostLEDs = (hostLEDs << 1) | 1;

            G8RTOS_WaitSemaphore(&I2CSemaphore);

            //changed the LED output
            LP3943_LedModeSet(RED, hostLEDs);

            G8RTOS_SignalSemaphore(&I2CSemaphore);

            //reset the previous score to the current score
            prevScores[0] = HostToClient.LEDScores[0];
        }
        else if (myPlayer == Client
                && HostToClient.LEDScores[1] != prevScores[1])
        {
            //change the LED variable to increment based on the difference
            for (int i = 0; i < abs(HostToClient.LEDScores[1] - prevScores[1]);
                    i++)
                clientLEDs = (clientLEDs << 1) | 1;

            G8RTOS_WaitSemaphore(&I2CSemaphore);

            //changed the LED output
            LP3943_LedModeSet(BLUE, clientLEDs);

            G8RTOS_SignalSemaphore(&I2CSemaphore);

            //reset the previous score to the current score
            prevScores[1] = HostToClient.LEDScores[1];
        }
    }
}

void IdleThread()
{
    while (1)
    {
    }
}

/*********************** Common Threads ***********************/

/*********************** Host Threads ***********************/

/*
 * Thread for the host to create a game
 */
void CreateGame()
{
    /* Initialize Host */
    HostToClient.players[0].color = LCD_RED;
    HostToClient.players[0].currentCenter = PADDLE_X_CENTER;
    HostToClient.players[0].position = BOTTOM;

    /* Initialize Client */
    HostToClient.players[1].color = LCD_BLUE;
    HostToClient.players[1].currentCenter = PADDLE_X_CENTER;
    HostToClient.players[1].position = TOP;

    HostToClient.overallScores[0] = '0';
    HostToClient.overallScores[1] = '0';

    /* Connect Host */
    initCC3100(Host);

    /* Wait for confirmation from Client */
    while (ReceiveData(packetClient, sizeof(packetClient)) < 0)
        ;

    //empty the data into the specific player gamestate
    emptyPacket(&ClientToHost, &packetClient);
    HostToClient.player = ClientToHost.player;

    /* Turn on LED to show Client has connected */
    BITBAND_PERI(P1->OUT, 0) = !BITBAND_PERI(P1->OUT, 0);

    //fill a packet to send to client for server response
    fillPacket(&HostToClient, &packetHost);

    //Send server response
    SendData(packetHost, HostToClient.player.IP_address, sizeof(packetHost));

    /* Draw Arena and White Line Segments */
    LCD_DrawRectangle(MIN_SCREEN_X, MAX_SCREEN_X, MIN_SCREEN_Y, MAX_SCREEN_Y,
    LCD_BLACK);
    LCD_DrawRectangle(ARENA_MIN_X - 1, ARENA_MIN_X, ARENA_MIN_Y, ARENA_MAX_Y,
    LCD_WHITE);
    LCD_DrawRectangle(ARENA_MAX_X, ARENA_MAX_X + 1, ARENA_MIN_Y, ARENA_MAX_Y,
    LCD_WHITE);

    uint8_t strH[2], strC[2];
    sprintf(strC, "%c", HostToClient.overallScores[1]);

    LCD_Text(MIN_SCREEN_X + 10, MIN_SCREEN_Y + 4, "0",
             HostToClient.players[1].color);
    LCD_Text(MIN_SCREEN_X + 18, MIN_SCREEN_Y + 4, strC,
             HostToClient.players[1].color);

    sprintf(strH, "%c", HostToClient.overallScores[0]);

    LCD_Text(MIN_SCREEN_X + 10, MAX_SCREEN_Y - 20, "0",
             HostToClient.players[0].color);
    LCD_Text(MIN_SCREEN_X + 18, MAX_SCREEN_Y - 20, strH,
             HostToClient.players[0].color);

    /* Draw Players in initial positions */
    LCD_DrawRectangle(HostToClient.players[1].currentCenter - PADDLE_LEN_D2,
                      HostToClient.players[1].currentCenter + PADDLE_LEN_D2,
                      ARENA_MIN_Y,
                      ARENA_MIN_Y + PADDLE_WID, HostToClient.players[1].color);
    LCD_DrawRectangle(HostToClient.players[0].currentCenter - PADDLE_LEN_D2,
                      HostToClient.players[0].currentCenter + PADDLE_LEN_D2,
                      ARENA_MAX_Y - PADDLE_WID,
                      ARENA_MAX_Y, HostToClient.players[0].color);

    /* Add threads that need to run */
    G8RTOS_AddThread(DrawObjects, 2, "DrawObjects");
    G8RTOS_AddThread(ReceiveDataFromClient, 4, "ReceiveClient");
    G8RTOS_AddThread(SendDataToClient, 5, "SendClient");
    G8RTOS_AddThread(ReadJoystickHost, 6, "JoyStickHost");
    G8RTOS_AddThread(GenerateBall, 7, "GenerateBall");
    G8RTOS_AddThread(MoveLEDs, 200, "MoveLEDs");
    G8RTOS_AddThread(IdleThread, 254, "Idle");

    /* Kill this thread */
    G8RTOS_KillSelf();
}

/*
 * Thread that sends game state to client
 */
void SendDataToClient()
{
    while (1)
    {
        // Fills the packet for the client
        fillPacket(&HostToClient, &packetHost);

        // Sends the packet to the client
        G8RTOS_WaitSemaphore(&CC3100Semaphore);
        SendData(packetHost, HostToClient.player.IP_address,
                 sizeof(packetHost));
        G8RTOS_SignalSemaphore(&CC3100Semaphore);

        // Checks to see if the game is done
        if (HostToClient.gameDone)
            G8RTOS_AddThread(EndOfGameHost, 1, "EndGameHost"); // Thread to end the game

        // Sleeps for 5ms (good amount of time for synchronization)
        OS_Sleep(5);
    }
}

/*
 * Thread that receives UDP packets from client
 */
void ReceiveDataFromClient()
{
    while (1)
    {
        G8RTOS_WaitSemaphore(&CC3100Semaphore);
        while (ReceiveData(packetClient, sizeof(packetClient)) < 0)
        {
            G8RTOS_SignalSemaphore(&CC3100Semaphore);

            // Sleeps to avoid deadlock
            OS_Sleep(1);

            G8RTOS_WaitSemaphore(&CC3100Semaphore);
        }
        G8RTOS_SignalSemaphore(&CC3100Semaphore);

        // Empties the packets content
        emptyPacket(&ClientToHost, &packetClient);

        //  Updates the players current center with the received displacement
        HostToClient.players[1].currentCenter +=
                ClientToHost.player.displacement;

        OS_Sleep(2);
    }
}

/*
 * Generate Ball thread
 */
void GenerateBall()
{
    while (1)
    {
        // Checks to see if the number of balls is < max
        if (HostToClient.numberOfBalls < MAX_NUM_OF_BALLS)
        {
            G8RTOS_AddThread(MoveBall, 3, "MoveBall");
            HostToClient.numberOfBalls++;          // Increments number of balls

        }

        /* Will sleep proportional to amount of balls currently in play.
         The mores balls in play, the longer is will take to spawn another ball.
         The balls spawns are seperated by 1 second intervals.  */
        OS_Sleep(HostToClient.numberOfBalls * 1000);

    }
}

/*
 * Thread to read host's joystick
 */
void ReadJoystickHost()
{
    int16_t x_coord, y_coord;
    while (1)
    {
        //Read the joystick
        GetJoystickCoordinates(&x_coord, &y_coord);

        //check joystick value
        if (x_coord > 2000)
            HostToClient.player.displacement = -4;
        else if (x_coord < -2000)
            HostToClient.player.displacement = 4;
        else
            //reset displacement
            HostToClient.player.displacement = 0;

        //Sleep before updating bottom player's position
        OS_Sleep(10);

        //modify position
        HostToClient.players[0].currentCenter +=
                HostToClient.player.displacement;

    }
}

/*
 * Thread to move a single ball
 */
void MoveBall()
{
    uint16_t index;
    int32_t w, h, dx0, dx1, dy0, dy1, wy, hx;
    int8_t signX = 1, signY = 1;

    for (int i = 0; i < MAX_NUM_OF_BALLS; i++)
    {
        if (HostToClient.balls[i].alive == 0)
        {
            //initialize the balls current centers, velocities, and color
            HostToClient.balls[i].alive = 1;
            HostToClient.balls[i].currentCenterX = (rand()
                    % (HORIZ_CENTER_MAX_BALL - HORIZ_CENTER_MIN_BALL - 10)
                    + (HORIZ_CENTER_MIN_BALL + 10));
            HostToClient.balls[i].currentCenterY = (rand()
                    % (VERT_CENTER_MAX_BALL / 2) + (VERT_CENTER_MIN_BALL + 60));
            HostToClient.balls[i].velocityX = (rand() % 7 + -2);
            HostToClient.balls[i].velocityY = (rand() % 7 + -2);

            if (HostToClient.balls[i].velocityX == 0)
                HostToClient.balls[i].velocityX = 1;
            if (HostToClient.balls[i].velocityY == 0)
                HostToClient.balls[i].velocityY = 1;

            HostToClient.balls[i].color = LCD_WHITE;
            index = i;
            break;
        }
    }

    while (1)
    {
        /*  Checking for collision given the current center and the velocity.
         *  If collision occurs, adjust velocity and color accordingly. */

        w = WIDTH_TOP_OR_BOTTOM + WIGGLE_ROOM;// + (abs(HostToClient.balls[index].velocityX) >> 1);
        h = HEIGHT_TOP_OR_BOTTOM;// + (abs(HostToClient.balls[index].velocityY) >> 1);
        dx0 = HostToClient.players[0].currentCenter + 1
                - HostToClient.balls[index].currentCenterX;
        // bottom player needed an extra buffer for dy0 to avoid erasing the paddle during collisions
        dy0 = (BOTTOM_PLAYER_CENTER_Y - 1)
                - HostToClient.balls[index].currentCenterY;
        dx1 = HostToClient.players[1].currentCenter + 1
                - HostToClient.balls[index].currentCenterX;
        dy1 = (TOP_PLAYER_CENTER_Y + 1)
                - HostToClient.balls[index].currentCenterY;

        if (HostToClient.balls[index].velocityX > 0)
            signX = 1;
        else
            signX = -1;

        if (HostToClient.balls[index].velocityY > 0)
            signY = 1;
        else
            signY = -1;

        //The first if statement handles Host collisions
        if (abs(dx0) <= w && abs(dy0) <= h)
        {
            /* collision! */
            wy = w * dy0;
            hx = h * dx0;

            HostToClient.balls[index].color = HostToClient.players[0].color;

            if (wy > hx)
            {
                if (wy > -hx)
                {
                    /* collision at the top */
                    //if-else if tree to handle the paddles 3 sections
                    if (dx0 >= 10 && HostToClient.balls[index].velocityX >= 0)
                    { //left side
                        if (HostToClient.balls[index].velocityX == 0)
                            HostToClient.balls[index].velocityX = -1
                                    * _1_3_PADDLE;
                        else
                            HostToClient.balls[index].velocityX = -1
                                    * (abs(HostToClient.balls[index].velocityX)
                                            + _1_3_PADDLE) * signX;

                    }
                    else if (dx0 <= -10
                            && HostToClient.balls[index].velocityX <= 0)
                    { //right side
                        if (HostToClient.balls[index].velocityX == 0)
                            HostToClient.balls[index].velocityX = 1
                                    * _1_3_PADDLE;
                        else
                            HostToClient.balls[index].velocityX = -1
                                    * (abs(HostToClient.balls[index].velocityX)
                                            + _1_3_PADDLE) * signX;

                    }
                    else if (dx0 < 10 && dx0 > -10)
                    { //middle
                        HostToClient.balls[index].velocityX = 0;
                    }

                    HostToClient.balls[index].velocityY = -1
                            * (abs(HostToClient.balls[index].velocityY)
                                    + _1_3_PADDLE) * signY;
                }
                else
                {
                    /* on the left */
                    HostToClient.balls[index].velocityX = -1
                            * (abs(HostToClient.balls[index].velocityX)
                                    + _1_3_PADDLE) * signX;
                }
            }
            else
            {
                    /* on the right */
                    HostToClient.balls[index].velocityX = -1
                            * (abs(HostToClient.balls[index].velocityX)
                                    + _1_3_PADDLE) * signX;

            }
        }
        else if (abs(dx1) <= w && abs(dy1) <= h) //The else if statement handles client collisions
        {
            /* collision! */
            wy = w * dy1;
            hx = h * dx1;

            HostToClient.balls[index].color = HostToClient.players[1].color;

            if (wy > hx)
            {
                    /* on the left */
                    HostToClient.balls[index].velocityX = -1
                            * (abs(HostToClient.balls[index].velocityX)
                                    + _1_3_PADDLE) * signX;
            }
            else
            {
                if (wy > -hx)
                    /* on the right */
                    HostToClient.balls[index].velocityX = -1
                            * (abs(HostToClient.balls[index].velocityX)
                                    + _1_3_PADDLE) * signX;
                else /* at the bottom */
                {
                    //if-else if tree to handle the paddles 3 sections
                    if (dx1 >= 10 && HostToClient.balls[index].velocityX >= 0)
                    { //left side
                        if (HostToClient.balls[index].velocityX == 0)
                            HostToClient.balls[index].velocityX = -1
                                    * _1_3_PADDLE;
                        else
                            HostToClient.balls[index].velocityX = -1
                                    * (abs(HostToClient.balls[index].velocityX)
                                            + _1_3_PADDLE) * signX;

                    }
                    else if (dx1 <= -10
                            && HostToClient.balls[index].velocityX <= 0)
                    { //right side
                        if (HostToClient.balls[index].velocityX == 0)
                            HostToClient.balls[index].velocityX = 1
                                    * _1_3_PADDLE;
                        else
                            HostToClient.balls[index].velocityX = -1
                                    * (abs(HostToClient.balls[index].velocityX)
                                            + _1_3_PADDLE) * signX;

                    }
                    else if (dx1 < 10 && dx1 > -10)
                    { //middle
                        HostToClient.balls[index].velocityX = 0;
                    }

                    HostToClient.balls[index].velocityY = -1
                            * (abs(HostToClient.balls[index].velocityY)
                                    + _1_3_PADDLE) * signY;
                }
            }
        }

        //Checking Arena Edge Collisions
        if ((HostToClient.balls[index].currentCenterX + BALL_SIZE_D2 + abs(HostToClient.balls[index].velocityX)
                >= ARENA_MAX_X - 2)
                || (HostToClient.balls[index].currentCenterX - BALL_SIZE_D2 - abs(HostToClient.balls[index].velocityX)
                        <= ARENA_MIN_X + 2))
        {
            HostToClient.balls[index].velocityX = -1
                    * (HostToClient.balls[index].velocityX);
        }
        else if ((HostToClient.balls[index].currentCenterY
                >= ARENA_MAX_Y)
                || (HostToClient.balls[index].currentCenterY
                        <= ARENA_MIN_Y))
        {
            /*  If the ball passes the boundary edge, adjust
             score, account for the game possibly ending, and
             kill self   */

            //kill the ball and decrement number of current balls
            HostToClient.balls[index].alive = false;
            HostToClient.balls[index].killed = true;
            HostToClient.numberOfBalls--;

            //If pass boundary edge, increment score on LEDs
            if ((HostToClient.balls[index].currentCenterY + BALL_SIZE_D2
                    >= ARENA_MAX_Y)
                    && HostToClient.balls[index].color != LCD_WHITE) //the client scored
                HostToClient.LEDScores[1]++;
            else if ((HostToClient.balls[index].currentCenterY - BALL_SIZE_D2
                    <= ARENA_MIN_Y)
                    && HostToClient.balls[index].color != LCD_WHITE) //the host scored
                HostToClient.LEDScores[0]++;

            //if that score has reached 16, game is done, increment overall score, if host, winner is true
            if (HostToClient.LEDScores[1] >= 16)
            {
                HostToClient.gameDone = true;
                HostToClient.overallScores[1]++;
                HostToClient.winner = false;    //client
            }

            if (HostToClient.LEDScores[0] >= 16)
            {
                HostToClient.gameDone = true;   //host
                HostToClient.overallScores[0]++;
                HostToClient.winner = true;
            }

            //kill the thread
            G8RTOS_KillSelf();
        }

        /*  Otherwise, just move the ball in its current
         direction according to its velocity  */
        if (HostToClient.balls[index].velocityX >= MAX_BALL_SPEED)
            HostToClient.balls[index].velocityX = MAX_BALL_SPEED;
        if (HostToClient.balls[index].velocityY >= MAX_BALL_SPEED)
            HostToClient.balls[index].velocityY = MAX_BALL_SPEED;

        if (!HostToClient.balls[index].killed)
        {
            HostToClient.balls[index].currentCenterX +=
                    HostToClient.balls[index].velocityX;
            HostToClient.balls[index].currentCenterY +=
                    HostToClient.balls[index].velocityY;
        }

        /* Sleep for 35ms  */
        OS_Sleep(35);
    }
}

void EndOfGameHost()
{
    //Wait for all semaphores to release
    G8RTOS_WaitSemaphore(&LCDSemaphore);
    G8RTOS_WaitSemaphore(&CC3100Semaphore);
    G8RTOS_WaitSemaphore(&I2CSemaphore);

    //Kill all threads
    G8RTOS_KillAll();

    //Kills all the ball threads
    for (int i = 0; i < MAX_NUM_OF_BALLS; i++)
    {
        HostToClient.balls[i].alive = false;
        HostToClient.balls[i].killed = false;
    }

    //Reset number of balls in the game
    HostToClient.numberOfBalls = 0;

    //Re-initialize semaphores
    G8RTOS_InitSemaphore(&CC3100Semaphore, 1);
    G8RTOS_InitSemaphore(&LCDSemaphore, 1);
    G8RTOS_InitSemaphore(&I2CSemaphore, 1);

    //Clear the screen with the winner's color
    if (HostToClient.winner == true) //host wins
        LCD_Clear(HostToClient.players[0].color);
    else
        //client wins
        LCD_Clear(HostToClient.players[1].color);

    // Initialize the top button on the daughter board
    P4->DIR &= ~BIT4;   // Sets the direction to input
    P4->IFG &= ~BIT4;   // P4.4 IFG cleared
    P4->IE |= BIT4;     // Enable interrupt on P4.4
    P4->IES |= BIT4;    // high-to-low resolution
    P4->REN |= BIT4;    // Pull-up resister
    P4->OUT |= BIT4;    // Sets res to pull-up

    // Prints a message for the host to start a new game
    LCD_Text(30, 120, "Press top button to play again!", LCD_BLACK);

    /* aperiodic thread that waits for the host’s button press
     * (the client will just be waiting on the host to start a new game
     * */
    G8RTOS_AddAPeriodicEvent(ButtonTap, 2, PORT4_IRQn);

    //wait for button press by host
    while (!buttonFlag)
        ;

    //reset flag
    buttonFlag = 0;

    //acknowledge the game restarting
    HostToClient.player.acknowledge = true;

    //fill a packet to send to client for server response
    fillPacket(&HostToClient, &packetHost);

    //Send server response
    SendData(packetHost, HostToClient.player.IP_address, sizeof(packetHost));

    //Initialize the board again
    InitBoardState();

    //Kill this thread
    G8RTOS_KillSelf();
}

void ButtonTap()
{
    uint32_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P4);

    if (status & GPIO_PIN4)
    {
        if (P4IV == (GPIO_INPUT_PIN_LOW))
        {
            buttonFlag = 1;
            P4->IE &= ~BIT4;
            return;
        }
    }

    return;
}

/*********************** Host Threads ***********************/

/*********************** Client Threads ***********************/

void JoinGame()
{
    /* Connect Client */
    initCC3100(Client);

    //Initialize the client and fill the packet to send over wifi
    ClientToHost.player.IP_address = getLocalIP();
    ClientToHost.player.acknowledge = false;
    ClientToHost.player.displacement = 0;
    ClientToHost.player.joined = true;
    ClientToHost.player.playerNumber = 1;
    ClientToHost.player.ready = true;

    fillPacket(&ClientToHost, &packetClient);

    /* Send packet to host of clients initialization */
    SendData(packetClient, HOST_IP_ADDR, sizeof(packetClient));

    /* Wait for server response */
    while (ReceiveData(packetHost, sizeof(packetHost)) < 0)
        ;
    emptyPacket(&HostToClient, &packetHost);

    /* Turn on LED to show Client has connected */
    BITBAND_PERI(P1->OUT, 0) = !BITBAND_PERI(P1->OUT, 0);

    /* Draw Arena and White Line Segments */
    LCD_DrawRectangle(MIN_SCREEN_X, MAX_SCREEN_X, MIN_SCREEN_Y, MAX_SCREEN_Y,
    LCD_BLACK);

    uint8_t strH[2], strC[2];
    sprintf(strC, "%c", HostToClient.overallScores[1]);

    LCD_Text(MIN_SCREEN_X + 10, MIN_SCREEN_Y + 4, "0",
             HostToClient.players[1].color);
    LCD_Text(MIN_SCREEN_X + 18, MIN_SCREEN_Y + 4, strC,
             HostToClient.players[1].color);

    sprintf(strH, "%c", HostToClient.overallScores[0]);

    LCD_Text(MIN_SCREEN_X + 10, MAX_SCREEN_Y - 20, "0",
             HostToClient.players[0].color);
    LCD_Text(MIN_SCREEN_X + 18, MAX_SCREEN_Y - 20, strH,
             HostToClient.players[0].color);

    LCD_DrawRectangle(ARENA_MIN_X - 1, ARENA_MIN_X, ARENA_MIN_Y, ARENA_MAX_Y,
    LCD_WHITE);
    LCD_DrawRectangle(ARENA_MAX_X, ARENA_MAX_X + 1, ARENA_MIN_Y, ARENA_MAX_Y,
    LCD_WHITE);

    /* Draw Players in initial positions */
    LCD_DrawRectangle(HostToClient.players[1].currentCenter - PADDLE_LEN_D2,
                      HostToClient.players[1].currentCenter + PADDLE_LEN_D2,
                      ARENA_MIN_Y,
                      ARENA_MIN_Y + PADDLE_WID, HostToClient.players[1].color);
    LCD_DrawRectangle(HostToClient.players[0].currentCenter - PADDLE_LEN_D2,
                      HostToClient.players[0].currentCenter + PADDLE_LEN_D2,
                      ARENA_MAX_Y - PADDLE_WID,
                      ARENA_MAX_Y, HostToClient.players[0].color);

    /* Add threads that need to run */
    G8RTOS_AddThread(DrawObjects, 2, "DrawObjects");
    G8RTOS_AddThread(ReceiveDataFromHost, 3, "ReceiveHost");
    G8RTOS_AddThread(SendDataToHost, 4, "SendHost");
    G8RTOS_AddThread(ReadJoystickClient, 5, "JoyStickClient");
    G8RTOS_AddThread(MoveLEDs, 200, "MoveLEDs");
    G8RTOS_AddThread(IdleThread, 254, "Idle");

    /* Kill this thread */
    G8RTOS_KillSelf();
}

/*
 * Thread that receives game state packets from host
 */
void ReceiveDataFromHost()
{
    while (1)
    {
        G8RTOS_WaitSemaphore(&CC3100Semaphore);
        while (ReceiveData(packetHost, sizeof(packetHost)) < 0)
        {
            G8RTOS_SignalSemaphore(&CC3100Semaphore);

            // Sleeps to avoid deadlock
            OS_Sleep(1);

            G8RTOS_WaitSemaphore(&CC3100Semaphore);
        }
        G8RTOS_SignalSemaphore(&CC3100Semaphore);

        // Empties the packets content
        emptyPacket(&HostToClient, &packetHost);

        //Check if game is done
        if (HostToClient.gameDone)
            G8RTOS_AddThread(EndOfGameClient, 1, "EndGameClient");

        OS_Sleep(2);
    }
}

/*
 * Thread that sends UDP packets to host
 */
void SendDataToHost()
{
    while (1)
    {
        //fill packet to send
        fillPacket(&ClientToHost, &packetClient);

        //send packet
        G8RTOS_WaitSemaphore(&CC3100Semaphore);
        SendData(packetClient, HOST_IP_ADDR, sizeof(packetClient));
        G8RTOS_SignalSemaphore(&CC3100Semaphore);

        //adjust clients displacement after being sent once
        ClientToHost.player.displacement = 0;

        OS_Sleep(5);
    }
}

/*
 * Thread to read client's joystick
 */
void ReadJoystickClient()
{
    int16_t x_coord, y_coord;
    while (1)
    {
        //Read the joystick
        GetJoystickCoordinates(&x_coord, &y_coord);

        //check joystick value, adjust displacement
        if (x_coord > 2000)
        {
            ClientToHost.player.displacement = -4;
        }
        else if (x_coord < -2000)
        {
            ClientToHost.player.displacement = 4;
        }
        else
            ClientToHost.player.displacement = 0;

        //sleep 10 ms
        OS_Sleep(10);
    }
}

/*
 * End of game for the client
 */
void EndOfGameClient()
{
    //Wait for all semaphores to release
    G8RTOS_WaitSemaphore(&CC3100Semaphore);
    G8RTOS_WaitSemaphore(&LCDSemaphore);
    G8RTOS_WaitSemaphore(&I2CSemaphore);

    //Kill all threads
    G8RTOS_KillAll();

    //Kills all the ball threads
    for (int i = 0; i < MAX_NUM_OF_BALLS; i++)
    {
        HostToClient.balls[i].alive = false;
        HostToClient.balls[i].killed = false;
    }

    //Reset number of balls in the game
    HostToClient.numberOfBalls = 0;

    //Re-initialize semaphores
    G8RTOS_InitSemaphore(&CC3100Semaphore, 1);
    G8RTOS_InitSemaphore(&LCDSemaphore, 1);
    G8RTOS_InitSemaphore(&I2CSemaphore, 1);

    //Clear the screen with the winner's color
    if (HostToClient.winner == true) //host wins
        LCD_Clear(HostToClient.players[0].color);
    else
        //client wins
        LCD_Clear(HostToClient.players[1].color);

    // Prints a message for the host to start a new game
    LCD_Text(100, 120, "Waiting on Host", LCD_BLACK);

    /* Waiting for server response */
    while (ReceiveData(packetClient, sizeof(packetClient)) < 0)
        ;

    emptyPacket(&HostToClient, &packetClient);

    if (HostToClient.player.acknowledge == true)
        //Initialize the board again
        InitBoardState();

    //Kill this thread
    G8RTOS_KillSelf();
}

/*********************** Client Threads ***********************/

/*********************** Public Functions *********************/

void InitBoardState()
{

    /* Initialize Host */
    HostToClient.players[0].color = LCD_RED;
    HostToClient.players[0].currentCenter = PADDLE_X_CENTER;
    HostToClient.players[0].position = BOTTOM;

    /* Initialize Client */
    HostToClient.players[1].color = LCD_BLUE;
    HostToClient.players[1].currentCenter = PADDLE_X_CENTER;
    HostToClient.players[1].position = TOP;

    /* Initialize Client variables */
    HostToClient.player.acknowledge = 0;
    HostToClient.player.displacement = 0;
    HostToClient.player.joined = 1;
    HostToClient.player.playerNumber = 1;
    HostToClient.player.ready = 1;

    //Reset Scores
    HostToClient.LEDScores[0] = 0;
    HostToClient.LEDScores[1] = 0;
    HostToClient.gameDone = false;

    /* Draw Arena and White Line Segments */
    LCD_DrawRectangle(MIN_SCREEN_X, MAX_SCREEN_X, MIN_SCREEN_Y, MAX_SCREEN_Y,
    LCD_BLACK);
    LCD_DrawRectangle(ARENA_MIN_X - 1, ARENA_MIN_X, ARENA_MIN_Y, ARENA_MAX_Y,
    LCD_WHITE);
    LCD_DrawRectangle(ARENA_MAX_X, ARENA_MAX_X + 1, ARENA_MIN_Y, ARENA_MAX_Y,
    LCD_WHITE);

    uint8_t strH[2], strC[2];
    sprintf(strC, "%c", HostToClient.overallScores[1]);

    LCD_Text(MIN_SCREEN_X + 10, MIN_SCREEN_Y + 4, "0",
             HostToClient.players[1].color);
    LCD_Text(MIN_SCREEN_X + 18, MIN_SCREEN_Y + 4, strC,
             HostToClient.players[1].color);

    sprintf(strH, "%c", HostToClient.overallScores[0]);

    LCD_Text(MIN_SCREEN_X + 10, MAX_SCREEN_Y - 20, "0",
             HostToClient.players[0].color);
    LCD_Text(MIN_SCREEN_X + 18, MAX_SCREEN_Y - 20, strH,
             HostToClient.players[0].color);

    /* Draw Players in initial positions */
    LCD_DrawRectangle(HostToClient.players[1].currentCenter - PADDLE_LEN_D2,
                      HostToClient.players[1].currentCenter + PADDLE_LEN_D2,
                      ARENA_MIN_Y,
                      ARENA_MIN_Y + PADDLE_WID, HostToClient.players[1].color);
    LCD_DrawRectangle(HostToClient.players[0].currentCenter - PADDLE_LEN_D2,
                      HostToClient.players[0].currentCenter + PADDLE_LEN_D2,
                      ARENA_MAX_Y - PADDLE_WID,
                      ARENA_MAX_Y, HostToClient.players[0].color);

    /* Add threads that need to run and clear LEDs */
    if (myPlayer == Host)
    {
        LP3943_LedModeSet(RED, UNIT_OFF);
        G8RTOS_AddThread(DrawObjects, 2, "DrawObjects");
        G8RTOS_AddThread(ReceiveDataFromClient, 4, "ReceiveClient");
        G8RTOS_AddThread(SendDataToClient, 5, "SendClient");
        G8RTOS_AddThread(ReadJoystickHost, 6, "JoyStickHost");
        G8RTOS_AddThread(GenerateBall, 7, "GenerateBall");
        G8RTOS_AddThread(MoveLEDs, 200, "MoveLEDs");
        G8RTOS_AddThread(IdleThread, 254, "Idle");
    }
    else
    {
        LP3943_LedModeSet(BLUE, UNIT_OFF);
        G8RTOS_AddThread(DrawObjects, 2, "DrawObjects");
        G8RTOS_AddThread(ReceiveDataFromHost, 3, "ReceiveHost");
        G8RTOS_AddThread(SendDataToHost, 4, "SendHost");
        G8RTOS_AddThread(ReadJoystickClient, 5, "JoyStickClient");
        G8RTOS_AddThread(MoveLEDs, 200, "MoveLEDs");
        G8RTOS_AddThread(IdleThread, 254, "Idle");
    }
}

/*********************** Public Functions *********************/

