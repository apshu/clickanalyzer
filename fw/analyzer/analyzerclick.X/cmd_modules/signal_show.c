#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "../hwresources.h"
#include "../pinhelper.h"
#include "../mcc_generated_files/mcc.h"
#include "../tasks.h"
#include "../cmdline.h"
#include "signal_show.h"
#include "../global_settings.h"
#include "../xterm.h"
#include "../mouse.h"

#define SSHOW_maxNumParameters 4

typedef struct {
    PINS_pindef_t *inPin;
    uint8_t offDelayTicks;
    uint8_t activityCtr;
} SSHOW_statusled_t;

static SSHOW_statusled_t SSHOW_LEDs[4];

static enum {
    SSHOW_waitTxBufferReady,
    SSHOW_waitSendComplete,
} SSHOW_state = SSHOW_waitTxBufferReady;

static char* SSHOW_strinsert_LEDstatus(char* renderBuf, const char* name, PINS_pindef_t *inPin) {
    if (name) {
        if (inPin) {
#if defined(CIRCUITBOARD) && (CIRCUITBOARD > 0) && ((CIRCUITBOARD == PCB_gumstick) || (CIRCUITBOARD == PCB_gumstick_alt_UART))
            sprintf(renderBuf, "%s" ANSI_SETCOLOR("0") " Signal LED assigned to pin %d (R%c%d)",
                    name,
                    PINS_pinDefToPinNum(inPin),
                    inPin->portName,
                    inPin->portBit);
#else
            sprintf(renderBuf, "%s" ANSI_SETCOLOR("0") " Signal LED assigned to pin %d",
                    name,
                    PINS_pinDefToPinNum(inPin));
#endif
        } else {
            sprintf(renderBuf, "%s" ANSI_SETCOLOR("0") " Signal LED not assigned to signal change monitoring", name);
        }
    }
    return renderBuf + strlen(renderBuf);
}

static void SSHOW_task(void) {
    switch (SSHOW_state) {
        case SSHOW_waitTxBufferReady:
        {
            char* outBuf = CMD_claimOutputBuffer();
            if (outBuf) {
                *outBuf = 0; /* Empty buffer */
                char *replyBuf = XTERM_strinsert_clientScrollingWindowPreamble(outBuf, true);
                switch (GLOBAL_getOutputMode()) {
                    case GLOBAL_output_mode_binary:
                    case GLOBAL_output_mode_JSON:
#pragma warning push
#pragma warning disable 336 /* Disable warning: (336) string concatenation across lines */
                        sprintf(replyBuf, "{\"pins\":{\"LED\":{"
                                "\"YELLOW\":%d,"
                                "\"ORANGE\":%d,"
                                "\"GREEN\":%d,"
                                "\"RED\":%d"
                                "}}}",
#pragma warning pop
                                PINS_pinDefToPinNum(SSHOW_LEDs[PINS_LED_YELLOW].inPin),
                                PINS_pinDefToPinNum(SSHOW_LEDs[PINS_LED_ORANGE].inPin),
                                PINS_pinDefToPinNum(SSHOW_LEDs[PINS_LED_GREEN].inPin),
                                PINS_pinDefToPinNum(SSHOW_LEDs[PINS_LED_RED].inPin)
                                );
                        break;
                    case GLOBAL_output_mode_XTERM:
                        replyBuf = SSHOW_strinsert_LEDstatus(replyBuf, ANSI_SETCOLOR("1;33") "YELLOW", SSHOW_LEDs[PINS_LED_YELLOW].inPin);
                        replyBuf = SSHOW_strinsert_LEDstatus(replyBuf, "\r\n" ANSI_SETCOLOR("38;5;208") "ORANGE", SSHOW_LEDs[PINS_LED_ORANGE].inPin);
                        replyBuf = SSHOW_strinsert_LEDstatus(replyBuf, "\r\n" ANSI_SETCOLOR("32") "GREEN", SSHOW_LEDs[PINS_LED_GREEN].inPin);
                        replyBuf = SSHOW_strinsert_LEDstatus(replyBuf, "\r\n" ANSI_SETCOLOR("1;31") "RED", SSHOW_LEDs[PINS_LED_RED].inPin);
                        break;
                    default:
                        /* Unimplemented mode with no reply */
                        TS_setTaskFunc(NULL);
                        break;
                }
                CMD_setPayloadType(CMD_payload_is_GET_RESPONSE);
                CMD_setNumPayloadBytes(strlen(outBuf));
                if (CMD_startSendOutputBuffer()) {
                    CMD_refreshFrame();
                    SSHOW_state = SSHOW_waitSendComplete;
                }
            }
        }
            break;

        case SSHOW_waitSendComplete:
            if (CMD_isTransmissionComplete()) {
                TS_setTaskFunc(NULL);
            }
            break;

        default:
            TS_setTaskFunc(NULL);
            break;
    }
}

bool SSHOW_onCommand(const char* cmdLine) {
    if (!strcmp(CMD_getCommand(cmdLine), "LED")) {

        /* The command is for this module */
        enum {
            paraccept = 1,
            par1 = paraccept << 1,
            par2 = par1 << 1,
            par3 = par2 << 1,
            par4 = par3 << 1,
            optionalParams = par1 | par2 | par3 | par4,
            mandatoryParams = paraccept,
        } status = paraccept;
        uint8_t paramId;
        char parName[CMD_maxParamNameLength + 1];
        long parValue;
        for (paramId = 1; (status & paraccept) && CMD_getParam(cmdLine, paramId, parName, &parValue) && (paramId <= SSHOW_maxNumParameters); ++paramId) {
            if ((parValue >= 0) && ((unsigned) parValue <= PINS_getNumPins())) {
                /* pin number parameter is in required range */
                PINS_output_t thePin;
                if (!strcmp(parName, "RED")) {
                    thePin = PINS_LED_RED;
                    status |= par1;
                } else {
                    if (!strcmp(parName, "GREEN")) {
                        thePin = PINS_LED_GREEN;
                        status |= par2;
                    } else {
                        if (!strcmp(parName, "YELLOW")) {
                            thePin = PINS_LED_YELLOW;
                            status |= par3;
                        } else {
                            if (!strcmp(parName, "ORANGE")) {
                                thePin = PINS_LED_ORANGE;
                                status |= par4;
                            } else {
                                /* Unknown parameter */
                                status = 0;
                                continue;
                            }
                        }
                    }
                }
                SSHOW_LEDs[thePin].inPin = PINS_pinnumToPinDef(parValue);
                SSHOW_LEDs[thePin].activityCtr = 0;
                SSHOW_LEDs[thePin].offDelayTicks = SSHOW_defaultOffDelayTicks();
            }
        }
        if ((status & (optionalParams | mandatoryParams)) >= mandatoryParams) { /* all parameters are optional */
            /* Command processed successfully */
            SSHOW_state = SSHOW_waitTxBufferReady;
            TS_setTaskFunc(SSHOW_task);
            return true;
        }
        /* Errors detected */
        CMD_sendNAK();
        /* Command processed */
        return true;
    }
    /* Command not recognized */
    return false;
}

static void SSHOW_processStatusLED(SSHOW_statusled_t *theLED, PINS_output_t outPin) {
    if (theLED) {
        if (PINS_isDigitalInputChanged(theLED->inPin)) {
            theLED->activityCtr = theLED->offDelayTicks;
            PINS_setPin(outPin, true);
        } else {
            if (theLED->activityCtr && theLED->inPin) {
                --theLED->activityCtr;
            } else {
                PINS_setPin(outPin, false);
            }
        }
    }
}

void SSHOW_onTick(void) {
    PINS_fetchDigitalInputChanges();
    uint8_t i = sizeof (SSHOW_LEDs) / sizeof (SSHOW_LEDs[0]);
    for (; i > 0; --i) {
        SSHOW_processStatusLED(SSHOW_LEDs + i - 1, i - 1);
    }
}

uint8_t SSHOW_getInputPinForLED(PINS_output_t outPin) {
    if (outPin < (sizeof (SSHOW_LEDs) / sizeof (SSHOW_LEDs[0]))) {
        return PINS_pinDefToPinNum(SSHOW_LEDs[outPin].inPin);
    }
    return 0;
}

void SSHOW_onMouse(uint16_t mouseX, uint16_t mouseY, uint16_t mouseEvent) {
    const char* ledcolor = NULL;
    PINS_output_t thePin;
    if ((mouseX >= XTERM_buttonYellowLeft) && (mouseX <= XTERM_buttonYellowRight) && (mouseY == XTERM_buttonYellowTopRow)) {
        ledcolor = "YELLOW";
        thePin = PINS_LED_YELLOW;
    } else {
        if ((mouseX >= XTERM_buttonOrangeLeft) && (mouseX <= XTERM_buttonOrangeRight) && (mouseY == XTERM_buttonOrangeTopRow)) {
            ledcolor = "ORANGE";
            thePin = PINS_LED_ORANGE;
        } else {
            if ((mouseX >= XTERM_buttonGreenLeft) && (mouseX <= XTERM_buttonGreenRight) && (mouseY == XTERM_buttonGreenTopRow)) {
                ledcolor = "GREEN";
                thePin = PINS_LED_GREEN;
            } else {
                if ((mouseX >= XTERM_buttonRedLeft) && (mouseX <= XTERM_buttonRedRight) && (mouseY == XTERM_buttonRedTopRow)) {
                    ledcolor = "RED";
                    thePin = PINS_LED_RED;
                } else {
                    return;
                }
            }
        }
    }
    if (mouseEvent & MOUSE_EVT_release) {
        mouseEvent = MOUSE_eventBurronsOnly(mouseEvent);
        if (mouseEvent == MOUSE_EVT_BTN_left) {
            char cmdlinebuf[20];
            sprintf(cmdlinebuf, "LED%c%s", CMD_char_SEPARATOR, ledcolor);
            CMD_startSingleCommand(cmdlinebuf, CMD_char_ASSIGNMENT);
        } else {
            if (mouseEvent == MOUSE_EVT_BTN_right) {
                SSHOW_LEDs[thePin].inPin = NULL;
                CMD_refreshFrame();
            } else {
                if ((mouseEvent == MOUSE_EVT_BTN_wheelup) || (mouseEvent == MOUSE_EVT_BTN_wheeldown)) {
                    int8_t newPin = PINS_pinDefToPinNum(SSHOW_LEDs[thePin].inPin);
                    newPin += ((mouseEvent == MOUSE_EVT_BTN_wheelup) ? 1 : -1);
                    if (newPin < 0) {
                        newPin = PINS_getNumPins();
                    }
                    if (newPin > PINS_getNumPins()) {
                        newPin = 0;
                    }
                    SSHOW_LEDs[thePin].inPin = PINS_pinnumToPinDef(newPin);
                    PINS_isDigitalInputChanged(SSHOW_LEDs[thePin].inPin); /* Clear detected edges */
                    SSHOW_LEDs[thePin].activityCtr = 0;
                    SSHOW_LEDs[thePin].offDelayTicks = SSHOW_defaultOffDelayTicks();
                    CMD_refreshFrame();
                }
            }
        }
    }
}