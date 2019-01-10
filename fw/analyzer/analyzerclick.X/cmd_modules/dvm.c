#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "../hwresources.h"
#include "../mcc_generated_files/mcc.h"
#include "../tasks.h"
#include "../cmdline.h"
#include "../pinhelper.h"
#include "../global_settings.h"
#include "../hwresources.h"
#include "../xterm.h"
#include "../string_helpers.h"
#include "../timer_helper.h"
#include "dvm.h"

#define DVM_ANALOG_CHANGE_MSEC  250
#define DVM_ANALOG_CHANGE_TICKS (DVM_ANALOG_CHANGE_MSEC / TS_getTickTimeMsec())

static enum {
    DVM_idle,
    DVM_init,
    DVM_selectNextChannel,
    DVM_waitConvertDone,
    DVM_convertJSON,
    DVM_convertXTERM,
    DVM_sendCmdBuffer,
    DVM_waitSend,
} DVM_state = DVM_idle;

typedef struct {
    float vrefVoltage;
    uint8_t ADCbits;
    uint8_t numChannels;
    uint16_t analogPinRaw[1];
} DVM_samplingBuf_t;

static DVM_samplingBuf_t* DVM_samplingBuf;
static uint8_t* DVM_outputBuf;
static bit DVM_firstAcquisition;
static uint8_t DVM_ticks;

static char* DVM_strinsert_channelNumeric(char* renderBuf, uint8_t pinNum, uint16_t adcRaw) {
    float analogValue = adcRaw * DVM_samplingBuf->vrefVoltage / 4095.0;
    sprintf(renderBuf, ANSI_CSI "%dH" ANSI_SETCOLOR("0;30;1") "Pin %-2d" ANSI_SETCOLOR("37") " %#.3fV",
            ((pinNum - 1) << 1) + XTERM_DVM_firstRow,
            pinNum,
            analogValue
            );
    renderBuf = renderBuf + strlen(renderBuf);
    return renderBuf;
}

static char* DVM_strinsert_channelBar(char* renderBuf, uint8_t pinNum, uint16_t adcRaw) {
    uint8_t magnitudeBar = ((unsigned long) adcRaw * XTERM_DVM_magnitudeLength) / 4095;
    sprintf(renderBuf, ANSI_CSI "%d;" ___mkstr(XTERM_DVM_magnitudeLeft) "H" ANSI_SETCOLOR("0;1;36") "[",
            ((pinNum - 1) << 1) + XTERM_DVM_firstRow
            );
    renderBuf = renderBuf + strlen(renderBuf);
    uint8_t i;
    for (i = 0; i < XTERM_DVM_magnitudeLength; ++i) {
        *renderBuf++ = (magnitudeBar > i) ? '#' : '-';
    }
    *renderBuf++ = ']';
    *renderBuf = 0;
    return renderBuf;
}

static void DVM_task(void) {
    switch (DVM_state) {
        case DVM_idle:
            TS_setExitFunc(NULL);
            break;

        case DVM_init:
            DVM_outputBuf = NULL;
            DVM_samplingBuf = (DVM_samplingBuf_t*) GLOBAL_claimSamplingBuffer();
            if (DVM_samplingBuf) {
                PINS_setAllPinsAnalog(true);
                GLOBAL_setSampleBufByteSize(sizeof (DVM_samplingBuf_t) + (PINS_getNumPins() - 1) * sizeof (DVM_samplingBuf->analogPinRaw[0]));
                DVM_samplingBuf->vrefVoltage = PINS_measureVref();
                DVM_samplingBuf->ADCbits = 12;
                DVM_samplingBuf->numChannels = 0;
                DVM_state = DVM_selectNextChannel;
            }
            break;

        case DVM_selectNextChannel:
            if (DVM_samplingBuf->numChannels < PINS_getNumPins()) {
                /* assign ADC to the channel */
                if (PINS_pinToADC(PINS_pinnumToPinDef(DVM_samplingBuf->numChannels + 1U))) {
                    // Turn on the ADC module
                    ADCON0bits.ADON = 1;
                    // Start the conversion
                    ADCON0bits.ADGO = 1;
                    DVM_state = DVM_waitConvertDone;
                }
            } else {
                DVM_state = (GLOBAL_getOutputMode() == GLOBAL_output_mode_binary) ? DVM_sendCmdBuffer : DVM_waitSend;
            }
            break;

        case DVM_waitConvertDone:
            if (!ADCON0bits.ADGO) {
                DVM_samplingBuf->analogPinRaw[DVM_samplingBuf->numChannels++] = (unsigned)((ADRESH << 8) + ADRESL);
                DVM_state = DVM_selectNextChannel;
            }
            break;

        case DVM_convertXTERM:
            if (DVM_outputBuf) {
                *DVM_outputBuf = 0; /* If buffer used for text, make sure it's empty... */
                if (DVM_samplingBuf->numChannels == PINS_getNumPins()) {
                    /* First run */
                    DVM_outputBuf = XTERM_strinsert_clientWindowPreamble(DVM_outputBuf, (bool)DVM_firstAcquisition);
                    DVM_outputBuf = XTERM_strinsert_spinner(DVM_outputBuf);
                    if (DVM_firstAcquisition) {
                        /* First time first run */
                        DVM_ticks = DVM_ANALOG_CHANGE_TICKS; /* At first acquisition make sure, numerical display is refreshed */
                    }
                }
                if (DVM_ticks >= DVM_ANALOG_CHANGE_TICKS) {
                    /* Time to update the decimal value */
                    DVM_ticks -= DVM_ANALOG_CHANGE_TICKS;
                    sprintf(DVM_outputBuf, ANSI_SETCOLOR("0") ANSI_SETCURSOR(XTERM_userWindowLeft, XTERM_userWindowTopRow) "Analyzer USB voltage is " ANSI_SETCOLOR("1;35") "%.3fV" ANSI_SETCOLOR("0"), DVM_samplingBuf->vrefVoltage);
                    DVM_outputBuf = DVM_outputBuf + strlen(DVM_outputBuf);
                    uint8_t pinIter;
                    for (pinIter = 0; pinIter < PINS_getNumPins(); ++pinIter) {
                        DVM_outputBuf = DVM_strinsert_channelNumeric(DVM_outputBuf, pinIter + 1U, DVM_samplingBuf->analogPinRaw[pinIter]);
                    }
                    DVM_state = DVM_sendCmdBuffer;
                }
                uint8_t pinNum = PINS_getNumPins() - DVM_samplingBuf->numChannels; /* pinNum adjusted for array access */
                DVM_outputBuf = DVM_strinsert_channelBar(DVM_outputBuf, pinNum + 1U, DVM_samplingBuf->analogPinRaw[pinNum]);
                if (!--DVM_samplingBuf->numChannels || (strlen(CMD_getOutputBufferAddress()) > (CMD_getOutputBufferSize() - 300))) {
                    /* Send if no more channels to render or output buffer near capacity */
                    DVM_state = DVM_sendCmdBuffer;
                }
            } else {
                /* try to claim output buffer */
                DVM_outputBuf = CMD_claimOutputBuffer();
            }
            break;

        case DVM_convertJSON:
            if (DVM_outputBuf) {
                *DVM_outputBuf = 0; /* If buffer used for text, make sure it's empty... */
                if (DVM_samplingBuf->numChannels == PINS_getNumPins()) {
                    /* First run */
                    strcpy(DVM_outputBuf, "{\"DVM\":{\"voltages\":[");
                    DVM_outputBuf = DVM_outputBuf + strlen(DVM_outputBuf);
                }
                sprintf(DVM_outputBuf, "%f", DVM_samplingBuf->analogPinRaw[--DVM_samplingBuf->numChannels] * DVM_samplingBuf->vrefVoltage / 4095.0);
                DVM_outputBuf = DVM_outputBuf + strlen(DVM_outputBuf);
                if (DVM_samplingBuf->numChannels) {
                    *DVM_outputBuf++ = ',';
                    *DVM_outputBuf = 0;
                } else {
                    strcat(DVM_outputBuf, "]}}");
                    DVM_state = DVM_sendCmdBuffer;
                }
            } else {
                /* try to claim output buffer */
                DVM_outputBuf = CMD_claimOutputBuffer();
            }
            break;

        case DVM_sendCmdBuffer:
            if (CMD_startSendOutputBuffer()) {
                /* Buffer is scheduled for sending */
                DVM_outputBuf = NULL;
                CMD_setNumPayloadBytes((GLOBAL_getOutputMode() == GLOBAL_output_mode_binary) ? GLOBAL_getSampleBufByteSize() : strlen(CMD_getOutputBufferAddress()));
                CMD_setPayloadType(CMD_payload_is_DIGITAL_VOLTMETER);
                DVM_state = DVM_waitSend;
            }
            break;

        case DVM_waitSend:
            if (CMD_isTransmissionComplete()) {
                if (DVM_samplingBuf->numChannels) {
                    switch (GLOBAL_getOutputMode()) {
                        case GLOBAL_output_mode_binary:
                            DVM_samplingBuf->numChannels = 0;
                            break;
                        case GLOBAL_output_mode_JSON:
                            DVM_state = DVM_convertJSON;
                            break;
                        case GLOBAL_output_mode_XTERM:
                            DVM_state = DVM_convertXTERM;
                            break;
                        default:
                            DVM_state = DVM_idle;
                            break;
                    }
                } else {
                    DVM_firstAcquisition = 0;
                    if (GLOBAL_acq_mode_continous == GLOBAL_getAcquisitionMode()) {
                        if (GLOBAL_isStopRequested()) {
                            GLOBAL_setStopRequest(false);
                            CMD_refreshFrame();
                            DVM_state = DVM_idle; /* Stop does this, but better be safe */
                        } else {
                            /* Restart with current parameters */
                            DVM_state = DVM_init;
                        }
                    } else {
                        CMD_refreshFrame();
                        DVM_state = DVM_idle; /* Stop does this, but better be safe */
                    }
                }
            }
            break;

        default:
            DVM_state = DVM_idle;
            break;
    }
}

static void DVM_onExit(void) {
    DVM_state = DVM_idle;
    PINS_setAllPinsAnalog(false);
    TS_setOnTickFunc(NULL);
    TS_setTaskFunc(NULL);
    TS_setOnTickFunc(NULL);
}

static void DVM_onTick() {
    if (DVM_ticks < 255) {
        ++DVM_ticks;
    }
}

bool DVM_onCommand(const char* cmdLine) {
    if (!strcmp(CMD_getCommand(cmdLine), "DVM")) {
        /* The command is for this module */
        GLOBAL_setStopRequest(false);
        TS_setExitFunc(DVM_onExit);
        TS_setTaskFunc(DVM_task);
        TS_setOnTickFunc(DVM_onTick);
        DVM_state = DVM_init;
        DVM_firstAcquisition = 1;
        return true;
    }
    return false;
}
