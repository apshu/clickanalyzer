#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "flash_helper.h"
/*
 * Device Information Area (DIA) Table
 */
#if DCI_EESIZ != 0x3FFF06
//Compiler errata workaround
#undef DIA_MUI   
#undef DIA_MUI0  
#undef DIA_MUI1  
#undef DIA_MUI2  
#undef DIA_MUI3  
#undef DIA_MUI4  
#undef DIA_MUI5  
#undef DIA_MUI6  
#undef DIA_MUI7  
#undef DIA_EUI   
#undef DIA_EUI0  
#undef DIA_EUI1  
#undef DIA_EUI2  
#undef DIA_EUI3  
#undef DIA_EUI4  
#undef DIA_EUI5  
#undef DIA_EUI6  
#undef DIA_EUI7  
#undef DIA_EUI8  
#undef DIA_EUI9  
#undef DIA_TSLR  
#undef DIA_TSLR1 
#undef DIA_TSLR2 
#undef DIA_TSLR3 
#undef DIA_TSHR  
#undef DIA_TSHR1 
#undef DIA_TSHR2 
#undef DIA_TSHR3 
#undef DIA_FVRA  
#undef DIA_FVRA1X
#undef DIA_FVRA2X
#undef DIA_FVRA4X
#undef DIA_FVRC  
#undef DIA_FVRC1X
#undef DIA_FVRC2X
#undef DIA_FVRC4X
#undef DCI_ERSIZ 
#undef DCI_WLSIZ 
#undef DCI_URSIZ 
#undef DCI_EESIZ 
#undef DCI_PCNT  
#define DIA_MUI                                             0x3F0000
#define DIA_MUI0                                            0x3F0000
#define DIA_MUI1                                            0x3F0002
#define DIA_MUI2                                            0x3F0004
#define DIA_MUI3                                            0x3F0006
#define DIA_MUI4                                            0x3F0008
#define DIA_MUI5                                            0x3F000A
#define DIA_MUI6                                            0x3F000C
#define DIA_MUI7                                            0x3F000E
#define DIA_EUI                                             0x3F0010
#define DIA_EUI0                                            0x3F0010
#define DIA_EUI1                                            0x3F0012
#define DIA_EUI2                                            0x3F0014
#define DIA_EUI3                                            0x3F0016
#define DIA_EUI4                                            0x3F0018
#define DIA_EUI5                                            0x3F001A
#define DIA_EUI6                                            0x3F001C
#define DIA_EUI7                                            0x3F001E
#define DIA_EUI8                                            0x3F0020
#define DIA_EUI9                                            0x3F0022
#define DIA_TSLR                                            0x3F0024
#define DIA_TSLR1                                           0x3F0024
#define DIA_TSLR2                                           0x3F0026
#define DIA_TSLR3                                           0x3F0028
#define DIA_TSHR                                            0x3F002A
#define DIA_TSHR1                                           0x3F002A
#define DIA_TSHR2                                           0x3F002C
#define DIA_TSHR3                                           0x3F002E
#define DIA_FVRA                                            0x3F0030
#define DIA_FVRA1X                                          0x3F0030
#define DIA_FVRA2X                                          0x3F0032
#define DIA_FVRA4X                                          0x3F0034
#define DIA_FVRC                                            0x3F0036
#define DIA_FVRC1X                                          0x3F0036
#define DIA_FVRC2X                                          0x3F0038
#define DIA_FVRC4X                                          0x3F003A
#define DCI_ERSIZ                                           0x3FFF00
#define DCI_WLSIZ                                           0x3FFF02
#define DCI_URSIZ                                           0x3FFF04
#define DCI_EESIZ                                           0x3FFF06
#define DCI_PCNT                                            0x3FFF08
#endif

#define DEVID_DEVID 0x3FFFFEUL
#define DEVID_REVID 0x3FFFFCUL
#define CFG_CFG4    0x300006UL
#define SAFbytes 128U

extern const uint16_t DCI_eraseRowWords __at(DCI_ERSIZ);
extern const uint16_t DCI_writeRowBytes __at(DCI_WLSIZ);
extern const uint16_t DCI_userRows __at(DCI_URSIZ);
extern const uint16_t DCI_EEPROMSizeBytes __at(DCI_EESIZ);
extern const uint16_t DCI_PinCount __at(DCI_PCNT);
extern const uint8_t DIA_MicrochipEUI[16] __at(DIA_MUI);
extern const uint16_t DEVID_deviceID __at(DEVID_DEVID);

extern const struct {
    uint16_t MNRREV : 6;
    uint16_t MJRREV : 6;
    uint16_t fixVal : 4;
} DEVID_revisionID __at(DEVID_REVID);

extern const struct {
    uint16_t BBSIZE : 3;
    uint16_t nBBEN : 1;
    uint16_t nSAFEN : 1;
uint16_t:
    2;
    uint16_t nWRTAPP : 1;
    uint16_t nWRTB : 1;
    uint16_t nWRTC : 1;
    uint16_t nWRTD : 1;
    uint16_t nWRTSAF : 1;
uint16_t:
    1;
    uint16_t LVP : 1;
uint16_t:
    2;
} CFG_4bits __at(CFG_CFG4);

static uint8_t inline FH_getStorageFLASHwriteableBytes(void) {
    return (CFG_4bits.nSAFEN || CFG_4bits.nWRTSAF) ? 0U : SAFbytes;
}

static uint32_t inline FH_getFLASHWrapAddress(void) {
    return ((uint32_t) DCI_eraseRowWords * DCI_userRows) << 1UL;
}

/* Returns the first flash word address NOT belonging to BOOT */
uint32_t FH_getBootEndAddress(void) {
    uint16_t bbReqBytes;
    if (!CFG_4bits.nBBEN) {
        bbReqBytes = 131072UL >> CFG_4bits.BBSIZE;
        uint16_t memHalf = FH_getFLASHWrapAddress() >> 1;
        if (bbReqBytes > memHalf) {
            bbReqBytes = memHalf;
        }
    } else {
        /* Boot block disabled, estimate something */
        bbReqBytes = DCI_eraseRowWords;
        asm("rlcf FH_getBootEndAddress@bbReqBytes, c");
        asm("rlcf FH_getBootEndAddress@bbReqBytes+1, c");
        --bbReqBytes;
        asm("movlw LOW(start)");
        asm("iorwf FH_getBootEndAddress@bbReqBytes, c");
        asm("movlw HIGH(start)");
        asm("iorwf FH_getBootEndAddress@bbReqBytes+1, c");
        ++bbReqBytes;
    }
    return bbReqBytes;
}

/* Returns the first flash word address NOT belonging to APP */
uint32_t FH_getAppEndAddress(void) {
    return FH_getFLASHWrapAddress() - ((CFG_4bits.nSAFEN) ? 0U : SAFbytes);
}

/* Return true if success */
static bit FH_eraseCurrentFLASHRow(void) {
    NVMCON1bits.FREE = 1; //write operation means erase
    NVMCON2 = 0x55; //Unlock
    NVMCON2 = 0xAA; //Unlock
    NVMCON1bits.WR = 1; //Write
    return !(NVMCON1bits.WRERR);
}

/* Return true if erase was successful */
bit FH_eraseAppSpace(void) {
    if (CFG_4bits.nWRTAPP) {
        char GIEsave = INTCON0bits.GIE;
        INTCON0bits.GIE = 0; // Disable interrupts
        uint32_t endAddress = FH_getAppEndAddress();
        uint8_t eraseRowBytes = DCI_eraseRowWords << 1;
        NVMCON1 = 0x84; /* operate on FLASH and enable writes */
        for (TBLPTR = FH_getBootEndAddress(); TBLPTR < endAddress; TBLPTR += eraseRowBytes) {
            if (!FH_eraseCurrentFLASHRow()) {
                break;
            }
        }
        NVMCON1 &= _NVMCON1_WRERR_MASK;
        GIE = GIEsave;
        return !(NVMCON1bits.WRERR);
    }
    return false;
}
#pragma regsused FH_eraseAppSpace TBLPTRL, TBLPTRH, TBLPTRU, TABLAT

bit FH_eraseStorageFLASHSpace() {
    if (FH_getStorageFLASHwriteableBytes()) {
        char GIEsave = INTCON0bits.GIE;
        INTCON0bits.GIE = 0; // Disable interrupts
        TBLPTR = FH_getAppEndAddress();
        NVMCON1 = 0x84; /* operate on FLASH and enable writes */
        FH_eraseCurrentFLASHRow();
        NVMCON1 &= _NVMCON1_WRERR_MASK;
        GIE = GIEsave;
        return !(NVMCON1bits.WRERR);
    }
    return 0;
}
#pragma regsused FH_eraseStorageFLASHSpace TBLPTRL, TBLPTRH, TBLPTRU

void FH_initFlashStruct(flashStruct_t *data) {
    data->dataBuffer = NULL;
    data->flashAddress = FH_getBootEndAddress();
    data->flashWriteBytes = DCI_writeRowBytes;
}

bit FH_writeVerifyFlash(bool enableErase, flashStruct_t *wrData) {
    uint8_t flashBuf[256];
    uint8_t flashUnitBytes = DCI_eraseRowWords << 1U;
    uint32_t address = wrData->flashAddress;
    uint8_t *buffer = wrData->dataBuffer;
    if (((address & ((DCI_eraseRowWords << 1) - 1)) == 0) && (wrData->flashWriteBytes <= flashUnitBytes)) {
        char GIEsave = INTCON0bits.GIE;
        /* address is row aligned */
        if (address < FH_getFLASHWrapAddress()) {
            /* Check if erase needed */
            memcpy(flashBuf, (const void*) address, flashUnitBytes);
            uint8_t i;
            for (i = 0; i < flashUnitBytes; ++i) {
                if ((flashBuf[i] != 0xFF) && (flashBuf[i] != buffer[i])) {
                    /* Non FF found, erase row */
                    if (enableErase) {
                        TBLPTR = address;
                        FH_eraseCurrentFLASHRow();
                    } else {
                        return false;
                    }
                }
                break;
            }
            // Write block of data
            flashUnitBytes = DCI_writeRowBytes;
            TBLPTR = address;
            for (i = 0; i < flashUnitBytes; i++) {
                TABLAT = buffer[i]; // Load data byte
                if (i == (flashUnitBytes - 1)) {
                    asm("TBLWT");
                } else {
                    asm("TBLWTPOSTINC");
                }
            }
        }
        NVMCON1 = 0x84; /* operate on FLASH and enable writes */
        INTCON0bits.GIE = 0; // Disable interrupts
        NVMCON2 = 0x55;
        NVMCON2 = 0xAA;
        NVMCON1bits.WR = 1; // Start program
        NVMCON1 &= _NVMCON1_WRERR_MASK;
        GIE = GIEsave;
        return (memcmp(buffer, (const void*) address, flashUnitBytes) == 0);
    }
    return false;
}
#pragma regsused FH_writeVerifyFlash TBLPTRL, TBLPTRH, TBLPTRU, TABLAT

void FH_jumpToApp(uint16_t offset) {
    STKPTR = 1;
    TOS = FH_getBootEndAddress() + offset;
    STKPTR = 1;
    asm("RETURN");
}
