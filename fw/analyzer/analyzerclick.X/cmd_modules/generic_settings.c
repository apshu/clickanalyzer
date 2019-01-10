#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "../hwresources.h"
#include "../tasks.h"
#include "../cmdline.h"
#include "generic_settings.h"
#include "logic_scope.h"
#include "scope.h"
#include "../global_settings.h"
#include "../string_helpers.h"
#include "../xterm.h"

/* Disable warning: (336) string concatenation across lines */
#pragma warning disable 336

#define VERSION_HW      "1.0"
#define VERSION_FW      "2.00"
#define VERSION_COMM    "1.10"

static uint8_t* CMDGET_outBuf = NULL;
static const char CMDGET_led_options[] = {
    "{"
    "\"type\":\"led\","
    "\"optional\":true,"
    "\"values\":{"
    "\"range\":[0,14,0,14]"
    "}"
    "}"
};
static const char CMDGET_get_options[] = {
    "{"
    "\"type\":null,"
    "\"optional\":true,"
    "\"values\":{}"
    "}"
};
static const char CMDGET_NCO1_options[] =
        "\"FREQ\":{"
        "\"type\":\"samplerate\","
        "\"optional\":false,"
        "\"values\":{"
        "\"range\":[61,4000000,61,4000000]"
        "}";

bool CMDGET_onCommand(const char* cmdLine) {
    if (!strcmp(CMD_getCommand(cmdLine), "COMMANDS")) {
        /* The command is for this module */
        TS_setTaskFunc(NULL);
        TS_setExitFunc(NULL);
        CMDGET_outBuf = CMD_claimOutputBuffer();
        if (CMDGET_outBuf && !CMD_getParam(cmdLine, 1, NULL, NULL)) {
            /* We can output to serial port and no parameters */
            CMD_setPayloadType(CMD_payload_is_GET_RESPONSE);
            *CMDGET_outBuf = 0; /* Zero terminate string */
            char* CMDGET_replyBuf = XTERM_strinsert_clientScrollingWindowPreamble(CMDGET_outBuf, true);
            strcpy(CMDGET_replyBuf,
                    "{"
                    "\"commandline\":{"
                    "\"separator_commands\":\"$\"," //$ is template placeholder
                    "\"separator_parameters\":\"$\"," //$ is template placeholder
                    "\"assign_number\":\"$\"" //$ is template placeholder
                    "},"
                    "\"commands\":{"
                    "\"COMMANDS\":{\"details\":\"GET$COMMANDS_INFO\"},"
                    "\"GOTOBOOTLOADER\":{\"details\":\"GET$BLDR_INFO\"},"
                    "\"LS\":{\"details\":\"GET$LS_INFO\"},"
                    "\"SCOPE\":{\"details\":\"GET$SCOPE_INFO\"},"
                    "\"LED\":{\"details\":\"GET$LED_INFO\"},"
                    "\"DVM\":{\"details\":\"GET$DVM_INFO\"},"
                    "\"GET\":{\"details\":\"GET$GET_INFO\"},"
                    "\"SET\":{\"details\":\"GET$SET_INFO\"}"
                    "}"
                    "}"
                    );
            *strchr(CMDGET_replyBuf, '$') = CMD_char_EOS;
            *strchr(CMDGET_replyBuf, '$') = CMD_char_SEPARATOR;
            *strchr(CMDGET_replyBuf, '$') = CMD_char_ASSIGNMENT;
            strReplaceAllChar(CMDGET_replyBuf, '$', CMD_char_SEPARATOR);
            CMD_setNumPayloadBytes(strlen(CMDGET_outBuf));
            if (CMD_getNumPayloadBytes()) {
                if (CMD_startSendOutputBuffer()) {
                    return true;
                }
            }
        }
        /* Unknown or missing parameter */
        CMD_sendNAK();
        /* Command processed */
        return true;
    }

    if (!strcmp(CMD_getCommand(cmdLine), "GET")) {
        /* The command is for this module */
        TS_setTaskFunc(NULL);
        TS_setExitFunc(NULL);
        CMDGET_outBuf = CMD_claimOutputBuffer();
        if (CMDGET_outBuf && !CMD_getParam(cmdLine, 2, NULL, NULL)) {
            /* We can output to serial port and no second parameter */
            CMD_setPayloadType(CMD_payload_is_GET_RESPONSE);
            *CMDGET_outBuf = 0; /* Zero terminate string */
            char* CMDGET_replyBuf = XTERM_strinsert_clientScrollingWindowPreamble(CMDGET_outBuf, true);
            char parName[CMD_maxParamNameLength + 1];
            if (CMD_getParam(cmdLine, 1, parName, NULL)) {
                if (!strcmp(parName, "PRODUCT")) {
                    strcpy(CMDGET_replyBuf,
                            "{"
                            "\"product\":{"
                            "\"name\":\"PIC18F26K42 Click Analyzer\","
                            "\"version\":{"
                            "\"HW\":\"" VERSION_HW "\","
                            "\"FW\":\"" VERSION_FW "\","
                            "\"COMM\":\"" VERSION_COMM "\"},"
                            "\"serialID\":\""
                            );
                    getSerialnumberString(CMDGET_replyBuf + strlen(CMDGET_replyBuf));
                    strcat(CMDGET_replyBuf,
                            "\"}"
                            "}"
                            );
                }
                if (!strcmp(parName, "LS_INFO")) {
                    sprintf(CMDGET_replyBuf,
                            "{"
                            "\"commands\":{"
                            "\"LS\":{"
                            "\"description\":\"Logic scope\","
                            "\"clock\":64000000,"
                            "\"bytesPerSample\":%d,"
                            "\"parameters\":{"
                            "\"NUMSMP\":{"
                            "\"type\":\"numSamples\","
                            "\"optional\":false,"
                            "\"values\":{"
                            "\"range\":[1,%d,1,%d]"
                            "}"
                            "},"
                            "%s"
                            "}"
                            "}"
                            "}"
                            "}",
                            sizeof (((LS_payloadBuf_t*) 0)->dataRecords[0]),
                            LS_getMaxNumberOfSamples(),
                            LS_getMaxNumberOfSamples(),
                            CMDGET_NCO1_options);
                }
                if (!strcmp(parName, "SCOPE_INFO")) {
                    sprintf(CMDGET_replyBuf,
                            "{"
                            "\"commands\":{"
                            "\"SCOPE\":{"
                            "\"description\":\"Analog scope\","
                            "\"maxResolutionBits\":12,"
                            "\"bytesPerSample\":%d,"
                            "\"parameters\":{"
                            "\"NUMSMP\":{"
                            "\"type\":\"numSamples\","
                            "\"optional\":false,"
                            "\"values\":{"
                            "\"range\":[1,%d,1,%d]"
                            "}"
                            "},"
                            "\"PIN\":{"
                            "\"type\":\"analog\","
                            "\"optional\":false,"
                            "\"values\":{"
                            "\"range\":[1,14,1,14]"
                            "}"
                            "},"
                            "\"FREQ\":{"
                            "\"type\":\"samplerate\","
                            "\"optional\":false,"
                            "\"values\":{"
                            "\"range\":[61,66667,61,66667]"
                            "}"
                            "}"
                            "}"
                            "}"
                            "}"
                            "}",
                            sizeof (((LS_payloadBuf_t*) 0)->dataRecords[0]),
                            SCOPE_getMaxNumberOfSamples(),
                            SCOPE_getMaxNumberOfSamples());
                }
                if (!strcmp(parName, "LED_INFO")) {
                    sprintf(CMDGET_replyBuf,
                            "{"
                            "\"commands\":{"
                            "\"LED\":{"
                            "\"description\":\"Pin changed notification LEDs\","
                            "\"parameters\":{"
                            "\"RED\":%s,"
                            "\"GREEN\":%s,"
                            "\"YELLOW\":%s,"
                            "\"ORANGE\":%s"
                            "}"
                            "}"
                            "}"
                            "}",
                            CMDGET_led_options,
                            CMDGET_led_options,
                            CMDGET_led_options,
                            CMDGET_led_options
                            );
                }
                if (!strcmp(parName, "GET_INFO")) {
                    sprintf(CMDGET_replyBuf,
                            "{"
                            "\"commands\":{"
                            "\"GET\":{"
                            "\"description\":\"Get setting(s)\","
                            "\"parameters\":{"
                            "\"PRODUCT\":%s,"
                            "\"COMMANDS_INFO\":%s,"
                            "\"GET_INFO\":%s,"
                            "\"SET_INFO\":%s,"
                            "\"LED_INFO\":%s,"
                            "\"LS_INFO\":%s,"
                            "\"SCOPE_INFO\":%s"
                            "}"
                            "}"
                            "}"
                            "}",
                            CMDGET_get_options,
                            CMDGET_get_options,
                            CMDGET_get_options,
                            CMDGET_get_options,
                            CMDGET_get_options,
                            CMDGET_get_options,
                            CMDGET_get_options
                            );
                }
                if (!strcmp(parName, "SET_INFO")) {
                    sprintf(CMDGET_replyBuf,
                            "{"
                            "\"commands\":{"
                            "\"SET\":{"
                            "\"description\":\"Set setting(s)\","
                            "\"parameters\":{"
                            "\"OUTPUT%cBIN\":%s,"
                            "\"OUTPUT%cJSON\":%s,"
                            "\"OUTPUT%cANSI\":%s,"
                            "\"OUTPUT%cXTERM\":%s",
                            CMD_char_SEPARATOR,
                            CMDGET_get_options,
                            CMD_char_SEPARATOR,
                            CMDGET_get_options,
                            CMD_char_SEPARATOR,
                            CMDGET_get_options,
                            CMD_char_SEPARATOR,
                            CMDGET_get_options
                            );
                    CMDGET_replyBuf = CMDGET_replyBuf + strlen(CMDGET_replyBuf);
                    sprintf(CMDGET_replyBuf,
                            "\"REPEAT\":%s"
                            "\"NOREPEAT\":%s"
                            "}"
                            "}"
                            "}"
                            "}",
                            CMDGET_get_options,
                            CMDGET_get_options
                            );
                }
                if (!strcmp(parName, "COMMANDS_INFO")) {
                    strcpy(CMDGET_replyBuf,
                            "{"
                            "\"commands\":{"
                            "\"COMMANDS\":{"
                            "\"description\":\"List all commands\","
                            "\"parameters\":{}"
                            "}"
                            "}"
                            "}");
                }
                if (!strcmp(parName, "DVM_INFO")) {
                    strcpy(CMDGET_replyBuf,
                            "{"
                            "\"commands\":{"
                            "\"DVM\":{"
                            "\"description\":\"Digital Voltmeter\","
                            "\"parameters\":{}"
                            "}"
                            "}"
                            "}");
                }
                if (!strcmp(parName, "BLDR_INFO")) {
                    strcpy(CMDGET_replyBuf,
                            "{"
                            "\"commands\":{"
                            "\"GOTOBOOTLOADER\":{"
                            "\"description\":\"Start bootloader session\","
                            "\"parameters\":{}"
                            "}"
                            "}"
                            "}");
                }
                CMD_setNumPayloadBytes(strlen(CMDGET_outBuf));
                if (CMD_getNumPayloadBytes()) {
                    if (CMD_startSendOutputBuffer()) {
                        return true;
                    }
                }
            }
        }
        /* Unknown or missing parameter */
        CMD_sendNAK();
        /* Command processed */
        return true;
    }
    /* Command not recognized */
    return false;
}

bool CMDSET_onCommand(const char* cmdLine) {
    if (!strcmp(CMD_getCommand(cmdLine), "SET")) {
        /* The command is for this module */
        TS_setTaskFunc(NULL);
        TS_setExitFunc(NULL);
        char parName[CMD_maxParamNameLength + 1];
        if (CMD_getParam(cmdLine, 1, parName, NULL)) {
            if (!strcmp(parName, "OUTPUT")) {
                if (CMD_getParam(cmdLine, 2, parName, NULL)) {
                    GLOBAL_output_mode_t newMode = GLOBAL_output_mode_INVALID;
                    if (!strcmp(parName, "BIN")) {
                        newMode = GLOBAL_output_mode_binary;
                    } else {
                        if (!strcmp(parName, "JSON")) {
                            newMode = GLOBAL_output_mode_JSON;
                        } else {
                            if (!strcmp(parName, "ANSI") || !strcmp(parName, "XTERM")) {
                                CMDGET_outBuf = CMD_claimOutputBuffer();
                                if (CMDGET_outBuf) {
                                    *CMDGET_outBuf = 0;
                                    newMode = GLOBAL_output_mode_XTERM;
                                    XTERM_strcat_enterXTERMmode(CMDGET_outBuf);
                                    CMD_setNumPayloadBytes(strlen(CMDGET_outBuf));
                                }
                            }
                        }
                    }
                    if (!GLOBAL_setOutputMode(newMode)) {
                        CMD_sendNAK(); /* Command unsuccesful, send NAK */
                    } else {
                        if (CMD_getNumPayloadBytes()) {
                            if (!CMD_startSendOutputBuffer()) {
                                CMD_sendNAK(); /* Command unsuccesful, send NAK */
                            }
                        }
                    }
                    return true;
                }
            }
            if (!strcmp(parName, "REPEAT")) {
                GLOBAL_setAcquisitionMode(GLOBAL_acq_mode_continous);
                return true;
            }
            if (!strcmp(parName, "NOREPEAT")) {
                GLOBAL_setAcquisitionMode(GLOBAL_acq_mode_single);
                return true;
            }
        }
        /* Unknown or missing parameter */
        CMD_sendNAK();
        /* Command processed */
        return true;
    }
    /* Command not recognized */
    return false;
}
