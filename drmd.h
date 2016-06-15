#ifndef DRMD_H_
#define DRMD_H_

#include <bcm2835.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Use with bcm2835_gpio_fsel(PIN, INPUT/OUTPUT)
#define INPUT     BCM2835_GPIO_FSEL_INPT // Pin input mode
#define OUTPUT    BCM2835_GPIO_FSEL_OUTP // Pin output mode

// Use with bcm2835_gpio_set_pud(PIN, PULLDOWN/PULLUP/NOPULL)
#define PULLDOWN  BCM2835_GPIO_PUD_DOWN // Input with a pulldown resistor
#define PULLUP    BCM2835_GPIO_PUD_UP   // Input with a pullup resistor
#define NOPULL    BCM2835_GPIO_PUD_OFF  // Input with no pulldown or pullup resistors

// GPIO pin mappings to the bcm2835 for rpi b+ and rpi 2 40 pin headers
#define GPIO2     RPI_BPLUS_GPIO_J8_03
#define GPIO3     RPI_BPLUS_GPIO_J8_05
#define GPIO4     RPI_BPLUS_GPIO_J8_07
#define GPIO5     RPI_BPLUS_GPIO_J8_29
#define GPIO6     RPI_BPLUS_GPIO_J8_31
#define GPIO7     RPI_BPLUS_GPIO_J8_26
#define GPIO8     RPI_BPLUS_GPIO_J8_24
#define GPIO9     RPI_BPLUS_GPIO_J8_21
#define GPIO10    RPI_BPLUS_GPIO_J8_19
#define GPIO11    RPI_BPLUS_GPIO_J8_23
#define GPIO12    RPI_BPLUS_GPIO_J8_32
#define GPIO13    RPI_BPLUS_GPIO_J8_33
#define GPIO14    RPI_BPLUS_GPIO_J8_08
#define GPIO15    RPI_BPLUS_GPIO_J8_10
#define GPIO16    RPI_BPLUS_GPIO_J8_36
#define GPIO17    RPI_BPLUS_GPIO_J8_11
#define GPIO18    RPI_BPLUS_GPIO_J8_12
#define GPIO19    RPI_BPLUS_GPIO_J8_35
#define GPIO20    RPI_BPLUS_GPIO_J8_38
#define GPIO21    RPI_BPLUS_GPIO_J8_40
#define GPIO22    RPI_BPLUS_GPIO_J8_15
#define GPIO23    RPI_BPLUS_GPIO_J8_16
#define GPIO24    RPI_BPLUS_GPIO_J8_18
#define GPIO25    RPI_BPLUS_GPIO_J8_22
#define GPIO26    RPI_BPLUS_GPIO_J8_37
#define GPIO27    RPI_BPLUS_GPIO_J8_13

#define SDA1      GPIO2
#define SCL1      GPIO3

#define PDN1      GPIO4       // Absorbance LED
#define PDN2      GPIO17      // Flourescence LED
#define PDN3      GPIO21      // Peristaltic Pump

/*  DRV8825 PINS:
      PD      Input with internal PullDown resistor
      PU      Input with internal PullUp resistor
      PDPU    Input with internal PullDown and PullUp resistors
      O       Output
      OD      Output with an Open Drain
*/
#define nENBL     GPIO6       // PD     0, open = enable; 1 = disable
#define nSLEEP    GPIO20      // PD     0, open = sleep, 1 = enable
#define nRESET    GPIO26      // PD     0, open = reset internal logic
#define DECAY     GPIO16      // PDPU   0 = slow, open = mixed, 1 = fast
#define STEP      GPIO12      // PD     rising edge steps motor
#define DIR       GPIO13      // PD     0 = forward, 1 = reverse

#define nHOME     GPIO11      // OD     0 when at home state, open otherwise
#define nFAULT    GPIO19      // OD     0 when overtemp/overcurrent, open otherwise

/*  [MODE2, MODE1, MODE0] determines the degree of microstepping
      000       Full step
      001       1/2 step
      010       1/4 step
      011       1/8 step
      100       1/16 step
      101-111   1/32 step
*/
#define MODE0     GPIO5       // PD
#define MODE1     GPIO7       // PD
#define MODE2     GPIO8       // PD

#define STEPS_PER_REV   200   // Number of steps for a single stepper motor revolution

#endif