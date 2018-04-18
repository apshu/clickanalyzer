#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "mcc_generated_files/mcc.h"
#include "pinhelper.h"
#include "hwresources.h"

#define PORTPINDEF(port,bit) { port##PORT, _PORT##port##_R##port##bit##_POSITION }
#define APORT 'A'
#define BPORT 'B'
#define CPORT 'C'

static PINS_pindef_t PINS_pinlist[] = {
#if defined( CIRCUITBOARD ) && ( CIRCUITBOARD > 0 )
#if (CIRCUITBOARD == PCB_gumstick_alt_UART) || (CIRCUITBOARD == PCB_gumstick)
    PORTPINDEF(B, 0),
    PORTPINDEF(B, 1),
    PORTPINDEF(B, 2),
    PORTPINDEF(B, 3),
    PORTPINDEF(B, 6),
    PORTPINDEF(B, 7),
    PORTPINDEF(C, 0),
    PORTPINDEF(C, 1),
    PORTPINDEF(C, 2),
    PORTPINDEF(C, 3),
    PORTPINDEF(C, 4),
    PORTPINDEF(C, 5),
    PORTPINDEF(C, 6),
    PORTPINDEF(C, 7),
#elif (CIRCUITBOARD == PCB_clickboard)
    PORTPINDEF(C, 1), // MB1
    PORTPINDEF(B, 6), // MB2
    PORTPINDEF(B, 7), // MB3
    PORTPINDEF(C, 3), // MB4
    PORTPINDEF(C, 5), // MB5
    PORTPINDEF(C, 0), // MB6
    PORTPINDEF(C, 2), // MB7
    PORTPINDEF(C, 6), // MB8
    PORTPINDEF(B, 2), // SDA
    PORTPINDEF(B, 1), // SCL
    PORTPINDEF(C, 7), // MB9
    PORTPINDEF(B, 0), // MB10
    PORTPINDEF(C, 4), // MB11
    PORTPINDEF(B, 3), // MB12
#else
#error "Unknown board type"
#endif
#else
#error "Please define circuit board type and/or include hwresources.h"
#endif
};

PINS_pindef_t* PINS_pinnumToPinDef(uint8_t pinId) {
    --pinId;
    if (pinId <= sizeof (PINS_pinlist) / sizeof (PINS_pinlist[0])) {
        return (PINS_pinlist + pinId);
    }
    return NULL;
}

uint8_t PINS_pinDefToPinNum(PINS_pindef_t* thePin) { /* 0 for error */
    if (thePin) {
        uint8_t i;
        for (i = 0; i<sizeof (PINS_pinlist) / sizeof (PINS_pinlist[0]); ++i) {
            if ((PINS_pinlist[i].portBit == thePin->portBit) && (PINS_pinlist[i].portName == thePin->portName)) {
                return (i + 1);
            }
        }
    }
    return 0;
}

static struct {
    uint8_t chgA;
    uint8_t chgB;
    uint8_t chgC;
} PINS_pinChangesCache;

void PINS_fetchDigitalInputChanges(void) {
    PINS_pinChangesCache.chgA = IOCAF;
    IOCAF &= ~PINS_pinChangesCache.chgA;
    PINS_pinChangesCache.chgB = IOCBF;
    IOCBF &= ~PINS_pinChangesCache.chgB;
    PINS_pinChangesCache.chgC = IOCCF;
    IOCCF &= ~PINS_pinChangesCache.chgC;
}

bool PINS_isDigitalInputChanged(PINS_pindef_t *thePin) {
    if (thePin) {
        uint8_t thePinMask = 1 << thePin->portBit;
        if (thePin->portName == 'A') {
            return PINS_pinChangesCache.chgA & thePinMask;
        }
        if (thePin->portName == 'B') {
            return PINS_pinChangesCache.chgB & thePinMask;
        }
        if (thePin->portName == 'C') {
            return PINS_pinChangesCache.chgC & thePinMask;
        }
    }
    return false;
}

#define PINCASE(color, ishigh) case PINS_LED_##color: do { IO_LED_##color##_SetDigitalMode(); IO_LED_##color##_OD = 0; IO_LED_##color##_SetDigitalOutput(); IO_LED_##color##_LAT = isHigh;} while (0); break;

void PINS_setPin(PINS_output_t thePin, bool isHigh) {
    switch (thePin) {
            PINCASE(RED, isHigh);
            PINCASE(GREEN, isHigh);
            PINCASE(ORANGE, isHigh);
            PINCASE(YELLOW, isHigh);
        default:
            break;
    }
}
#undef PINCASE

size_t inline PINS_getNumPins(void) {
    return sizeof (PINS_pinlist) / sizeof (PINS_pinlist[0]);
}

uint8_t PINS_pinToLinearBitnumber(PINS_pindef_t *thePin) { /* A0 = 0, A7=7, B0 = 8, etc... on error 0x3B */
    if (thePin) {
        if ((thePin->portName >= 'A') && (thePin->portName <= 'C')) {
            return (((thePin->portName - 'A') << 3) + thePin->portBit) & _ADPCH_ADPCH_MASK;
        }
    }
    return 0x3B; /* GND */
}

void PINS_setPinModeAnalog(PINS_pindef_t *thePin, bool isAnalog) {
    if (thePin) {
#if (CIRCUITBOARD == PCB_gumstick_alt_UART)
        if ((thePin->portName == 'B') && isAnalog) {
            if ((thePin->portBit == 4) || (thePin->portBit == 5)) {
                return; // Ignore analog mode for RB4 & RB5 in alt uart mode
            }
        }
#endif
        uint8_t thePinMask = 1 << thePin->portBit;
        switch (thePin->portName) {
            case 'A':
                if (isAnalog) {
                    ANSELA |= thePinMask;
                } else {
                    ANSELA &= ~thePinMask;
                }
                break;
            case 'B':
                if (isAnalog) {
                    ANSELB |= thePinMask;
                } else {
                    ANSELB &= ~thePinMask;
                }
                break;
            case 'C':
                if (isAnalog) {
                    ANSELC |= thePinMask;
                } else {
                    ANSELC &= ~thePinMask;
                }
                break;
        }
    }
}

bool PINS_pinToADC(PINS_pindef_t *thePin) {
    PINS_setPinModeAnalog(thePin, true);
    ADPCH = PINS_pinToLinearBitnumber(thePin);
    return (ADPCH != 0x3B);
}

float PINS_measureVref(void) {
    uint32_t refmeas = 0;
    uint8_t avg;
    for (avg = 0; avg < 10; ++avg) {
        refmeas += ADCC_GetSingleConversion(channel_FVR_Buffer1);
    }
    return ((2.048 * 4095.0 * avg) / refmeas);
}
