/**
  Generated Pin Manager File

  Company:
    Microchip Technology Inc.

  File Name:
    pin_manager.c

  Summary:
    This is the Pin Manager file generated using MPLAB(c) Code Configurator

  Description:
    This header file provides implementations for pin APIs for all pins selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB(c) Code Configurator - 4.15.1
        Device            :  PIC18F25K42
        Driver Version    :  1.02
    The generated drivers are tested against the following:
        Compiler          :  XC8 1.35
        MPLAB             :  MPLAB X 3.40

    Copyright (c) 2013 - 2015 released Microchip Technology Inc.  All rights reserved.

    Microchip licenses to you the right to use, modify, copy and distribute
    Software only when embedded on a Microchip microcontroller or digital signal
    controller that is integrated into your product or third party product
    (pursuant to the sublicense terms in the accompanying license agreement).

    You should refer to the license agreement accompanying this Software for
    additional information regarding your rights and obligations.

    SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
    EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
    MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
    IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
    CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
    OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
    INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
    CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
    SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
    (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

 */

#include <xc.h>
#include "pin_manager.h"
#include "stdbool.h"
#include "../hwresources.h"

#pragma warning disable 520     /* disable warning: function "xxxx" is never called */
#pragma warning disable 1090    /* disable warning: variable "xxxx" is not used */
#pragma warning disable 759     /* disable warning: expression generates no code */

void PIN_MANAGER_Initialize(void) {
    /**
    LATx registers
     */
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;

    /**
    TRISx registers
     */
    TRISA = 0xE5;
    TRISC = 0xFF;

    /**
    ANSELx registers
     */
    ANSELC = 0x00;
    ANSELB = 0x00;
    ANSELA = 0x00;

    /**
    WPUx registers
     */
    WPUE = 0x08;
    WPUA = 0xE0;
    WPUC = 0x00;

    /**
    ODx registers
     */
    ODCONA = 0x00;
    ODCONB = 0x00;
    ODCONC = 0x00;


    /**
    IOCx registers
     */
    IOCBF = IOCCF = 0;
    IOCBN = IOCBP = IOCCN = IOCCP = 0xFF;

    bool state = GIE;
    GIE = 0;
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x00; // unlock PPS

    T2INPPSbits.T2INPPS = 0x04; //RA4->TMR2:T2IN;
#if defined( CIRCUITBOARD ) && ( CIRCUITBOARD > 0 )
#if (CIRCUITBOARD == PCB_gumstick)
    TRISB = 0xBF;
    WPUB = 0x40;
    U1RXPPSbits.U1RXPPS = 0x0F; //RB7->UART1:RX1;
    RB6PPS = 0x13; //RB6->UART1:TX1;
#elif (CIRCUITBOARD == PCB_gumstick_alt_UART) || (CIRCUITBOARD == PCB_clickboard)
    TRISB = 0xFF;
    WPUB = 0x30;
    U1RXPPSbits.U1RXPPS = 0x0C;   //RB4->UART1:RX1;
    U1CTSPPSbits.U1CTSPPS = 0x0D;   //RB5->UART1:CTS1;
    T2INPPSbits.T2INPPS = 0x07;   //RA7->TMR2:T2IN;
    RA1PPS = 0x02; //CLC2:CLC2OUT->RA1
#else
#error "Unknown board type"
#endif
#else
#error "Please define circuit board type and/or include hwresources.h"
#endif

    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x01; // lock PPS

    GIE = state;
}

/**
 End of File
 */
