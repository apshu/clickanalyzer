#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "../hwresources.h"
#include "../mcc_generated_files/mcc.h"
#include "../tasks.h"
#include "../cmdline.h"
#include "logic_scope.h"
#include "../pinhelper.h"
#include "../global_settings.h"
#include "../hwresources.h"
#include "../xterm.h"
#include "../string_helpers.h"
#include "../timer_helper.h"

#define LS_maxNumParameters 4

static uint8_t* LS_outputBuf = NULL;
static LS_payloadBuf_t* LS_samplingBuf = NULL;
static uint8_t LS_numSamples;
static uint8_t LS_numBuffersToSend;
static bit LS_GIEsave;
static bit LS_firstAcquisition;

static enum {
    LS_idle,
    LS_init,
    LS_waitSampleDone,
    LS_convert_JSON,
    LS_convert_XTERM,
    LS_sendCmdBuffer,
    LS_waitSend,
} LS_state = LS_idle;

static void inline LS_stop(void) {
    CPUDOZE = 0; /* Restore slow motion for CPU */
    LS_NCOreg(NCO, CONbits).EN = 0;
    LS_DMAreg(DMA, DGO) = 0;
    LS_DMAreg(DMA, EN) = 0;
    CMD_unlockOutputBuffer();
    LS_state = LS_idle;
}

size_t inline LS_getMaxNumberOfSamples(void) {
    return ((GLOBAL_getMaxSampleBufBytes() - sizeof (LS_samplingBuf->pinmapInfo)) / sizeof (LS_samplingBuf->dataRecords[0]));
}

/* Returns number of records populated to the buffer */
static uint8_t LS_populatePinmap(uint8_t *thePinmap) {
    if (thePinmap) {
        uint8_t i;
        for (i = 1; i <= (sizeof (LS_samplingBuf->pinmapInfo.pinmap) / sizeof (LS_samplingBuf->pinmapInfo.pinmap[0])); ++i) {
            PINS_pindef_t* pindef = PINS_pinnumToPinDef(i);
            if (GLOBAL_getOutputMode() == GLOBAL_output_mode_binary) {
                *thePinmap++ = (unsigned)(PINS_pinToLinearBitnumber(pindef) - 8); /* -8 : we start streaming from PORTB */
            } else {
                /* Pinmap is comma separated list of strings */
                sprintf(thePinmap, "%s%u", (i == 1) ? "" : ",", 1 << (PINS_pinToLinearBitnumber(pindef) - 8));
                thePinmap = thePinmap + strlen(thePinmap);
            }
        }
        return (sizeof (LS_samplingBuf->pinmapInfo.pinmap) / sizeof (LS_samplingBuf->pinmapInfo.pinmap[0]));
    }
    return 0;
}

static void LS_onExit(void) {
    LS_stop();
    TS_setTaskFunc(NULL);
}

static void LS_task(void) {
    switch (LS_state) {
        case LS_idle:
            LS_stop();
            TS_setExitFunc(NULL);
            TS_setTaskFunc(NULL);
            break;

        case LS_init:
            LS_samplingBuf = (LS_payloadBuf_t*) GLOBAL_claimSamplingBuffer();
            LS_outputBuf = NULL;
            if (LS_samplingBuf) {
                GLOBAL_setSampleBufByteSize(LS_numSamples * sizeof (LS_samplingBuf->dataRecords[0]) + sizeof (LS_samplingBuf->pinmapInfo));
                LS_numBuffersToSend = 0xFF; /* Indicate first send */
                /* Add pinmap */
                LS_samplingBuf->pinmapInfo.pinmapEntries = LS_populatePinmap(LS_samplingBuf->pinmapInfo.pinmap);
                LS_NCOreg(NCO, CONbits).EN = 0;
                LS_NCOreg(NCO, ACC) = 0;
                LS_DMAreg(DMA, DGO) = 0;
                LS_DMAreg(DMA, EN) = 0;
                LS_DMAreg(DMA, CON1) = 0x62; /* Configures increment and stop conditions */
                LS_DMAreg(DMA, SSA) = (volatile unsigned short long) &PORTB;
                LS_DMAreg(DMA, DSA) = (volatile unsigned short) LS_samplingBuf->dataRecords;
                LS_DMAreg(DMA, SSZ) = sizeof (LS_samplingBuf->dataRecords[0]);
                LS_DMAreg(DMA, DSZ) = GLOBAL_getSampleBufByteSize() - sizeof (LS_samplingBuf->pinmapInfo);
                LS_DMAreg(DMA, SIRQ) = LS_NCOreg(IRQ_NCO,;);
                LS_DMAreg(DMA, SIRQEN) = 1;
                LS_DMAreg(DMA, AIRQ) = IRQ_SWINT;
                LS_DMAreg(DMA, AIRQEN) = 0;
                LS_DMAreg(DMA, DCNTIF) = 0;
                LS_DMAreg(DMA, EN) = 1;
                CPUDOZE = 0x80; /* IDLEN */
                LS_GIEsave = GIEL;
                GIEL = 0;
                LS_DMAreg(DMA, DCNTIE) = 1;
                LS_state = LS_waitSampleDone;
                LS_NCOreg(NCO, CONbits).EN = 1;
                Sleep();
            }
            break;

        case LS_waitSampleDone:
            if (LS_DMAreg(DMA, DCNTIF) == 1) {
                LS_DMAreg(DMA, DCNTIF) = 0;
                LS_DMAreg(DMA, DCNTIE) = 0;
                GIEL = LS_GIEsave;
                LS_DMAreg(DMA, EN) = 0;
                LS_state = LS_waitSend;
            } else {
                Sleep();
            }
            break;

        case LS_sendCmdBuffer:
            if (CMD_startSendOutputBuffer()) {
                /* Buffer is scheduled for sending */
                LS_outputBuf = NULL;
                CMD_setNumPayloadBytes((GLOBAL_getOutputMode() == GLOBAL_output_mode_binary) ? GLOBAL_getSampleBufByteSize() : strlen(CMD_getOutputBufferAddress()));
                CMD_setPayloadType(CMD_payload_is_LOGIC_SCOPE);
                LS_state = LS_waitSend;
            }
            break;

        case LS_convert_JSON:
            if (LS_outputBuf) {
                *LS_outputBuf = 0; /* If buffer used for text, make sure it's empty... */
                if (LS_numBuffersToSend == 0xFF) {
                    /* First run */
                    sprintf(LS_outputBuf, "{\"LS\":{\"samplerate\":%f,\"pins\":[", TMRxToHz(LS_NCOID | TMRHELPER_TMRIS_NCO, _XTAL_FREQ));
                    LS_populatePinmap(LS_outputBuf + strlen(LS_outputBuf));
                    strcat(LS_outputBuf, "],\"data\":[");
                    LS_outputBuf = LS_outputBuf + strlen(LS_outputBuf);
                    LS_numBuffersToSend = LS_numSamples;
                } else {
                    sprintf(LS_outputBuf, "%u", LS_samplingBuf->dataRecords[(unsigned)(LS_numSamples - LS_numBuffersToSend)]);
                    LS_outputBuf = LS_outputBuf + strlen(LS_outputBuf);
                    if (--LS_numBuffersToSend) {
                        *LS_outputBuf++ = ',';
                        *LS_outputBuf = 0;
                    } else {
                        strcat(LS_outputBuf, "]}}");
                        LS_state = LS_sendCmdBuffer;
                    }
                }
            } else {
                /* try to claim output buffer */
                LS_outputBuf = CMD_claimOutputBuffer();
            }
            break;

        case LS_convert_XTERM:
            if (LS_outputBuf) {
                if (LS_numBuffersToSend == 0xFF) {
                    /* First run */
                    *LS_outputBuf = 0; /* Buffer used for text, make sure it's empty... */
                    LS_outputBuf = XTERM_strinsert_spinner(LS_outputBuf);
                    LS_outputBuf = XTERM_strinsert_clientWindowPreamble(LS_outputBuf, (bool)LS_firstAcquisition);
                    float tmrFrequency = TMRxToHz(LS_NCOID | TMRHELPER_TMRIS_NCO, (float) _XTAL_FREQ);
                    char tmpBuf1[30];
                    char tmpBuf2[30];
                    sprintf(LS_outputBuf, "One character is " ANSI_SETCOLOR("1;35") "%ss" ANSI_SETCOLOR("0") " (=%ssps) (Maximum bandwidth 4.0Msps)\r\n", floatToEng(1.0 / tmrFrequency, tmpBuf1), floatToEng(tmrFrequency, tmpBuf2));
                    LS_numBuffersToSend = PINS_getNumPins();
                }
                LS_outputBuf += strlen(LS_outputBuf);
                uint8_t pinNum = PINS_getNumPins() - LS_numBuffersToSend + 1;
                PINS_pindef_t* thePin = PINS_pinnumToPinDef(pinNum);
                sprintf(LS_outputBuf, ANSI_CSI "%d;1H" ANSI_SETCOLOR("30;1") "Pin %d\r\n" ANSI_SETCOLOR("36"), ((pinNum - 1) << 1) + XTERM_LS_firstRow, pinNum);
                uint8_t smpId;
                uint16_t pinMask = (unsigned)(1 << (PINS_pinToLinearBitnumber(thePin) - 8));
                LS_outputBuf = LS_outputBuf + strlen(LS_outputBuf);
                for (smpId = 0; smpId < LS_numSamples; ++smpId) {
                    *LS_outputBuf++ = (LS_samplingBuf->dataRecords[smpId] & pinMask) ? 'H' : '-';
                }
                *LS_outputBuf = 0;
                if (!--LS_numBuffersToSend || (strlen(CMD_getOutputBufferAddress()) > (CMD_getOutputBufferSize() - 200))) {
                    LS_state = LS_sendCmdBuffer;
                }
            } else {
                /* try to claim output buffer */
                LS_outputBuf = CMD_claimOutputBuffer();
            }
            break;

        case LS_waitSend:
            if (CMD_isTransmissionComplete()) {
                if (LS_numBuffersToSend) {
                    switch (GLOBAL_getOutputMode()) {
                        case GLOBAL_output_mode_binary:
                            LS_numBuffersToSend = 0;
                            LS_state = LS_sendCmdBuffer;
                            break;
                        case GLOBAL_output_mode_JSON:
                            LS_state = LS_convert_JSON;
                            break;
                        case GLOBAL_output_mode_XTERM:
                            LS_state = LS_convert_XTERM;
                            break;
                        default:
                            LS_state = LS_idle;
                            break;
                    }
                } else {
                    LS_firstAcquisition = 0;
                    if (GLOBAL_acq_mode_continous == GLOBAL_getAcquisitionMode()) {
                        if (GLOBAL_isStopRequested()) {
                            GLOBAL_setStopRequest(false);
                            CMD_refreshFrame();
                            LS_state = LS_idle; /* Stop does this, but better be safe */
                        } else {
                            /* Restart with current parameters */
                            LS_state = LS_init;
                        }
                    } else {
                        CMD_refreshFrame();
                        LS_state = LS_idle; /* Stop does this, but better be safe */
                    }
                }
            }
            break;

        default:
            LS_state = LS_idle;
            break;
    }
}

bool LS_onCommand(const char* cmdLine) {
    if (!strcmp(CMD_getCommand(cmdLine), "LS")) {
        /* The command is for this module */
        LS_stop();
        TS_setExitFunc(NULL);
        TS_setTaskFunc(NULL);
        //            CMD_setPayloadType(CMD_payload_is_LOGIC_SCOPE);

        enum {
            paraccept = 1,
            par1 = paraccept << 1,
            par2 = par1 << 1,
            par3 = par2 << 1, //FREQ
            par4 = par3 << 1, //NUMSMP
            mandatoryParams = paraccept | par3 | par4,
        } status = paraccept;
        if (GLOBAL_getOutputMode() == GLOBAL_output_mode_XTERM) {
            LS_numSamples = LS_getMaxNumberOfSamples();
            if (LS_numSamples > (XTERM_userWindowRight - XTERM_userWindowLeft + 1)) {
                LS_numSamples = XTERM_userWindowRight - XTERM_userWindowLeft + 1;
            }
            status |= par4;
        }
        uint8_t paramId;
        char parName[CMD_maxParamNameLength + 1];
        long parValue;
        for (paramId = 1; (status & paraccept) && CMD_getParam(cmdLine, paramId, parName, &parValue) && (paramId <= LS_maxNumParameters); ++paramId) {
            if (!strcmp(parName, "FREQ")) {
                if ((parValue >= 61) && (parValue <= 4000000L)) {
                    LS_NCOreg(NCO, INC) = (volatile unsigned short long)(((long double) parValue * 1048576.0) / (1.0 * _XTAL_FREQ));
                    if (!LS_NCOreg(NCO, INC)) {
                        LS_NCOreg(NCO, INC) = 1;
                    }
                    status |= par3;
                    continue;
                }
            }
            if (!strcmp(parName, "NUMSMP")) {
                if ((parValue > 0) && (parValue <= LS_getMaxNumberOfSamples())) {
                    LS_numSamples = (unsigned)parValue;
                    status |= par4;
                    continue;
                }
            }
            /* Unknown parameter */
            status = 0;
        }
        if ((status & mandatoryParams) == mandatoryParams) {
            TS_setExitFunc(LS_onExit);
            TS_setTaskFunc(LS_task);
            LS_state = LS_init;
            LS_firstAcquisition = 1;
            GLOBAL_setStopRequest(false);
            /* Command processed successfully */
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
