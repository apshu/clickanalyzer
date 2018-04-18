#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "hwresources.h"
#ifdef __XC8
#include "mcc_generated_files/mcc.h"
#endif
#include "cmdline.h"
#include "hwresources.h"
#include "global_settings.h"
#include "xterm.h"
#include "cmd_modules/reset_all.h"
#include "mouse.h"

static uint8_t CMD_inBuf[CMD_inBufferSize];
static uint8_t CMD_newCommand = false;
static uint8_t CMD_inANSIBuf[CMD_inANSIbufferSize];
static uint8_t CMD_lastInChar;
static bit CMD_isOutbufLocked;

static struct {
    CMD_header_t header;
    uint8_t payload[CMD_outBufferSize];
} CMD_outBuffer;

#define CMD_setDMABytesToSend(numBytes) do { CMD_DMAreg(DMA, SSZ) = (numBytes); } while(0)
#define CMD_getDMABytesToSend() ( CMD_DMAreg(DMA, SSZ) )
#define CMD_setDMAsourceAddress(outAddress) do { CMD_DMAreg(DMA, SSA) = (volatile unsigned short long) (outAddress); } while(0) /* Source address */
#define CMD_isUARTreradyForNewTXDMAcycle() ((CMD_DMAreg(DMA, SCNTIF) || (!CMD_DMAreg(DMA, EN)))&& CMD_UARTreg(U,TXBE))

static enum {
    CMD_i_init,
    CMD_i_startNewCommand,
    CMD_i_waitChar,
    CMD_i_gotChar,
    CMD_i_gotEOS,
    CMD_i_gotANSIchar,
    CMD_i_processANSIsequence,
} CMD_stateIn = CMD_i_init;

static enum {
    CMD_o_greeting,
    CMD_o_idle,
    CMD_o_binBufSend,
    CMD_o_waitCRCcomplete,
    CMD_o_textBufSend,
    CMD_o_sendBufInit,
    CMD_o_waitTXcomplete,
    CMD_o_drawStaticXTERMframe,
    CMD_o_drawDynamicXTERMframe,
    CMD_o_abortTX,
} CMD_stateOut = CMD_o_greeting;

static void inline CMD_lockOutputBuffer(bool isLock) {
    CMD_isOutbufLocked = isLock;
}

void inline CMD_unlockOutputBuffer(void) {
    CMD_lockOutputBuffer(false);
}

static bit inline CMD_isOutputBufferLocked(void) {
    return (CMD_isOutbufLocked);
}

void inline CMD_setNumPayloadBytes(size_t numPayloadBytes) {
    CMD_outBuffer.header.numPayloadBytes = numPayloadBytes;
}

size_t inline CMD_getNumPayloadBytes(void) {
    return ((size_t) CMD_outBuffer.header.numPayloadBytes);
}

void inline CMD_setPayloadType(CMD_payloadTypes_t payloadType) {
    CMD_outBuffer.header.payloadCode = payloadType;
}

static inline bool CMD_UART_isNewChar(void) {
#if 0
    return CMD_UARTreg(U, RXIF);
#else
    bool UART1_isNewChar(void);
    return CMD_UARTreg(UART, _isNewChar)();
#endif
}

static inline uint8_t CMD_UART_getc(void) {
#if 0
    CMD_UARTreg(U, CON0bits).RXEN = 1;
    if (1 == CMD_UARTreg(U, ERRIRbits).FERIF) {
        // UART1 error - restart
        CMD_UARTreg(U, CON1bits).ON = 0;
        CMD_UARTreg(U, CON1bits).ON = 1;
    }
    return CMD_UARTreg(U, RXB);
#else
    return CMD_UARTreg(UART, _Read)();
#endif
}

static bit CMD_readingANSIsequence;

static inline void CMD_ansiSequenceStarted(bool isEnabled) {
    if (isEnabled) {
        *CMD_inANSIBuf = 0;
    }
    CMD_readingANSIsequence = isEnabled;
}

static inline bit CMD_isAnsiSequenceActive(void) {
    return CMD_readingANSIsequence;
}

static void CMD_initialize(void) {
    INTCON0bits.GIE = 1;
    INTCON0bits.GIEL = 1;
    CMD_UARTreg(UART, _Initialize)();
    CMD_ansiSequenceStarted(0);
    CMD_stateOut = CMD_o_greeting;
    CMD_stateIn = CMD_i_startNewCommand;
}

static bit CMD_frameRefreshNeeded;

inline void CMD_refreshFrame(void) {
    CMD_frameRefreshNeeded = 1;
}

static inline void CMD_completeFrameRefresh(void) {
    CMD_frameRefreshNeeded = 0;
}

static inline bit CMD_isFrameRefreshNeeded(void) {
    return CMD_frameRefreshNeeded && (GLOBAL_getOutputMode() == GLOBAL_output_mode_XTERM);
}

void CMD_startSingleCommand(const char* commandline, char queChar) {
    if (commandline) {
        CMD_lastInChar = queChar;
        strcpy(CMD_inBuf, commandline);
        return;
    }
    CMD_lastInChar = 0;
}

void CMD_taskIn(void) {
    /* RX state machine */
    switch (CMD_stateIn) {
        case CMD_i_init:
            CMD_initialize();
            break;

        case CMD_i_startNewCommand:
            memset(CMD_inBuf, 0, sizeof (CMD_inBuf));
            CMD_newCommand = false;
            CMD_stateIn = CMD_i_waitChar;
            CMD_refreshFrame();
            break;

        case CMD_i_waitChar:
            CMD_newCommand = false;
            if (CMD_UART_isNewChar() || CMD_lastInChar) {
                if (!CMD_lastInChar) {
                    CMD_lastInChar = CMD_UART_getc();
                }
                if (RST_onNewChar(CMD_lastInChar)) {
                    CMD_stateIn = CMD_isAnsiSequenceActive() ? CMD_i_gotANSIchar : CMD_i_gotChar;
                    if (CMD_lastInChar == 27) { /* Move to ANSI mode after ESC character received, discard the character */
                        CMD_ansiSequenceStarted(1);
                        CMD_stateIn = CMD_i_waitChar;
                        CMD_lastInChar = 0;
                    }
                } else {
                    CMD_lastInChar = 0;
                }
            }
            break;
        case CMD_i_gotChar:
            if (CMD_lastInChar) {
                uint8_t inBufLen = strlen(CMD_inBuf);
                if (GLOBAL_getOutputMode() == GLOBAL_output_mode_XTERM) {
                    /* Some extra input conversions for XTERM mode */
                    if ((CMD_lastInChar == 10) || (CMD_lastInChar == 13)) { /* Treat Enter as END of String */
                        CMD_lastInChar = CMD_char_EOS;
                    } else {
                        if ((CMD_lastInChar == 127) || (CMD_lastInChar == 8)) { /* Backspace */
                            CMD_lastInChar = 0;
                            if (inBufLen) {
                                --inBufLen;
                            }
                        } else {
                            if ((CMD_lastInChar == 9) || ((CMD_lastInChar & 0x80) > 0)) {
                                /* TABs and special codes eliminated */
                                CMD_lastInChar = 0;
                                break;
                            }
                        }
                    }
                    CMD_lastInChar = toupper(CMD_lastInChar); /* Convert characters to upper case for simplicity */
                }
                if (CMD_lastInChar == CMD_char_EOS) {
                    CMD_lastInChar = 0;
                    CMD_stateIn = CMD_i_gotEOS;
                } else {
                    CMD_stateIn = CMD_i_waitChar;
                }
                if (inBufLen < sizeof (CMD_inBuf) - 2) {
                    CMD_inBuf[inBufLen++] = CMD_lastInChar;
                    CMD_refreshFrame();
                }
                CMD_lastInChar = 0;
                CMD_inBuf[inBufLen] = 0; /* Add string terminator */
            } else {
                /* When incoming char is invalid, wait for another char */
                CMD_stateIn = CMD_i_waitChar;
            }
            break;
        case CMD_i_gotANSIchar:
        {
            /* Append byte to ANSI buffer */
            uint8_t inBufLen = strlen(CMD_inANSIBuf);
            if (inBufLen < sizeof (CMD_inANSIBuf) - 1) {
                CMD_inANSIBuf[inBufLen++] = CMD_lastInChar;
            }
            CMD_inANSIBuf[inBufLen] = 0; /* Add string terminator */
            CMD_stateIn = (isalpha(CMD_lastInChar) || (CMD_lastInChar == '~')) ? CMD_i_processANSIsequence : CMD_i_waitChar;
            CMD_lastInChar = 0;
        }
            break;
        case CMD_i_processANSIsequence:
            if (XTERM_executeANSIsequence(CMD_inANSIBuf)) {
                CMD_ansiSequenceStarted(false);
                CMD_stateIn = CMD_i_waitChar;
            }
            break;
        case CMD_i_gotEOS:
            if (CMD_isTransmissionComplete()) {
                uint8_t inLen = CMD_newCommand = strlen(CMD_inBuf);
                if (CMD_newCommand) {
                    uint8_t* s = CMD_inBuf;
                    const char toRemove[3] = {CMD_char_SEPARATOR, CMD_char_SEPARATOR, 0};
                    while (s = strstr(s, toRemove)) { /* Remove all double (or more) command separators */
                        memmove(s, s + strlen(toRemove), 1 + strlen(s + strlen(toRemove)));
                    }
                    inLen = CMD_newCommand = strlen(CMD_inBuf);
                    s = CMD_inBuf;
                    while (*s) {
                        if (*s == CMD_char_SEPARATOR) {
                            *s = 0;
                        }
                        ++s;
                    }
                }
                CMD_inBuf[inLen + 1] = 0; /* Add string list terminator */
                CMD_stateIn = CMD_i_startNewCommand;
            }
            break;
        default:
            CMD_stateIn = CMD_i_init;
            break;
    }
}

/* Standard values for CMD_startSendDMA con1Value*/
#define CMD_DMA_fromRAM (0x03)
#define CMD_DMA_fromFLASH (CMD_DMA_fromRAM | (1 << _DMA1CON1_SMR_POSITION))

static inline void CMD_startSendDMA(uint8_t CON1value) {
    CMD_DMAreg(DMA, CON0) = 0x40; //set control register0 and disable DMA
    /* Comm buffer prepared in RAM */
    CMD_DMAreg(DMA, CON1) = CON1value; //set control register1 
    CMD_DMAreg(DMA, SIRQ) = CMD_UARTreg(IRQ_U, TX); //set DMA Transfer Trigger Source = UART TX
    CMD_DMAreg(DMA, DSA) = (volatile unsigned short) &CMD_UARTreg(U, TXB); /* Destination address */
    CMD_DMAreg(DMA, DSZ) = 1; /* Destination length */
    /* Source address already defined by call to CMD_setDMAsourceAddress() */
    /* Source length already defined by call to CMD_setNumBytesToSend() */
    CMD_DMAreg(DMA, SCNTIF) = 0;
    CMD_UARTreg(U, TXEN) = 1;
    CMD_DMAreg(DMA, DGO) = 1;
    CMD_DMAreg(DMA, EN) = 1;
    CMD_UARTreg(U, TXIF) = 0;
}

void CMD_taskOut(void) {
    /* TX state machine */
    switch (CMD_stateOut) {
        case CMD_o_greeting:
            if (CMD_stateIn != CMD_i_init) {
                /* Wait until init completes */
#if defined( CIRCUITBOARD ) && ( CIRCUITBOARD > 0 )
#if (CIRCUITBOARD == PCB_gumstick_alt_UART) || (CIRCUITBOARD == PCB_clickboard)
                strcpy(CMD_outBuffer.payload, "{\"commandline\":{\"separator_commands\":\"$\"}}\033[5n"); /* Greeting + Terminal client autodetect */
#elif (CIRCUITBOARD == PCB_gumstick)
                strcpy(CMD_outBuffer.payload, "{\"commandline\":{\"separator_commands\":\"$\"}}"); /* Greeting */
                static const char XTERM_command_startXTERMmode[] = {'S', 'E', 'T', CMD_char_SEPARATOR, 'O', 'U', 'T', 'P', 'U', 'T', CMD_char_SEPARATOR, 'X', 'T', 'E', 'R', 'M', 0};
                CMD_startSingleCommand(XTERM_command_startXTERMmode, CMD_char_EOS);
                CMD_stateIn = CMD_i_waitChar;
#else
#error "Unknown board type"
#endif
#else
#error "Please define circuit board type and/or include hwresources.h"
#endif                      
                *strchr(CMD_outBuffer.payload, '$') = CMD_char_EOS;
                CMD_setNumPayloadBytes(strlen(CMD_outBuffer.payload));
                CMD_stateOut = CMD_o_textBufSend;
            }
            break;

        case CMD_o_idle:
            if (CMD_isFrameRefreshNeeded()) {
                CMD_stateOut = CMD_o_drawStaticXTERMframe;
            }
            break;

        case CMD_o_binBufSend:
            if (CMD_isUARTreradyForNewTXDMAcycle()) {
                CMD_UARTreg(U, TXEN) = 0;
                CRC_Initialize();
                CMD_DMAreg(DMA, CON0) = 0x40; //set control register0 and disable DMA
                CMD_DMAreg(DMA, CON1) = 0x03; //set control register1 
                CMD_DMAreg(DMA, SIRQ) = IRQ_CRC; //set DMA Transfer Trigger Source = CRC
                CMD_DMAreg(DMA, DSA) = (volatile unsigned short) &CRCDATL; /* Destination address */
                CMD_DMAreg(DMA, DSZ) = 1; /* Destination length */
                CMD_setDMAsourceAddress(((uint8_t*) & CMD_outBuffer.header.crc16) + sizeof (CMD_outBuffer.header.crc16)); /* Source address */
                CMD_setDMABytesToSend(CMD_getNumPayloadBytes() + sizeof (CMD_outBuffer.header) - sizeof (CMD_outBuffer.header.crc16)); /* Source length */
                CRC_Start();
                CMD_DMAreg(DMA, SCNTIF) = 0;
                CMD_DMAreg(DMA, DGO) = 1;
                CMD_DMAreg(DMA, EN) = 1;
                CMD_stateOut = CMD_o_waitCRCcomplete;
            }
            break;

        case CMD_o_waitCRCcomplete:
            if (CMD_DMAreg(DMA, SCNTIF)) {
                /* DMA completed */
                CMD_DMAreg(DMA, CON0) = 0x40; //set control register0 and disable DMA
                if (!CRC_IsBusy()) {
                    /* CRC calculation completed */
                    if ((CRCACCL == '{') || (CRCACCL == 27)) { /* if CRC value confuse JSON or ANSI auto-detect, inject an extra zero  */
                        CRCDATL = 0;
                        break;
                    }
                    /* CRC value compatible with auto-detect */
                    CRCGO = 0;
                    CRCIF = 0;
                    CMD_outBuffer.header.crc16 = CRCACC;
                    CRCEN = 0;
                    /* Comm buffer prepared in RAM */
                    /* Go to start TX DMA state */
                    CMD_setDMABytesToSend(CMD_getNumPayloadBytes() + sizeof (CMD_header_t)); /* Source length */
                    CMD_setDMAsourceAddress(&CMD_outBuffer); /* Source address */
                    CMD_stateOut = CMD_o_sendBufInit;
                }
            }
            break;

        case CMD_o_textBufSend:
            if (CMD_isUARTreradyForNewTXDMAcycle()) {
                CMD_setDMABytesToSend(CMD_getNumPayloadBytes()); /* Source length */
                CMD_setDMAsourceAddress(CMD_outBuffer.payload); /* Source address */
                CMD_stateOut = CMD_o_sendBufInit;
            }
            break;

        case CMD_o_sendBufInit:
            /* Check if there is data to send */
            if (CMD_getDMABytesToSend()) {
                CMD_startSendDMA(CMD_DMA_fromRAM);
                if (GLOBAL_getOutputMode() == GLOBAL_output_mode_XTERM) {
                    /* Draw frame and other screen objects */
                    CMD_stateOut = CMD_isFrameRefreshNeeded() ? CMD_o_drawStaticXTERMframe : CMD_o_waitTXcomplete;
                } else {
                    /* Go to wait TX DMA complete state */
                    CMD_stateOut = CMD_o_waitTXcomplete;
                }
            } else {
                *CMD_outBuffer.payload = 0; /* Make output buffer a zero length string for safer operation */
                CMD_setNumPayloadBytes(0);
                CMD_stateOut = CMD_o_idle;
            }
            break;

        case CMD_o_waitTXcomplete:
            if (CMD_isUARTreradyForNewTXDMAcycle()) {
                if (CMD_UARTreg(U, TXMTIF)) {
                    CMD_DMAreg(DMA, EN) = 0;
                    *CMD_outBuffer.payload = 0; /* Make output buffer a zero length string for safer operation */
                    CMD_setNumPayloadBytes(0);
                    CMD_lockOutputBuffer(false);
                    CMD_stateOut = CMD_o_idle;
                }
            }
            break;

        case CMD_o_drawStaticXTERMframe:
            if (CMD_isUARTreradyForNewTXDMAcycle()) {
                /* Waveform display now finished */
                CMD_setDMABytesToSend(XTERM_getStaticFrameLength()); /* Source length */
                CMD_setDMAsourceAddress(XTERM_getStaticFrame()); /* Source address */
                CMD_startSendDMA(CMD_DMA_fromFLASH);
                CMD_stateOut = CMD_o_drawDynamicXTERMframe;
            }
            break;

        case CMD_o_drawDynamicXTERMframe:
            if (CMD_isUARTreradyForNewTXDMAcycle()) {
                /* create status information */
                /* Dynamic frame to CMD_outBuffer.payload  */
                CMD_setNumPayloadBytes(XTERM_renderDynamicFrame(CMD_outBuffer.payload, sizeof (CMD_outBuffer.payload), CMD_inBuf));
                if (CMD_getNumPayloadBytes()) {
                    CMD_setDMABytesToSend(CMD_getNumPayloadBytes()); /* Source length */
                    CMD_setDMAsourceAddress(CMD_outBuffer.payload); /* Source address */
                    CMD_startSendDMA(CMD_DMA_fromRAM);
                }
                CMD_completeFrameRefresh();
                CMD_stateOut = CMD_o_waitTXcomplete;
            }
            break;

        case CMD_o_abortTX:
            CMD_DMAreg(DMA, EN) = 0;
            if (CMD_UARTreg(U, TXMTIF)) {
                /* Transmission totally stopped */
                CMD_lockOutputBuffer(false);
                CMD_stateOut = CMD_o_idle;
            }
            break;

        default:
            *CMD_outBuffer.payload = 0; /* Make output buffer a zero length string for safer operation */
            CMD_setNumPayloadBytes(0);
            CMD_stateOut = CMD_o_idle;
            break;
    }
}

bool CMD_isNewCommand(void) {
    return CMD_newCommand;
}

const char* CMD_getCommand(const char* commandPrompt) {
    return (const char*) commandPrompt;
}

const char* CMD_getCommandPrompt(void) {

    return CMD_isNewCommand() ? (const char*) CMD_inBuf : NULL;
}

/* commandPrompt: input, stringlist containing the full command line 
 paramNum: input, number of parameter to query ( 1 is first parameter)
 name: output, parameter string
 value: output, parameter numeric value
 returns true if number of parameter exists
 */


bool CMD_getParam(const char* commandPrompt, uint8_t paramNum, char* name, long* value) {
    if (commandPrompt && paramNum) {
        for (; paramNum--;) {
            commandPrompt = commandPrompt + strlen(commandPrompt) + 1;
            if (!*commandPrompt) {
                return false;
            }
        }
        if (name) {
            while (*commandPrompt && (*commandPrompt != CMD_char_ASSIGNMENT)) {
                if (strlen(name) < CMD_maxParamNameLength) {
                    *name++ = *commandPrompt;
                }
                ++commandPrompt;
            }
            *name = 0;
            if (value) {
                char* mantisa;
                *value = strtol(++commandPrompt, &mantisa, 0);
                if ('M' == *mantisa) {
                    *value *= 1000000L;
                } else {
                    if ('K' == toupper(*mantisa)) {
                        *value *= 1000L;
                    }
                }
            }
        }
        return true;
    }
    return false;
}

uint8_t* CMD_getOutputBufferAddress(void) {
    return (CMD_outBuffer.payload);
}

uint8_t* CMD_claimOutputBuffer(void) {
    if (!CMD_isOutputBufferLocked()) {
        if (CMD_isTransmissionComplete()) {
            CMD_lockOutputBuffer(true);
            return (CMD_getOutputBufferAddress());
        }
    }
    return NULL;
}

size_t CMD_getOutputBufferSize(void) {
    return (sizeof (CMD_outBuffer.payload));
}

bool CMD_startSendOutputBuffer(void) {
    if (CMD_isTransmissionComplete()) {
        CMD_stateOut = (GLOBAL_getOutputMode() == GLOBAL_output_mode_binary) ? CMD_o_binBufSend : CMD_o_textBufSend;
        return true;
    }
    return false;
}

bool CMD_isTransmissionComplete(void) {
    return (CMD_stateOut == CMD_o_idle);
}

static const char CMD_NAK_commandSequenceJSON[] = {'{', '"', 'l', 'a', 's', 't', '_', 'r', 'e', 's', 'p', 'o', 'n', 's', 'e', '"', ':', '"', CMD_char_NAK, '"', '}'};

bool CMD_sendNAK(void) {
    if (GLOBAL_getOutputMode() == GLOBAL_output_mode_XTERM) {
        XTERM_showNAK(true);
        return true;
    }
    uint8_t i;
    for (i = 0; i < 100; ++i) { /* Maximum 100ms delay */
        void* outBuf = CMD_claimOutputBuffer();
        if (outBuf) {
            CMD_setPayloadType(CMD_payload_is_NAK);
            switch (GLOBAL_getOutputMode()) {
                case GLOBAL_output_mode_binary:
                    CMD_setNumPayloadBytes(0);
                    break;

                case GLOBAL_output_mode_JSON:
                    CMD_setNumPayloadBytes(sizeof (CMD_NAK_commandSequenceJSON));
                    memmove(outBuf, CMD_NAK_commandSequenceJSON, CMD_getNumPayloadBytes());
                    break;

                default:
                    /* Unknown modes doesn't output NAK */
                    return false;
            }
            return CMD_startSendOutputBuffer();
        } else {
            __delay_ms(1); /* Wait until tx is done */
            CMD_lockOutputBuffer(false); /* Force unlocking of buffer, needed if task is stuck with locked buffer */
            CMD_taskOut(); /* Advance state machine */
        }
    }
    return false;
}

void CMD_abortSendOutputBuffer(void) {
    CMD_stateOut = CMD_o_abortTX;
}

void CMD_onMouse(uint16_t mouseX, uint16_t mouseY, uint16_t mouseEvent) {
    if ((mouseX == XTERM_helpIconLeft) && (mouseY == XTERM_helpIconTopRow) && (mouseEvent == (MOUSE_EVT_BTN_left | MOUSE_EVT_release))) {
        CMD_startSingleCommand("COMMANDS", CMD_char_EOS);
    }
}