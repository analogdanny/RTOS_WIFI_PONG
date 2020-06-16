#include "RGBLeds.h"
#include "ClockSys.h"

#ifndef RGBLEDS_C_
#define RGBLEDS_C_

void LP3943_ColorSet(uint32_t unit, uint32_t LED_MODE, uint32_t PWM_DATA)
{
    EUSCI_B2->I2CSA = 0x60 + unit;

    /* All LEDs off */
    if(PWM_DATA == UNIT_OFF)
    {
        write_I2C(0, LED_AUTO);
    }
    else
    {
        write_I2C(PWM_DATA, PWM_AUTO);
        write_I2C(LED_MODE, LED_AUTO);
    }
}

void LP3943_LedModeSet(uint32_t unit, uint16_t LED_DATA)
{
    /* This function will set each of the LEDs to the desired operating mode
     * The operating modes are on and off in this function */

    EUSCI_B2->I2CSA = 0x60 + unit;

    /* All LEDs off */
    if(LED_DATA == UNIT_OFF)
    {
        write_I2C(0, LED_AUTO);
    }
    else{
        uint32_t newData = 0;
        uint8_t bit = 1;

        /* This for loop recreates the data in a 32 bit format for each
         * LED based off the initial 16 bit value to be on or off*/
        for(int i = 0, j = 0; i < 32; i += 2, j++ )
        {
            if((LED_DATA & (bit << j)) != 0)
            {
                newData |= bit << i;
            }
        }
        write_I2C(newData, LED_AUTO);
    }
}

void init_RGBLEDS()
{
    /* Software reset enable */
    EUSCI_B2->CTLW0 = UCSWRST;

    /* Initialize I2C Master*/
    /* Set as master, I2c mode, Clock sync, SMCLK source, Transmitter */
    EUSCI_B2->CTLW0 &= (~UCSLA10); // 7 bit slave address
    EUSCI_B2->CTLW0 |= UCMODE_3  // I2C Mode
                        | UCMST     // Master Mode
                        | UCSYNC    // Clock sync
                        | UCSSEL_2 // select SMCLK
                        | UCTR;     // Transmitter

    uint32_t freq = ClockSys_GetSysFreq();

    /* ~7 = 3MHz(Clk speed)/400k Hz(baudrate) for bitrate */
    EUSCI_B2->BRW = freq/400000;

    /* Set P3.6 as UCB2_SDA and 3.7 as UCB2_SLC */
    P3SEL0 |= BIT7 | BIT6;      // Need 1's for SEL0
    P3SEL1 &= ~(BIT7 | BIT6);    // Need 0's for SEL1

    /* Bitwise AND all bits except UCSWRST */
    EUSCI_B2->CTLW0 &= (~UCSWRST);

    /* Turn off all LEDS */
    LP3943_LedModeSet(RED, UNIT_OFF);
    LP3943_LedModeSet(GREEN, UNIT_OFF);
    LP3943_LedModeSet(BLUE, UNIT_OFF);
}

void write_I2C(uint32_t data, uint8_t addr)
{
    EUSCI_B2->CTLW0 |= UCTXSTT;             // generate start and send the slave address
    while(UCTXSTT & EUSCI_B2->CTLW0);    // wait for start condition finished
 //   while(!(UCTXIFG0 & EUSCI_B2->IFG));

    EUSCI_B2->TXBUF = addr;                 // choose address to write out the data
    while(!(UCTXIFG0 & EUSCI_B2->IFG));      // wait until transmitter is ready to accept the data

    for(uint8_t i = 0; i < 4; i ++)
    {
        EUSCI_B2->TXBUF = (data >> 8*i) & 0xFF;     // write out the data
        while(!(UCTXIFG0 & EUSCI_B2->IFG));         // wait until transmitter is ready to accept the data
    }

    EUSCI_B2->CTLW0 |= UCTXSTP;          // generate stop condition
    while(UCTXSTP & EUSCI_B2->CTLW0);    // wait for stop condition finished
}

#endif /* RGBLEDS_C_ */
