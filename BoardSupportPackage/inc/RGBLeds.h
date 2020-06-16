#include "msp.h"

#define UNIT_OFF 0x0000
#define LS0 0x06
#define LS1 0x07
#define LS2 0x08
#define LS3 0x09
#define PSC0 0x02
#define PSC1 0x04
#define PWM0 0x03
#define PWM1 0x05
#define LED_AUTO 0x16
#define PWM_AUTO 0x12
#define SLAVE_ADDR 0x60
#define BITRATE 7 //~26 = 3MHz(Clk speed)/115200 Hz(baudrate)

#ifndef RGBLEDS_H_
#define RGBLEDS_H_

/* Enums for RGB LEDs */
typedef enum device
{
    BLUE = 0,
    GREEN = 1,
    RED = 2
} unit_desig;

void LP3943_ColorSet(uint32_t unit, uint32_t LED_MODE, uint32_t PWM_DATA);
void LP3943_LedModeSet(uint32_t unit, uint16_t LED_DATA);
void init_RGBLEDS();
void write_I2C(uint32_t data, uint8_t addr);


#endif /* RGBLEDS_H_ */
