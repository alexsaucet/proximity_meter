/*
 * File:   main.c
 * Author: Alexandre
 *
 * Created on October 13, 2016, 5:29 PM
 */

// CONFIG1
#pragma config FOSC = INTOSCIO        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is digital I/O, MCLR internally tied to VDD)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF         // Low-Voltage Programming Enable bit (RB3/PGM pin has PGM function, Low-Voltage Programming enabled)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off)
#pragma config CCPMX = RB0      // CCP1 Pin Selection bit (CCP1 function on RB0)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// CONFIG2
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode disabled)

#include <xc.h>
#include <stdint.h>

#include "dist_measure.h"

#define _XTAL_FREQ  8000000     // System clock frequency

// Delay between 2 distances measures (in ms)
#define DELAY_IDLE      5000    // When no object is close
#define DELAY_ACTIVE    750     // When an object is close

// Proximity levels (higher level = object is closer)
#define METER_LEVEL_1   70
#define METER_LEVEL_2   65
#define METER_LEVEL_3   55
#define METER_LEVEL_4   40

// If distance is the same for this many times, go to idle mode
#define THRESHOLD_COUNT_IDENTICAL   20

void init() {
    // Oscillator config
    OSCCONbits.SCS = 0b00;  // Oscillator mode defined by FOSC<2:0>
    OSCCONbits.IRCF = 0b111; // Internal RC freq to 8 MHz
    OSCCONbits.IOFS = 1;    // Int freq is stable
    
    ANSEL = 0x00;           // Set all I/O to digital I/O
    
    // Ports config
    PORTB = 0x00;       // PortB (0:4) is the proximity meter
    TRISB   = 0x00;

    // Init the distance measurer with 5 samples per measure
    HCInit(5);
}

void update_proximity_meter(uint16_t distance) {
    PORTBbits.RB3 = 0;
    PORTBbits.RB2 = 0;
    PORTBbits.RB1 = 0;
    PORTBbits.RB0 = 0;
    
    if (distance <= METER_LEVEL_4) {
        PORTBbits.RB3 = 1;
    }
    if (distance <= METER_LEVEL_3) {
        PORTBbits.RB2 = 1;
    }
    if (distance <= METER_LEVEL_2) {
        PORTBbits.RB1 = 1;
    }
    if (distance <= METER_LEVEL_1) {
        PORTBbits.RB0 = 1;
    }
}

void main(void) {
    init();
    
    uint8_t count_identical = 0;
    uint16_t distance = 0;
    uint16_t prev_distance = 0;
    
    while(1)
    {
        // Get distance
        distance = HCCalculateDistance();
        
        // Update counter of identical
        if (distance != prev_distance) {
            count_identical = 0;
        } else if (count_identical <= THRESHOLD_COUNT_IDENTICAL) {
            count_identical++;
        }
        prev_distance = distance;
            
        // Update the meter
        if (distance >= METER_LEVEL_1 || count_identical >= THRESHOLD_COUNT_IDENTICAL) {
            update_proximity_meter(METER_LEVEL_1 + 1);  // All LEDs OFF
            __delay_ms(DELAY_IDLE);
        } else {
            update_proximity_meter(distance);
            __delay_ms(DELAY_ACTIVE);
        }
    }
    return;
}
