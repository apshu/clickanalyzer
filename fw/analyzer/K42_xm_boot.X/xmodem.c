#include <xc.h>
#include <stdint.h>
#include "xmodem.h"
#include "mcc_generated_files/mcc.h"

#define XM_SOH  0x01
#define XM_STX  0x02
#define XM_EOT  0x04
#define XM_ACK  0x06
#define XM_NAK  0x15
#define XM_CAN  0x18

#define DLY_1S 3
#define DLY_2S 7
#define DLY_1_5S 5

static union {
    uint8_t asByte[1031];

    struct {
        uint8_t command;
        uint8_t paketNo;
        uint8_t invPaketNo;
        uint8_t payload[1024];
    } asXM_header;

    struct {
        uint8_t command;
        uint8_t paketNo;
        uint8_t invPaketNo;
        uint8_t payload[1024];
        uint16_t crc16;
    } asXM_1024;

    struct {
        uint8_t command;
        uint8_t paketNo;
        uint8_t invPaketNo;
        uint8_t payload[128];
        uint8_t chkSum;
    } asXM_128;

    struct {
        uint8_t command;
        uint8_t paketNo;
        uint8_t invPaketNo;
        uint8_t payload[128];
        uint16_t crc16;
    } asXM_crc128;
} XM_xbuff;
static uint16_t XM_bufPtr;
static uint8_t XM_packetno;
static uint8_t XM_outChar;
static uint8_t XM_numRetries;
static bit XM_crcMode;
static bit XM_gotPacket;

#define _outbyte(txData) UART1_Write(txData)

int _inbyte(uint8_t delayCoeff) {
    TMR2_Initialize();
    T2CONbits.T2OUTPS = delayCoeff;
    TMR2 = 0;
    PIR4bits.TMR2IF = 0;
    while (!PIR4bits.TMR2IF) {
        if (XM_bufPtr < DMA1DPTR) {
            return *(uint8_t *) XM_bufPtr++;
        }
    }
    return -1;
}

static void XM_flushinput(void) {
    while (_inbyte(DLY_1_5S) >= 0) {
        continue;
    }
}

static uint16_t XM_getCRC16(uint8_t* buf, uint16_t bufLen) {
    CRCACCH = CRCACCL = 0;
    if (bufLen) {
        // SHIFTM shift left; CRCGO disabled; CRCEN enabled; ACCM data augmented with 0's; 
        //        CRCCON0 = 0x90;
        CRCCON0 = 0x90;
        // DLEN 7; PLEN 15; 
        CRCCON1 = (7 << 4) | (15);
        CRCXORH = 0x10;
        CRCXORL = 0x21;
        while (bufLen) {
            if (!CRCCON0bits.CRCGO) {
                CRCCON0bits.CRCGO = 1;
            }
            if (!CRCCON0bits.FULL) {
                CRCDATL = *buf++;
                --bufLen;
            }
        }
        while (CRCCON0bits.BUSY) {
            continue;
        }
        CRCCON0 = 0;
    }
    return (uint16_t) ((CRCACCL << 8U) | CRCACCH);
}

static uint8_t XM_getChecksum(uint8_t* buf, uint16_t bufLen) {
    uint8_t csum = 0;
    for (; bufLen; --bufLen) {
        csum += *buf++;
    }
    return (uint8_t) (-csum);
}

/* return 0 is packet received correctly */
static bit XM_getPacket(void) {
    uint16_t XM_bufsz = (unsigned) XM_crcMode + ((XM_xbuff.asXM_header.command == XM_STX) ? 1027U : 131U);
    for (; XM_bufsz; --XM_bufsz) {
        if (_inbyte(DLY_1S) < 0) {
            return 0;
        }
    }
    if ((XM_xbuff.asXM_header.paketNo ^ XM_xbuff.asXM_header.invPaketNo) == 0xFF) {
        /* Packet number okay */
        if ((XM_xbuff.asXM_header.paketNo == XM_packetno) || (XM_xbuff.asXM_header.paketNo == (XM_packetno + 1))) {
            /* Packet counter accepted */
            bool chkResult = 0;
            if (XM_crcMode) {
                if (XM_xbuff.asXM_header.command == XM_SOH) {
                    chkResult = (bool) (XM_xbuff.asXM_crc128.crc16 == XM_getCRC16(XM_xbuff.asXM_crc128.payload, 128U));
                } else {
                    chkResult = (bool) (XM_xbuff.asXM_1024.crc16 == XM_getCRC16(XM_xbuff.asXM_1024.payload, 1024U));
                }
            } else {
                chkResult = (bool) (XM_xbuff.asXM_128.chkSum == (uint8_t) (-XM_getChecksum(XM_xbuff.asXM_128.payload, 128U)));
            }
            if (chkResult) {
                XM_packetno = XM_xbuff.asXM_header.paketNo;
                return 1;
            }
        }
    }
    return 0;
}

/* Returns pointer to buffer, length reported in pktLen. If no valid packet, return is NULL */
uint8_t *XM_getData(int16_t *pktLen) {
    if (XM_gotPacket) {
        if (pktLen) {
            *pktLen = (XM_xbuff.asXM_header.command == XM_STX) ? 1024 : 128;
        }
        return XM_xbuff.asXM_header.payload;
    } else {
        if (pktLen) {
            *pktLen = 0;
        }
    }
    return (uint8_t*) 0;
}

void inline XM_setupArbiter(void) {
    /* Setup Priorities */
    ISRPR = 0;
    MAINPR = 1;
    DMA1PR = 2;
    DMA2PR = 3;
    SCANPR = 4;
    /* Lock priorities */
    asm("BCF INTCON0,7");
    asm("BANKSEL PRLOCK");
    asm("MOVLW 0x55");
    asm("MOVWF LOW(PRLOCK)");
    asm("MOVLW 0xAA");
    asm("MOVWF LOW(PRLOCK)");
    asm("BSF LOW(PRLOCK), 0");
    asm("BSF INTCON0,7");
}

static void XM_resetRXDMA(void) {
    if (!PRLOCKbits.PRLOCKED) {
        XM_setupArbiter();
    }
    DMA1EN = 0;
    DMA1CON1 = 0x60; /* Configures increment and stop conditions */
    DMA1SSA = (volatile unsigned short long) &U1RXB;
    DMA1DSA = (volatile unsigned short) XM_xbuff.asByte;
    DMA1SSZ = 1;
    DMA1DSZ = sizeof (XM_xbuff.asByte);
    DMA1DCNT = 0;
    DMA1SIRQ = IRQ_U1RX;
    DMA1SIRQEN = 1;
    DMA1AIRQ = IRQ_SWINT;
    DMA1AIRQEN = 0;
    DMA1DCNTIF = 0;
    DMA1SCNTIF = 0;
    DMA1EN = 1;
    XM_bufPtr = DMA1DSA;
}

void XM_resetState(void) {
    XM_resetRXDMA();
    XM_packetno = 0;
    XM_outChar = 'C';
    XM_crcMode = 0;
    XM_numRetries = XM_MAXRETRIES;
    XM_flushinput();
}

/* return true if more cunks might follow */
bit XM_getNextChunk(void) {
    int inChar;
    uint8_t chTransmitted = XM_outChar;
    if (XM_outChar) {
        _outbyte(XM_outChar);
    }
    if (XM_numRetries < (XM_MAXRETRIES / 2)) {
        XM_outChar = XM_NAK;
    }
    if (XM_numRetries) {
        XM_gotPacket = 0; /* Packet data invalid */
        XM_resetRXDMA();
        if ((inChar = _inbyte(DLY_1S)) >= 0) {
            switch (inChar) {
                case XM_SOH:
                case XM_STX:
                    if (chTransmitted == 'C') {
                        XM_crcMode = 1;
                    }
                    if (XM_numRetries > 2) {
                        XM_numRetries = 2;
                    }
                    if (XM_getPacket()) {
                        /* Packet good */
                        XM_gotPacket = 1;
                        XM_numRetries = 2;
                        XM_outChar = XM_ACK;
                    } else {
                        /* Packet error */
                        XM_xbuff.asXM_header.command = 0; /* Packet data invalid */
                    }
                    break;
                case XM_EOT:
                    XM_flushinput();
                    XM_numRetries = 1;
                    _outbyte(XM_ACK);
                    return 0; /* normal end */
                case XM_CAN:
                    if (_inbyte(DLY_1S) == XM_CAN) {
                        XM_numRetries = 1;
                        XM_flushinput();
                        _outbyte(XM_ACK);
                        return 0; /* canceled by remote */
                    }
                    break;
                default:
                    break;
            }
        }
        if (XM_numRetries) {
            --XM_numRetries;
        }
        return 1;
    } else {
        XM_abortTransfer();
    }
    return 0;
}

void XM_abortTransfer(void) {
    XM_flushinput();
    _outbyte(XM_CAN);
    _outbyte(XM_CAN);
    _outbyte(XM_CAN);
    XM_outChar = 0;
    XM_numRetries = 0;
}
