/**
  CRC Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    crc.c

  @Summary
    This is the generated driver implementation file for the CRC driver using MPLAB(c) Code Configurator

  @Description
    This source file provides APIs for CRC.
    Generation Information :
        Product Revision  :  MPLAB(c) Code Configurator - 4.15.1
        Device            :  PIC18F25K42
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
#include "crc.h"

/**
 Section: Structures
 */

typedef struct
{
    uint8_t dataWidth;
    uint8_t polyWidth;
    uint8_t seedDirection;
}CRC_OBJ;

/**
 Section: Private Data
 */

static CRC_OBJ crcObj;


/**
  Section: CRCSCAN APIs
*/

void CRC_Initialize(void)
{
    uint16_t poly;
    uint16_t seed;    

    // SHIFTM shift left; CRCGO disabled; CRCEN enabled; ACCM data augmented with 0's; 
    CRCCON0 = 0x90;
    // DLEN 7; PLEN 15; 
    CRCCON1 = (7 << 4) | (15);
    // 
    CRCACCL = 0x00;
    // 
    CRCACCH = 0x00;
    // MREG Program Flash Memory; SGO disabled; TRIGEN disabled; BURSTMD CRC request and Trigger; EN disabled; 
    SCANCON0 = 0x00;
    // LADR 0; 
    SCANLADRU = 0x00;
    // LADR 0; 
    SCANLADRH = 0x00;
    // LADR 0; 
    SCANLADRL = 0x00;
    // HADR 63; 
    SCANHADRU = 0x3F;
    // HADR 255; 
    SCANHADRH = 0xFF;
    // HADR 255; 
    SCANHADRL = 0xFF;
    // TSEL LFINTOSC; 
    SCANTRIG = 0x00;

    poly = 4129;
    CRCXORH = poly >> 8;
    CRCXORL = poly;

    seed  = 65535;
    // CRCACC 65535
    CRCACCH = seed >> 8;
    CRCACCL = seed;

    crcObj.dataWidth = CRCCON1bits.DLEN + 1;
    crcObj.polyWidth = CRCCON1bits.PLEN + 1;
}

void CRC_Start(void)
{
    // Start the serial shifter
    CRCCON0bits.CRCGO = 1;
}

bool CRC_8BitDataWrite(uint8_t data)
{
    if(!CRCCON0bits.FULL)
    {
        CRCDATL = data;
        return(1);
    }
    else
    {
        return(0);
    }
}

static uint16_t CRC_ReverseValue(uint16_t crc)
{
    uint16_t mask;
    uint16_t reverse;

    mask = 1;
    mask <<= crcObj.polyWidth - 1;
    reverse = 0;

    while(crc)
    {
        if(crc & 0x01)
        {
            reverse |= mask;
        }
        mask >>= 1;
        crc >>= 1;
    }
    return reverse;
}

uint16_t CRC_CalculatedResultGet(bool reverse, uint16_t xorValue)
{
    uint16_t result,mask;
     
     // augmentation logic
     CRCCON0bits.CRCGO = 0;
     PIR0bits.CRCIF = 0;
     CRCCON1bits.DLEN= CRCCON1bits.PLEN;
     
     if(crcObj.polyWidth > 8)
     {
         CRCDATH = 0;
         CRCDATL = 0;  
     }
     else
     {
         CRCDATL = 0;
     }
     
     CRCCON0bits.CRCGO = 1;
     while(PIR0bits.CRCIF == 0);
     PIR0bits.CRCIF = 0;
     
     CRCCON0bits.CRCGO = 0;
     CRCCON1bits.DLEN = crcObj.dataWidth - 1;
     // Read result
     result = ((uint16_t)CRCACCH << 8)|CRCACCL;
     if(crcObj.polyWidth < 16)
     {
       // Polynomial mask   
        mask = (1 << crcObj.polyWidth) - 1;
        result &= mask;
     }

    if(reverse)
        result = CRC_ReverseValue(result);

    result ^= xorValue;

    return result;
}

bool CRC_IsBusy(void)
{
    // Is shifting in progress?
    return(CRCCON0bits.BUSY);
}

void CRC_SCAN_StartScanner(void)
{
    // Start the CRC serial shifter
    CRCCON0bits.CRCGO = 1;

    // Start the scanner
    SCANCON0bits.SGO = 1;
}

void CRC_SCAN_StopScanner(void)
{
    // Stop the scanner
    SCANCON0bits.SGO = 0;

    // Stop the CRC serial shifter
    CRCCON0bits.CRCGO = 0;
}

void CRC_SCAN_SetAddressLimit(uint32_t startAddr, uint32_t endAddr)
{
    // Load end address limit
	SCANHADRU = 0x3F & (endAddr >> 16);
	SCANHADRH = 0xFF & (endAddr >> 8);
	SCANHADRL = 0xFF & endAddr;
	
    // Load start address limit
	SCANLADRU = 0x3F & (startAddr >> 16);
	SCANLADRH = 0xFF & (startAddr >> 8);
	SCANLADRL = 0xFF & startAddr;
}	 


bool CRC_SCAN_IsScannerBusy(void)
{
    // Is scanner in progress?
    return(SCANCON0bits.BUSY);
}

bool CRC_SCAN_HasScanCompleted(void)
{
    // Has scanning completed?
    bool status = PIR0bits.SCANIF && PIR0bits.CRCIF;
    if(status)
    {
        PIR0bits.CRCIF = 0;
        PIR0bits.SCANIF = 0;        
    }
    return(status);
}


/**
 End of File
*/
