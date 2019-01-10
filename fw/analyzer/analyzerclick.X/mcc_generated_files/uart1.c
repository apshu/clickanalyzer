/**
  UART1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    uart1.c

  @Summary
    This is the generated driver implementation file for the UART1 driver using MPLAB(c) Code Configurator

  @Description
    This header file provides implementations for driver APIs for UART1.
    Generation Information :
        Product Revision  :  MPLAB(c) Code Configurator - 4.15.1
        Device            :  PIC18F25K42
        Driver Version    :  1.00
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

/**
  Section: Included Files
 */
#include <xc.h>
#include "uart1.h"
#include "interrupt_manager.h"
#include "../hwresources.h"

/**
  Section: Macro Declarations
 */
#define UART1_RX_BUFFER_SIZE 32

/**
  Section: Global Variables
 */
static uint8_t uart1RxHead = 0;
static uint8_t uart1RxTail = 0;
static uint8_t uart1RxBuffer[UART1_RX_BUFFER_SIZE];

/**
  Section: UART1 APIs
 */

void UART1_Initialize(void) {
    // Disable interrupts before changing states
    PIE3bits.U1RXIE = 0;
    PIE3bits.U1TXIE = 0;

    // Set the UART1 module to the options selected in the user interface.

    // RXB disabled; 
    //    U1RXB = 0x00;

    // RXCHK disabled; 
    U1RXCHK = 0x00;

    // TXB disabled; 
    //    U1TXB = 0x00;

    // TXCHK disabled; 
    U1TXCHK = 0x00;

    // P1L 0; 
    U1P1L = 0x00;

    // P1H 0; 
    U1P1H = 0x00;

    // P2L 0; 
    U1P2L = 0x00;

    // P2H 0; 
    U1P2H = 0x00;

    // P3L 0; 
    U1P3L = 0x00;

    // P3H 0; 
    U1P3H = 0x00;

    // BRGS high speed; MODE Asynchronous 8-bit mode; RXEN enabled; TXEN enabled; ABDEN disabled; 
    U1CON0 = 0xB0;

    // RXBIMD Set RXBKIF on rising RX input; BRKOVR disabled; WUE disabled; SENDB disabled; ON enabled; 
    U1CON1 = 0x80;

    // TXPOL not inverted; FLO hardware; C0EN Checksum Mode 0; RXPOL not inverted; RUNOVF RX input shifter stops all activity; STP Transmit 1Stop bit, receiver verifies first Stop bit; 
    U1CON2 = 0x02;
#if defined( CIRCUITBOARD ) && ( CIRCUITBOARD > 0 )
#if (CIRCUITBOARD == PCB_clickboard)
    /* 500000 */
    // BRGL 31; 
    U1BRGL = 0x1F;

    // BRGH 0; 
    U1BRGH = 0x00;
#else
#error "Unknown board type"
#endif
#else
#error "Please define circuit board type and/or include hwresources.h"
#endif
    
    // STPMD in middle of first Stop bit; TXWRE No error; 
    U1FIFO = 0x00;

    // ABDIF Auto-baud not enabled or not complete; WUIF WUE not enabled by software; ABDIE disabled; 
    U1UIR = 0x00;

    // ABDOVF Not overflowed; TXCIF 0; RXBKIF No Break detected; RXFOIF not overflowed; CERIF No Checksum error; 
    U1ERRIR = 0x00;

    // TXCIE disabled; FERIE disabled; TXMTIE disabled; ABDOVE disabled; CERIE disabled; RXFOIE disabled; PERIE disabled; RXBKIE disabled; 
    U1ERRIE = 0x00;


    // initializing the driver state
    uart1RxHead = 0;
    uart1RxTail = 0;

    // enable receive interrupt
    PIE3bits.U1RXIE = 1;
}

uint8_t UART1_Read(void) {
    uint8_t readValue = 0;
    if (uart1RxHead != uart1RxTail) {
        readValue = uart1RxBuffer[uart1RxTail++];
        if (sizeof (uart1RxBuffer) <= uart1RxTail) {
            uart1RxTail = 0;
        }
    }
    return readValue;
}

bool UART1_isNewChar(void) {
    return (bool)(uart1RxHead != uart1RxTail);
}

void UART1_Write(uint8_t txData) {
    while (0 == PIR3bits.U1TXIF) {
    }

    U1TXB = txData; // Write the data byte to the USART.
}

void __interrupt(irq(IRQ_U1RX), base(IVT1_BASE_ADDRESS), low_priority) UART1_Receive_ISR() {
    // add your UART1 interrupt custom code
    uart1RxBuffer[uart1RxHead] = U1RXB;
    uint8_t newRxhead = uart1RxHead + 1U;
    if (sizeof (uart1RxBuffer) <= newRxhead) {
        newRxhead = 0;
    }
    if (newRxhead != uart1RxTail) {
        uart1RxHead = newRxhead;
    }
}
/**
  End of File
 */
