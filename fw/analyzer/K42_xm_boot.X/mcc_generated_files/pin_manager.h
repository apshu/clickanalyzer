/**
  @Generated Pin Manager Header File

  @Company:
    Microchip Technology Inc.

  @File Name:
    pin_manager.h

  @Summary:
    This is the Pin Manager file generated using MPLAB(c) Code Configurator

  @Description:
    This header file provides implementations for pin APIs for all pins selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB(c) Code Configurator - 4.26.1
        Device            :  PIC18F26K42
        Version           :  1.01
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


#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set USBGP0 aliases
#define USBGP0_TRIS               TRISAbits.TRISA0
#define USBGP0_LAT                LATAbits.LATA0
#define USBGP0_PORT               PORTAbits.RA0
#define USBGP0_WPU                WPUAbits.WPUA0
#define USBGP0_OD                ODCONAbits.ODCA0
#define USBGP0_ANS                ANSELAbits.ANSELA0
#define USBGP0_SetHigh()            do { LATAbits.LATA0 = 1; } while(0)
#define USBGP0_SetLow()             do { LATAbits.LATA0 = 0; } while(0)
#define USBGP0_Toggle()             do { LATAbits.LATA0 = ~LATAbits.LATA0; } while(0)
#define USBGP0_GetValue()           PORTAbits.RA0
#define USBGP0_SetDigitalInput()    do { TRISAbits.TRISA0 = 1; } while(0)
#define USBGP0_SetDigitalOutput()   do { TRISAbits.TRISA0 = 0; } while(0)
#define USBGP0_SetPullup()      do { WPUAbits.WPUA0 = 1; } while(0)
#define USBGP0_ResetPullup()    do { WPUAbits.WPUA0 = 0; } while(0)
#define USBGP0_SetPushPull()    do { ODCONAbits.ODCA0 = 1; } while(0)
#define USBGP0_SetOpenDrain()   do { ODCONAbits.ODCA0 = 0; } while(0)
#define USBGP0_SetAnalogMode()  do { ANSELAbits.ANSELA0 = 1; } while(0)
#define USBGP0_SetDigitalMode() do { ANSELAbits.ANSELA0 = 0; } while(0)

// get/set U1RX aliases
#define U1RX_TRIS               TRISAbits.TRISA1
#define U1RX_LAT                LATAbits.LATA1
#define U1RX_PORT               PORTAbits.RA1
#define U1RX_WPU                WPUAbits.WPUA1
#define U1RX_OD                ODCONAbits.ODCA1
#define U1RX_ANS                ANSELAbits.ANSELA1
#define U1RX_SetHigh()            do { LATAbits.LATA1 = 1; } while(0)
#define U1RX_SetLow()             do { LATAbits.LATA1 = 0; } while(0)
#define U1RX_Toggle()             do { LATAbits.LATA1 = ~LATAbits.LATA1; } while(0)
#define U1RX_GetValue()           PORTAbits.RA1
#define U1RX_SetDigitalInput()    do { TRISAbits.TRISA1 = 1; } while(0)
#define U1RX_SetDigitalOutput()   do { TRISAbits.TRISA1 = 0; } while(0)
#define U1RX_SetPullup()      do { WPUAbits.WPUA1 = 1; } while(0)
#define U1RX_ResetPullup()    do { WPUAbits.WPUA1 = 0; } while(0)
#define U1RX_SetPushPull()    do { ODCONAbits.ODCA1 = 1; } while(0)
#define U1RX_SetOpenDrain()   do { ODCONAbits.ODCA1 = 0; } while(0)
#define U1RX_SetAnalogMode()  do { ANSELAbits.ANSELA1 = 1; } while(0)
#define U1RX_SetDigitalMode() do { ANSELAbits.ANSELA1 = 0; } while(0)

// get/set USBGP2 aliases
#define USBGP2_TRIS               TRISAbits.TRISA2
#define USBGP2_LAT                LATAbits.LATA2
#define USBGP2_PORT               PORTAbits.RA2
#define USBGP2_WPU                WPUAbits.WPUA2
#define USBGP2_OD                ODCONAbits.ODCA2
#define USBGP2_ANS                ANSELAbits.ANSELA2
#define USBGP2_SetHigh()            do { LATAbits.LATA2 = 1; } while(0)
#define USBGP2_SetLow()             do { LATAbits.LATA2 = 0; } while(0)
#define USBGP2_Toggle()             do { LATAbits.LATA2 = ~LATAbits.LATA2; } while(0)
#define USBGP2_GetValue()           PORTAbits.RA2
#define USBGP2_SetDigitalInput()    do { TRISAbits.TRISA2 = 1; } while(0)
#define USBGP2_SetDigitalOutput()   do { TRISAbits.TRISA2 = 0; } while(0)
#define USBGP2_SetPullup()      do { WPUAbits.WPUA2 = 1; } while(0)
#define USBGP2_ResetPullup()    do { WPUAbits.WPUA2 = 0; } while(0)
#define USBGP2_SetPushPull()    do { ODCONAbits.ODCA2 = 1; } while(0)
#define USBGP2_SetOpenDrain()   do { ODCONAbits.ODCA2 = 0; } while(0)
#define USBGP2_SetAnalogMode()  do { ANSELAbits.ANSELA2 = 1; } while(0)
#define USBGP2_SetDigitalMode() do { ANSELAbits.ANSELA2 = 0; } while(0)

// get/set RA3 procedures
#define RA3_SetHigh()    do { LATAbits.LATA3 = 1; } while(0)
#define RA3_SetLow()   do { LATAbits.LATA3 = 0; } while(0)
#define RA3_Toggle()   do { LATAbits.LATA3 = ~LATAbits.LATA3; } while(0)
#define RA3_GetValue()         PORTAbits.RA3
#define RA3_SetDigitalInput()   do { TRISAbits.TRISA3 = 1; } while(0)
#define RA3_SetDigitalOutput()  do { TRISAbits.TRISA3 = 0; } while(0)
#define RA3_SetPullup()     do { WPUAbits.WPUA3 = 1; } while(0)
#define RA3_ResetPullup()   do { WPUAbits.WPUA3 = 0; } while(0)
#define RA3_SetAnalogMode() do { ANSELAbits.ANSELA3 = 1; } while(0)
#define RA3_SetDigitalMode()do { ANSELAbits.ANSELA3 = 0; } while(0)

// get/set RA4 procedures
#define RA4_SetHigh()    do { LATAbits.LATA4 = 1; } while(0)
#define RA4_SetLow()   do { LATAbits.LATA4 = 0; } while(0)
#define RA4_Toggle()   do { LATAbits.LATA4 = ~LATAbits.LATA4; } while(0)
#define RA4_GetValue()         PORTAbits.RA4
#define RA4_SetDigitalInput()   do { TRISAbits.TRISA4 = 1; } while(0)
#define RA4_SetDigitalOutput()  do { TRISAbits.TRISA4 = 0; } while(0)
#define RA4_SetPullup()     do { WPUAbits.WPUA4 = 1; } while(0)
#define RA4_ResetPullup()   do { WPUAbits.WPUA4 = 0; } while(0)
#define RA4_SetAnalogMode() do { ANSELAbits.ANSELA4 = 1; } while(0)
#define RA4_SetDigitalMode()do { ANSELAbits.ANSELA4 = 0; } while(0)

// get/set RA5 procedures
#define RA5_SetHigh()    do { LATAbits.LATA5 = 1; } while(0)
#define RA5_SetLow()   do { LATAbits.LATA5 = 0; } while(0)
#define RA5_Toggle()   do { LATAbits.LATA5 = ~LATAbits.LATA5; } while(0)
#define RA5_GetValue()         PORTAbits.RA5
#define RA5_SetDigitalInput()   do { TRISAbits.TRISA5 = 1; } while(0)
#define RA5_SetDigitalOutput()  do { TRISAbits.TRISA5 = 0; } while(0)
#define RA5_SetPullup()     do { WPUAbits.WPUA5 = 1; } while(0)
#define RA5_ResetPullup()   do { WPUAbits.WPUA5 = 0; } while(0)
#define RA5_SetAnalogMode() do { ANSELAbits.ANSELA5 = 1; } while(0)
#define RA5_SetDigitalMode()do { ANSELAbits.ANSELA5 = 0; } while(0)

// get/set RA6 procedures
#define RA6_SetHigh()    do { LATAbits.LATA6 = 1; } while(0)
#define RA6_SetLow()   do { LATAbits.LATA6 = 0; } while(0)
#define RA6_Toggle()   do { LATAbits.LATA6 = ~LATAbits.LATA6; } while(0)
#define RA6_GetValue()         PORTAbits.RA6
#define RA6_SetDigitalInput()   do { TRISAbits.TRISA6 = 1; } while(0)
#define RA6_SetDigitalOutput()  do { TRISAbits.TRISA6 = 0; } while(0)
#define RA6_SetPullup()     do { WPUAbits.WPUA6 = 1; } while(0)
#define RA6_ResetPullup()   do { WPUAbits.WPUA6 = 0; } while(0)
#define RA6_SetAnalogMode() do { ANSELAbits.ANSELA6 = 1; } while(0)
#define RA6_SetDigitalMode()do { ANSELAbits.ANSELA6 = 0; } while(0)

// get/set PIN_INHIBIT_BOOT aliases
#define PIN_INHIBIT_BOOT_TRIS               TRISAbits.TRISA7
#define PIN_INHIBIT_BOOT_LAT                LATAbits.LATA7
#define PIN_INHIBIT_BOOT_PORT               PORTAbits.RA7
#define PIN_INHIBIT_BOOT_WPU                WPUAbits.WPUA7
#define PIN_INHIBIT_BOOT_OD                ODCONAbits.ODCA7
#define PIN_INHIBIT_BOOT_ANS                ANSELAbits.ANSELA7
#define PIN_INHIBIT_BOOT_SetHigh()            do { LATAbits.LATA7 = 1; } while(0)
#define PIN_INHIBIT_BOOT_SetLow()             do { LATAbits.LATA7 = 0; } while(0)
#define PIN_INHIBIT_BOOT_Toggle()             do { LATAbits.LATA7 = ~LATAbits.LATA7; } while(0)
#define PIN_INHIBIT_BOOT_GetValue()           PORTAbits.RA7
#define PIN_INHIBIT_BOOT_SetDigitalInput()    do { TRISAbits.TRISA7 = 1; } while(0)
#define PIN_INHIBIT_BOOT_SetDigitalOutput()   do { TRISAbits.TRISA7 = 0; } while(0)
#define PIN_INHIBIT_BOOT_SetPullup()      do { WPUAbits.WPUA7 = 1; } while(0)
#define PIN_INHIBIT_BOOT_ResetPullup()    do { WPUAbits.WPUA7 = 0; } while(0)
#define PIN_INHIBIT_BOOT_SetPushPull()    do { ODCONAbits.ODCA7 = 1; } while(0)
#define PIN_INHIBIT_BOOT_SetOpenDrain()   do { ODCONAbits.ODCA7 = 0; } while(0)
#define PIN_INHIBIT_BOOT_SetAnalogMode()  do { ANSELAbits.ANSELA7 = 1; } while(0)
#define PIN_INHIBIT_BOOT_SetDigitalMode() do { ANSELAbits.ANSELA7 = 0; } while(0)

// get/set RB4 procedures
#define RB4_SetHigh()    do { LATBbits.LATB4 = 1; } while(0)
#define RB4_SetLow()   do { LATBbits.LATB4 = 0; } while(0)
#define RB4_Toggle()   do { LATBbits.LATB4 = ~LATBbits.LATB4; } while(0)
#define RB4_GetValue()         PORTBbits.RB4
#define RB4_SetDigitalInput()   do { TRISBbits.TRISB4 = 1; } while(0)
#define RB4_SetDigitalOutput()  do { TRISBbits.TRISB4 = 0; } while(0)
#define RB4_SetPullup()     do { WPUBbits.WPUB4 = 1; } while(0)
#define RB4_ResetPullup()   do { WPUBbits.WPUB4 = 0; } while(0)
#define RB4_SetAnalogMode() do { ANSELBbits.ANSELB4 = 1; } while(0)
#define RB4_SetDigitalMode()do { ANSELBbits.ANSELB4 = 0; } while(0)

// get/set RB5 procedures
#define RB5_SetHigh()    do { LATBbits.LATB5 = 1; } while(0)
#define RB5_SetLow()   do { LATBbits.LATB5 = 0; } while(0)
#define RB5_Toggle()   do { LATBbits.LATB5 = ~LATBbits.LATB5; } while(0)
#define RB5_GetValue()         PORTBbits.RB5
#define RB5_SetDigitalInput()   do { TRISBbits.TRISB5 = 1; } while(0)
#define RB5_SetDigitalOutput()  do { TRISBbits.TRISB5 = 0; } while(0)
#define RB5_SetPullup()     do { WPUBbits.WPUB5 = 1; } while(0)
#define RB5_ResetPullup()   do { WPUBbits.WPUB5 = 0; } while(0)
#define RB5_SetAnalogMode() do { ANSELBbits.ANSELB5 = 1; } while(0)
#define RB5_SetDigitalMode()do { ANSELBbits.ANSELB5 = 0; } while(0)

// get/set RC0 procedures
#define RC0_SetHigh()    do { LATCbits.LATC0 = 1; } while(0)
#define RC0_SetLow()   do { LATCbits.LATC0 = 0; } while(0)
#define RC0_Toggle()   do { LATCbits.LATC0 = ~LATCbits.LATC0; } while(0)
#define RC0_GetValue()         PORTCbits.RC0
#define RC0_SetDigitalInput()   do { TRISCbits.TRISC0 = 1; } while(0)
#define RC0_SetDigitalOutput()  do { TRISCbits.TRISC0 = 0; } while(0)
#define RC0_SetPullup()     do { WPUCbits.WPUC0 = 1; } while(0)
#define RC0_ResetPullup()   do { WPUCbits.WPUC0 = 0; } while(0)
#define RC0_SetAnalogMode() do { ANSELCbits.ANSELC0 = 1; } while(0)
#define RC0_SetDigitalMode()do { ANSELCbits.ANSELC0 = 0; } while(0)

// get/set RC1 procedures
#define RC1_SetHigh()    do { LATCbits.LATC1 = 1; } while(0)
#define RC1_SetLow()   do { LATCbits.LATC1 = 0; } while(0)
#define RC1_Toggle()   do { LATCbits.LATC1 = ~LATCbits.LATC1; } while(0)
#define RC1_GetValue()         PORTCbits.RC1
#define RC1_SetDigitalInput()   do { TRISCbits.TRISC1 = 1; } while(0)
#define RC1_SetDigitalOutput()  do { TRISCbits.TRISC1 = 0; } while(0)
#define RC1_SetPullup()     do { WPUCbits.WPUC1 = 1; } while(0)
#define RC1_ResetPullup()   do { WPUCbits.WPUC1 = 0; } while(0)
#define RC1_SetAnalogMode() do { ANSELCbits.ANSELC1 = 1; } while(0)
#define RC1_SetDigitalMode()do { ANSELCbits.ANSELC1 = 0; } while(0)

/**
   @Param
    none
   @Returns
    none
   @Description
    GPIO and peripheral I/O initialization
   @Example
    PIN_MANAGER_Initialize();
 */
void PIN_MANAGER_Initialize (void);

/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handling routine
 * @Example
    PIN_MANAGER_IOC();
 */
void PIN_MANAGER_IOC(void);



#endif // PIN_MANAGER_H
/**
 End of File
*/