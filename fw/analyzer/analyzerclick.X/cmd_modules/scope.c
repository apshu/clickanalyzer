#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "../hwresources.h"
#include "../mcc_generated_files/mcc.h"
#include "../tasks.h"
#include "../cmdline.h"
#include "scope.h"
#include "../pinhelper.h"
#include "../global_settings.h"
#include "../hwresources.h"
#include "../xterm.h"
#include "../string_helpers.h"
#include "../timer_helper.h"

#define SCOPE_maxNumParameters (4U)
#define SCOPE_ROWNUM (21U)

static uint8_t* SCOPE_outputBuf = NULL;
static SCOPE_payloadBuf_t* SCOPE_samplingBuf = NULL;
static uint8_t SCOPE_numSamples;
static uint8_t SCOPE_numBuffersToSend;
static bit SCOPE_GIEsave;
static bit SCOPE_firstAcquisition;
static PINS_pindef_t* SCOPE_ADCpin;

static uint8_t SCOPE_stream_convert[101];

static void draw_grid_row(void);

static enum {
    SCOPE_idle,
    SCOPE_init,
    SCOPE_waitSampleDone,
    SCOPE_convert_JSON,
    SCOPE_convert_XTERM,
    SCOPE_sendCmdBuffer,
    SCOPE_waitSend,
} SCOPE_state = SCOPE_idle;

static void inline SCOPE_stop(void) {
    CPUDOZE = 0; /* Restore slow motion for CPU */
    SCOPE_NCOreg(NCO, CONbits).EN = 0;
    SCOPE_DMAreg(DMA, DGO) = 0;
    SCOPE_DMAreg(DMA, EN) = 0;
    CMD_unlockOutputBuffer();
    PINS_setAllPinsAnalog(false);
    SCOPE_state = SCOPE_idle;
}

size_t inline SCOPE_getMaxNumberOfSamples(void) {
    return ((GLOBAL_getMaxSampleBufBytes() - sizeof (SCOPE_payloadBuf_t) + sizeof (SCOPE_samplingBuf->dataRecords[0])) / sizeof (SCOPE_samplingBuf->dataRecords[0]));
}

static void SCOPE_onExit(void) {
    SCOPE_stop();
    TS_setTaskFunc(NULL);
}

static void SCOPE_task(void) {
    switch (SCOPE_state) {
        case SCOPE_idle:
            SCOPE_stop();
            TS_setExitFunc(NULL);
            TS_setTaskFunc(NULL);
            break;

        case SCOPE_init:
            SCOPE_samplingBuf = (SCOPE_payloadBuf_t*) GLOBAL_claimSamplingBuffer();
            SCOPE_outputBuf = NULL;
            if (SCOPE_samplingBuf) { //Buffer claimed
                SCOPE_samplingBuf->vrefVoltage = PINS_measureVref();
                if (PINS_pinToADC(SCOPE_ADCpin)) { //ADC pin assigned
                    SCOPE_samplingBuf->sampledPin = PINS_pinDefToPinNum(SCOPE_ADCpin);
                    SCOPE_samplingBuf->ADCbits = 12;
                    ADPRE = 0;
                    ADACQ = 0;
                    ADCON0bits.ADCS = 0; // clock derived by FOSC
                    ADCON0bits.ADFM = 1; // right justified
                    ADCLKbits.ADCS = 31; // FOSC / 64
                    ADACTbits.ACT = 0b10010; // NCO output as auto conversion trigger
                    ADCON0bits.ADON = 1;
                    GLOBAL_setSampleBufByteSize(SCOPE_numSamples * sizeof (SCOPE_samplingBuf->dataRecords[0]) + sizeof (SCOPE_payloadBuf_t) - sizeof (SCOPE_samplingBuf->dataRecords));
                    SCOPE_numBuffersToSend = 0xFF; /* Indicate first send */
                    /* Setup payload header */
                    SCOPE_NCOreg(NCO, CONbits).EN = 0;
                    SCOPE_NCOreg(NCO, CONbits).PFM = 1;
                    SCOPE_NCOreg(NCO, ACC) = 0;
                    SCOPE_DMAreg(DMA, DGO) = 0;
                    SCOPE_DMAreg(DMA, EN) = 0;
                    SCOPE_DMAreg(DMA, CON1) = 0x62;
                    SCOPE_DMAreg(DMA, SSA) = (volatile unsigned short long) &ADRES;
                    SCOPE_DMAreg(DMA, DSA) = (volatile unsigned short) SCOPE_samplingBuf->dataRecords;
                    SCOPE_DMAreg(DMA, SSZ) = sizeof (SCOPE_samplingBuf->dataRecords[0]);
                    SCOPE_DMAreg(DMA, DSZ) = SCOPE_numSamples * sizeof (SCOPE_samplingBuf->dataRecords[0]);
                    SCOPE_DMAreg(DMA, SIRQ) = IRQ_AD; //SCOPE_NCOreg(IRQ_NCO,;);
                    SCOPE_DMAreg(DMA, SIRQEN) = 1;
                    SCOPE_DMAreg(DMA, AIRQ) = IRQ_SWINT;
                    SCOPE_DMAreg(DMA, AIRQEN) = 0;
                    SCOPE_DMAreg(DMA, DCNTIF) = 0;
                    SCOPE_DMAreg(DMA, EN) = 1;
                    CPUDOZEbits.IDLEN = 1;
                    SCOPE_GIEsave = GIEL;
                    GIEL = 0;
                    SCOPE_DMAreg(DMA, DCNTIE) = 1;
                    SCOPE_state = SCOPE_waitSampleDone;
                    SCOPE_NCOreg(NCO, CONbits).EN = 1;
                }
            }
            break;

        case SCOPE_waitSampleDone:
            if (SCOPE_DMAreg(DMA, DCNTIF) == 1) {
                SCOPE_DMAreg(DMA, DCNTIF) = 0;
                SCOPE_DMAreg(DMA, DCNTIE) = 0;
                SCOPE_samplingBuf->sampleRate = TMRxToHz(SCOPE_NCOID | TMRHELPER_TMRIS_NCO, _XTAL_FREQ);
                GIEL = SCOPE_GIEsave;
                SCOPE_DMAreg(DMA, EN) = 0;
                SCOPE_state = SCOPE_waitSend;
            }
            break;

        case SCOPE_sendCmdBuffer:
            if (CMD_startSendOutputBuffer()) {
                /* Buffer is scheduled for sending */
                SCOPE_outputBuf = NULL;
                CMD_setNumPayloadBytes((GLOBAL_getOutputMode() == GLOBAL_output_mode_binary) ? GLOBAL_getSampleBufByteSize() : strlen(CMD_getOutputBufferAddress()));
                CMD_setPayloadType(CMD_payload_is_ANALOG_SCOPE);
                SCOPE_state = SCOPE_waitSend;
            }
            break;

        case SCOPE_convert_JSON:
            if (SCOPE_outputBuf) {
                *SCOPE_outputBuf = 0; /* If buffer used for text, make sure it's empty... */
                if (SCOPE_numBuffersToSend == 0xFF) {
                    /* First run */
                    sprintf(SCOPE_outputBuf, "{\"SCOPE\":{\"samplerate\":%f,\"pin\":%d,\"voltage\":[", SCOPE_samplingBuf->sampleRate, SCOPE_samplingBuf->sampledPin);
                    SCOPE_outputBuf = SCOPE_outputBuf + strlen(SCOPE_outputBuf);
                    SCOPE_numBuffersToSend = SCOPE_numSamples;
                } else {
                    sprintf(SCOPE_outputBuf, "%f", SCOPE_samplingBuf->dataRecords[(unsigned) SCOPE_numSamples - SCOPE_numBuffersToSend] * SCOPE_samplingBuf->vrefVoltage / 4095.0);
                    SCOPE_outputBuf = SCOPE_outputBuf + strlen(SCOPE_outputBuf);
                    if (--SCOPE_numBuffersToSend) {
                        *SCOPE_outputBuf++ = ',';
                        *SCOPE_outputBuf = 0;
                    } else {
                        strcat(SCOPE_outputBuf, "]}}");
                        SCOPE_state = SCOPE_sendCmdBuffer;
                    }
                }
            } else {
                /* try to claim output buffer */
                SCOPE_outputBuf = CMD_claimOutputBuffer();
            }
            break;

        case SCOPE_convert_XTERM:
            if (SCOPE_outputBuf) {
                if (SCOPE_numBuffersToSend == 0xFF) {
                    /* First run */
                    *SCOPE_outputBuf = 0; /* Buffer used for text, make sure it's empty... */
                    SCOPE_outputBuf = XTERM_strinsert_spinner(SCOPE_outputBuf);
                    SCOPE_outputBuf = XTERM_strinsert_clientWindowPreamble(SCOPE_outputBuf, (bool) SCOPE_firstAcquisition);
                    float tmrFrequency = SCOPE_samplingBuf->sampleRate;
                    char tmpBuf1[30];
                    char tmpBuf2[30];
                    sprintf(SCOPE_outputBuf, "Horizontal: " ANSI_SETCOLOR("1;35") "%ss/div" ANSI_SETCOLOR("0") " Vertical: " ANSI_SETCOLOR("1;35") "1V/div" ANSI_SETCOLOR("0") " (Maximum bandwidth 66ksps)\r\nSampling FREQ: %ssps\r\nAnalyzer pin: " ANSI_SETCOLOR("1;35") "%d" ANSI_SETCOLOR("0") "\r\n", floatToEng(10.0 / tmrFrequency, tmpBuf1), floatToEng(tmrFrequency, tmpBuf2), PINS_pinDefToPinNum(SCOPE_ADCpin));

                    // number of lines to print
                    SCOPE_numBuffersToSend = SCOPE_ROWNUM;

                    // scale down stored AD data to fit into grid 
                    for (uint8_t i = 0; i < 101; i++) {
                        SCOPE_stream_convert[i] = 40U - (uint8_t) (((uint32_t) SCOPE_samplingBuf->dataRecords[i] * 40) / 4095);
                    }
                }

                // draw the grid with the signal, row by row
                draw_grid_row();

                // keep being in this state until buffer runs out or there are no more lines to generate
                if (!--SCOPE_numBuffersToSend || (strlen(CMD_getOutputBufferAddress()) > (CMD_getOutputBufferSize() - 200))) {
                    SCOPE_state = SCOPE_sendCmdBuffer;
                }

            } else {
                /* try to claim output buffer */
                SCOPE_outputBuf = CMD_claimOutputBuffer();
            }
            break;

        case SCOPE_waitSend:
            if (CMD_isTransmissionComplete()) {
                if (SCOPE_numBuffersToSend) {
                    switch (GLOBAL_getOutputMode()) {
                        case GLOBAL_output_mode_binary:
                            SCOPE_numBuffersToSend = 0;
                            SCOPE_state = SCOPE_sendCmdBuffer;
                            break;
                        case GLOBAL_output_mode_JSON:
                            SCOPE_state = SCOPE_convert_JSON;
                            break;
                        case GLOBAL_output_mode_XTERM:
                            SCOPE_state = SCOPE_convert_XTERM;
                            break;
                        default:
                            SCOPE_state = SCOPE_idle;
                            break;
                    }
                } else {
                    SCOPE_firstAcquisition = 0;
                    if (GLOBAL_acq_mode_continous == GLOBAL_getAcquisitionMode()) {
                        if (GLOBAL_isStopRequested()) {
                            GLOBAL_setStopRequest(false);
                            CMD_refreshFrame();
                            SCOPE_state = SCOPE_idle; /* Stop does this, but better be safe */
                        } else {
                            /* Restart with current parameters */
                            SCOPE_state = SCOPE_init;
                        }
                    } else {
                        CMD_refreshFrame();
                        SCOPE_state = SCOPE_idle; /* Stop does this, but better be safe */
                    }
                }
            }
            break;

        default:
            SCOPE_state = SCOPE_idle;

            break;
    }
}

bool SCOPE_onCommand(const char* cmdLine) {
    if (!strcmp(CMD_getCommand(cmdLine), "SCOPE")) {

        /* The command is for this module */
        SCOPE_stop();
        TS_setExitFunc(NULL);
        TS_setTaskFunc(NULL);

        enum {
            paraccept = 1,
            par1 = paraccept << 1,
            par2 = par1 << 1, //PIN
            par3 = par2 << 1, //FREQ
            par4 = par3 << 1, //NUMSMP
            mandatoryParams = paraccept | par2 | par3 | par4,
        } status = paraccept;
        if (GLOBAL_getOutputMode() == GLOBAL_output_mode_XTERM) {
            SCOPE_numSamples = SCOPE_getMaxNumberOfSamples();
            if (SCOPE_numSamples > (XTERM_userWindowRight - XTERM_userWindowLeft + 1)) {
                SCOPE_numSamples = XTERM_userWindowRight - XTERM_userWindowLeft + 1;
            }
            status |= par4;
        }
        uint8_t paramId;
        char parName[CMD_maxParamNameLength + 1];
        long parValue;
        for (paramId = 1; (status & paraccept) && CMD_getParam(cmdLine, paramId, parName, &parValue) && (paramId <= SCOPE_maxNumParameters); ++paramId) {
            if (!strcmp(parName, "PIN")) {
                SCOPE_ADCpin = NULL;
                if ((parValue > 0) && ((unsigned) parValue <= PINS_getNumPins())) {
                    SCOPE_ADCpin = PINS_pinnumToPinDef((unsigned) parValue);
                    if (SCOPE_ADCpin) {
                        status |= par2;
                    }
                    continue;
                }
            }
            if (!strcmp(parName, "FREQ")) {
                if ((parValue >= 61) && (parValue <= 66667L)) {
                    SCOPE_NCOreg(NCO, INC) = (volatile unsigned short long) (((long double) parValue * 1048576.0) / (1.0 * _XTAL_FREQ));
                    if (!SCOPE_NCOreg(NCO, INC)) {
                        SCOPE_NCOreg(NCO, INC) = 1;
                    }
                    status |= par3;
                    continue;
                }
            }
            if (!strcmp(parName, "NUMSMP")) {
                if ((parValue > 0) && (parValue <= SCOPE_getMaxNumberOfSamples())) {
                    SCOPE_numSamples = (unsigned) parValue;
                    status |= par4;
                    continue;
                }
            }
            /* Unknown parameter */
            status = 0;
        }
        if ((status & mandatoryParams) == mandatoryParams) {
            TS_setExitFunc(SCOPE_onExit);
            TS_setTaskFunc(SCOPE_task);
            SCOPE_state = SCOPE_init;
            SCOPE_firstAcquisition = 1;
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

static void draw_grid_row() {
    uint8_t rowNum = SCOPE_ROWNUM - SCOPE_numBuffersToSend; // print a row each iteration
    uint8_t colNum;

    // set string pointer to end of string
    SCOPE_outputBuf += strlen(SCOPE_outputBuf);

    // position cursor at first column and current row
    sprintf(SCOPE_outputBuf, ANSI_CSI "%d;1H", rowNum + XTERM_SCOPE_firstRow);

    // set string pointer to end of string
    SCOPE_outputBuf = SCOPE_outputBuf + strlen(SCOPE_outputBuf);

    // write a line
    for (colNum = 0; colNum < XTERM_screenWidth; ++colNum) {

        if (!(rowNum % 4)) {
            // a horizontal line
            if (!(colNum % 10)) {
                // a horizontal line crossed by a vertical line
                if (rowNum == 0) {
                    // top border
                    if (colNum == 0) {
                        // upper left corner
                        *SCOPE_outputBuf = 0xda;
                    } else if (colNum == XTERM_screenWidth - 1) {
                        // upper right corner
                        *SCOPE_outputBuf = 0xbf;
                    } else {
                        // Tee somewhere in the middle
                        *SCOPE_outputBuf = 0xc2;
                    }

                } else if (rowNum == SCOPE_ROWNUM - 1) {
                    // bottom border
                    if (colNum == 0) {
                        // lower left corner
                        *SCOPE_outputBuf = 0xc0;
                    } else if (colNum == XTERM_screenWidth - 1) {
                        // lower right corner
                        *SCOPE_outputBuf = 0xd9;
                    } else {
                        // inv. Tee
                        *SCOPE_outputBuf = 0xc1;
                    }
                } else {
                    // a row somewhere in the middle
                    if (colNum == 0) {
                        // left border
                        *SCOPE_outputBuf = 0xc3;
                    } else if (colNum == XTERM_screenWidth - 1) {
                        // right border
                        *SCOPE_outputBuf = 0xb4;
                    } else {
                        // line crossing somewhere in the middle
                        *SCOPE_outputBuf = 0xc5;
                    }
                }
            } else {
                // no crossing, just a horizontal bar
                *SCOPE_outputBuf = 0xc4;
            }
        } else {
            if (!(colNum % 10)) {
                // no crossing, just a vertical bar 
                *SCOPE_outputBuf = 0xb3;
            } else {
                // space in between
                *SCOPE_outputBuf = ' ';
            }
        }

        // draw the signal into the grid
        if (SCOPE_stream_convert[colNum] >> 1 == rowNum) {
            // current grid position is a data point, overwrite grid char
            if (SCOPE_stream_convert[colNum] & 1) {
                // upper bar
                *SCOPE_outputBuf = 0xdc;
            } else {
                // lower bar
                *SCOPE_outputBuf = 0xdf;
            }
        }

        // char determined, move to the next char
        SCOPE_outputBuf++;
    }

    // put a EOS marker at end of string
    *SCOPE_outputBuf = 0;
}