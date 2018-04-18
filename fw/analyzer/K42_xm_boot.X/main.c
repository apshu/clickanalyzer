/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs 

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs  - 1.45
        Device            :  PIC18F26K42
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
#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <setjmp.h>
#include "mcc_generated_files/mcc.h"
#include "flash_helper.h"
#include "xmodem.h"
#include "SMTautotune.h"
#include "printf_helper.h"
#include "mcc_generated_files/tmr0.h"
#include "mcc_generated_files/clc1.h"
#include "mcc_generated_files/cwg3.h"

/* WHAT and how 
 The bootloader takes a binary file sent by XMODEM and programs it to app space.
 Full app space is erased after first valid frame received
 Jumping to address 0 starts the bootloader. If UART was configured, no pin/UART configuration is changed. (PPS, BAUD, TRIS, etc. preserved)
    This allow app custom UART/BAUD settings.
 If boot requested with uninitialized UART(e.g. after reset) the bootloader defaults to RB4/RB5 TX/RX 9600 BAUD
 Boot can be requested by pulling down FORCE BOOT PIN (RA0)
 Boot can be delayed by pulling low INHIBIT BOOT pin (RA1). 
    The pin is sampled at approx 1kHz.
    This pin has to be held low for minimum 5ms latest 45ms after reset to take effect. 
    The inhibited boot procedure resumes after pin is high for at least 5ms.
 */

flashStruct_t flashWr;
static bit isAppErased = 0;
static bit retried = 0;
static uint32_t appCodeEndBytes;

static const struct {
    uint8_t crc16L;
    uint8_t xF5;
    uint8_t crc16H;
    uint8_t xFA;
    uint8_t codeEndRowsL;
    uint8_t codeEndRowsH : 4;
    uint8_t xF : 4;
} *appDataSection;

static void addCRCbyte(uint8_t dataByte) {
    CRCCON0bits.CRCGO = 1;
    while (CRCCON0bits.FULL) {
        continue;
    }
    CRCDATL = dataByte;
}

/* return true if CRC matches */
static bit checkAppCRC(void) {
    CRCACCH = CRCACCL = 0xFF;
    // SHIFTM shift left; CRCGO disabled; CRCEN enabled; ACCM data augmented with 0's; 
    //        CRCCON0 = 0x90;
    CRCCON0 = 0x90;
    // DLEN 7; PLEN 15; 
    CRCCON1 = (7 << 4) | (15);
    CRCXORH = 0x10;
    CRCXORL = 0x21;

    union {
        uint32_t asui32;

        struct {
            uint8_t lwlb;
            uint8_t lwhb;
            uint8_t hwlb;
            uint8_t hwhb;
        };
    } appAddr;
    appAddr.asui32 = (uint32_t) appDataSection;
    addCRCbyte(appAddr.lwlb);
    addCRCbyte(appAddr.lwhb);
    addCRCbyte(appAddr.hwlb);
    addCRCbyte(appAddr.hwhb);
    CRCCON0bits.CRCGO = 1;
    TBLPTR = appAddr.asui32 + 4UL; /* This stores the boot start address */
    appAddr.asui32 += appCodeEndBytes;
    while (TBLPTR != appAddr.asui32) {
        if (!CRCCON0bits.CRCGO) {
            CRCCON0bits.CRCGO = 1;
        }
        if (!CRCCON0bits.FULL) {
            asm("TBLRDPOSTINC");
            CRCDATL = TABLAT;
        }
    }
    while (CRCCON0bits.BUSY) {
        continue;
    }
    CRCCON0 = 0;
    return ((CRCACCL == appDataSection->crc16L) && (CRCACCH == appDataSection->crc16H));
}

void inline restoreMCUstate(void) {
    GIE = TMR6ON = TMR6IE = 0;
}

void inline saveMCUstate(void) {
    /* Disable all interrupts */
    INTCON0 = PIE0 = PIE1 = PIE2 = PIE3 = PIE4 = PIE5 = PIE6 = PIE7 = PIE8 = PIE9 = PIE10 = 0;
    /* Disable vectored interrupts */
    GIE = 0;
    IVTLOCK = 0x55;
    IVTLOCK = 0xAA;
    IVTLOCKbits.IVTLOCKED = 0x00; // unlock IVT    
}

void inline INTOSC_autotune() {
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x00; // unlock PPS
    ANSELA7 = 0;
    CLCIN0PPSbits.CLCIN0PPS = 0x07; //RA7->CLC4:CLCIN0;
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x01; // lock PPS
    CLC4_Initialize();
    SMTTUN_tune();
}

bit isForcedBoot = 0;

void main(void) {
    saveMCUstate();
    /* configure pull-ups */
    LATA = 0;
    TRISA = 0x87;
    ANSELA = 0; /* PORTA is never analog input */
    WPUA = 0x86; /* Status line pull-ups */
    WPUB4 = WPUB5 = 1; /* USART pull-ups */
    OSCILLATOR_Initialize();
    __delay_ms(1); /* Wait for commchip stabilization */
    while (PIN_INHIBIT_BOOT_GetValue()) continue; /* Inhibit boot so log as CLKREF pin is held high */
    TMR6_Initialize(); /* start CLKREF watchdog that resets if steady state for >50ms */
    GIE = 1;
    TMR4_Initialize(); /* Overflows after 10ms */
    while (!PIN_INHIBIT_BOOT_GetValue()) {
        continue;
    } /* Loop while clock line held low */
    isForcedBoot = TMR4IF;
    /* CLKREF now stabilized, start autotuning */
    INTOSC_autotune();
    FH_initFlashStruct(&flashWr);
    appDataSection = (const void*) FH_getBootEndAddress();
    static bit bootRequest = false;
    appCodeEndBytes = (uint32_t) ((appDataSection->codeEndRowsH << 8U) + appDataSection->codeEndRowsL) * 128UL;
    if (U1CON1bits.U1ON != 0) {
        strOut(3);
        bootRequest = true;
    }
    if ((appDataSection->xF5 != 0xF5) || (appDataSection->xFA != 0xFA) || (appDataSection->xF != 0xF) || (!appCodeEndBytes)) {
        strOut(0);
        bootRequest = true;
    };
    if (appCodeEndBytes > FH_getAppEndAddress()) {
        appCodeEndBytes = FH_getAppEndAddress();
    }
    if (!checkAppCRC()) {
        strOut(1);
        bootRequest = true;
    }
    if (isForcedBoot) {
        strOut(2);
        bootRequest = true;
    }
    if (bootRequest) {
        /* TODO: In boot mode LED animation start */
        TMR0_Initialize(); /* Boot animation timer */
        CWG3_Initialize();
        CLC1_Initialize();
        strOut(4);
        TMR2_Initialize();

        XM_resetState();
        while (XM_getNextChunk()) {
            int16_t bytesGot;
            if ((flashWr.dataBuffer = XM_getData(&bytesGot))) {
                if (!isAppErased) {
                    isAppErased = FH_eraseAppSpace();
                    if (!isAppErased) {
                        XM_abortTransfer();
                        break;
                    }
                }
                while (bytesGot > 0) {
                    if (FH_writeVerifyFlash(true, &flashWr)) {
                        /* Write and verify successful */
                        flashWr.dataBuffer += flashWr.flashWriteBytes;
                        flashWr.flashAddress += flashWr.flashWriteBytes;
                        bytesGot -= flashWr.flashWriteBytes;
                        retried = false;
                    } else {
                        if (retried) {
                            XM_abortTransfer();
                            break;
                        }
                        retried = true;
                    }
                }
            }
        }
        while (1) {
            __delay_ms(500);
            Reset();
        }
    }
    restoreMCUstate();
    FH_jumpToApp(sizeof (*appDataSection));
}

/* Place a reset instruction at addres 8 (HIGHISR) */
asm("\tpsect reset_vec");
asm("\torg 4");
asm("\treset");
/**
 End of File
 */