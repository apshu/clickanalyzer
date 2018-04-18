/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using MPLAB(c) Code Configurator

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB(c) Code Configurator - 4.15.1
        Device            :  PIC18F25K42
        Driver Version    :  2.00
    The generated drivers are tested against the following:
        Compiler          :  XC8 1.35
        MPLAB             :  MPLAB X 3.40
 */

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
 */

#include <stdlib.h>
#include <stdint.h>
#include "mcc_generated_files/mcc.h"
#include "tasks.h"
//#include "cmdline.h"
#include "hwresources.h"

/*
                         Main application
 * UART 57600
 */
static const struct {
    uint32_t generatedsignature;
    uint16_t xFFFF;
} appDataSection __at(0x2000) = {(uint32_t)&appDataSection, 0xFFFF};

void main(void) {
    // Initialize the device
    SYSTEM_Initialize();

    /* Setup Priorities */
    LS_DMAreg(DMA, PR) = 0; /* Logic scope and other scope DMA registers are highest priority */
    ISRPR = 1;
    MAINPR = 2;
    CMD_DMAreg(DMA, PR) = 3;
    SCANPR = 4;

    /* Lock priorities */
    asm("BCF INTCON0,7");
    asm("BANKSEL PRLOCK");
    asm("MOVLW 0x55");
    asm("MOVWF PRLOCK & 0xFF");
    asm("MOVLW 0xAA");
    asm("MOVWF PRLOCK & 0xFF");
    asm("BSF PRLOCK & 0xFF, 0");

    __delay_ms(10);
    /* Enable interrupts */
    INTCON0bits.GIE = 1;
    INTCON0bits.GIEL = 1;
    while (1) {
        TS_task();
    }
}
/**
 End of File
 */