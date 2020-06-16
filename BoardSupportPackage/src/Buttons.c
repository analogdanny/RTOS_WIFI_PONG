/*
 * Buttons.c
 *
 *  Created on: Jan 16, 2020
 *      Author: Danny
 */
#include "Buttons.h"

void Port4InterInit()
{
    P4->DIR &= ~BIT4;
    P4->IFG &= ~BIT4;   // P4.4 IFG cleared
    P4->IE |= BIT4;     // Enable interrupt on P4.4
    P4->IES |= BIT4;    // high-to-low transition
    P4->REN |= BIT4;    // Pull-up resister
    P4->OUT |= BIT4;    // Sets res to pull-up

    P4->DIR &= ~BIT5;
    P4->IFG &= ~BIT5;   // P4.5 IFG cleared
    P4->IE |= BIT5;     // Enable interrupt on P4.4
    P4->IES |= BIT5;    // high-to-low transition
    P4->REN |= BIT5;    // Pull-up resister
    P4->OUT |= BIT5;    // Sets res to pull-up
}

void Port5InterInit()
{
    P5->DIR &= ~BIT5;
    P5->IFG &= ~BIT5;   // P5.5 IFG cleared
    P5->IE |= BIT5;     // Enable interrupt on P4.4
    P5->IES |= BIT5;    // high-to-low transition
    P5->REN |= BIT5;    // Pull-up resister
    P5->OUT |= BIT5;    // Sets res to pull-up

    P5->DIR &= ~BIT4;
    P5->IFG &= ~BIT4;   // P5.4 IFG cleared
    P5->IE |= BIT4;     // Enable interrupt on P4.4
    P5->IES |= BIT4;    // high-to-low transition
    P5->REN |= BIT4;    // Pull-up resister
    P5->OUT |= BIT4;    // Sets res to pull-up
}
