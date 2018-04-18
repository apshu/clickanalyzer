/**
  CWG3 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    cwg3.c

  @Summary
    This is the generated driver implementation file for the CWG3 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs 

  @Description
    This header file provides implementations for driver APIs for CWG3.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs  - 1.45
        Device            :  PIC18F26K42
        Driver Version    :  1.0
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
#include "cwg3.h"

/**
  Section: CWG3 APIs
*/

void CWG3_Initialize(void)
{
    // Set the CWG3 to the options selected in PIC10 / PIC12 / PIC16 / PIC18 MCUs 

    // Writing to CWGxCON0, CWGxCON1, CWGxCON2, CWGxDBR & CWGxDBF registers
	// CWG3POLA non inverted; CWG3POLC non inverted; CWG3POLB non inverted; CWG3POLD non inverted; 
	CWG3CON1 = 0x00;
	// CWG3DBR 0; 
	CWG3DBR = 0x00;
	// CWG3DBF 0; 
	CWG3DBF = 0x00;
	// CWG3SHUTDOWN No Auto-shutdown; CWG3REN disabled; CWG3LSBD tri-stated; CWG3LSAC tri-stated; 
	CWG3AS0 = 0x14;
	// AS1E disabled; AS0E disabled; AS3E disabled; AS2E disabled; AS5E disabled; AS4E disabled; AS6E disabled; 
	CWG3AS1 = 0x00;
	// CWG3CS FOSC; 
	CWG3CLKCON = 0x00;
	// CWG3STRD disabled; CWG3STRB disabled; CWG3STRC disabled; CWG3STRA disabled; CWG3OVRD low; CWG3OVRA low; CWG3OVRB low; CWG3OVRC low; 
	CWG3STR = 0x00;
	// IS CLC1_OUT; 
	CWG3ISM = 0x0F;
    
    // CWG3LD Buffer_not_loaded; CWG3EN enabled; CWG3MODE Half bridge mode; 
    CWG3CON0 = 0x84;
}

void CWG3_LoadRiseDeadbandCount(uint8_t dutyValue)
{
    // Writing 6 bits of rising dead band count into CWGxDBR register
    CWG3DBR = dutyValue;
}

void CWG3_LoadFallDeadbandCount(uint8_t dutyValue)
{
    // Writing 6 bits of rising dead band count into CWGxDBF register
    CWG3DBF = dutyValue;
}

void CWG3_LoadBufferEnable(void)
{
	//It sets the CWG3CON0<LD> bit
    CWG3CON0bits.CWG3LD = 1;
}

bool CWG3_IsModuleEnabled()
{
	//returns whether the module is enabled or not
    return (CWG3CON0bits.CWG3EN);
}

void CWG3_AutoShutdownEventSet()
{
    // Setting the SHUTDOWN bit of CWGxAS0 register
    CWG3AS0bits.CWG3SHUTDOWN = 1;
}

void CWG3_AutoShutdownEventClear()
{
    // Clearing the SHUTDOWN bit of CWGxAS0 register
    CWG3AS0bits.CWG3SHUTDOWN = 0;
}


/**
 End of File
*/