#include <xc.h>
#include <stdint.h>

uint16_t CRC_getCRC16CCITT_XMODEM(uint8_t* buf, uint16_t bufLen) {
    CRCACCH = CRCACCL = 0;
    if (bufLen) {
        // SHIFTM shift left; CRCGO disabled; CRCEN enabled; ACCM data augmented with 0's; 
        CRCCON0 = 0x90;
        // DLEN 7; PLEN 15; 
        CRCCON1 = (7 << 4) | (15);
        CRCXORH = 0x10;
        CRCXORL = 0x21;
        while (bufLen) {
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
    return CRCACC;
}
