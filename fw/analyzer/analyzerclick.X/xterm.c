#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <cci.h>
#include "hwresources.h"
#include "pinhelper.h"
#include "xterm.h"
#include "global_settings.h"
#include "cmdline.h"
#include "mouse.h"
#include "cmd_modules/signal_show.h"

#pragma warning push
#pragma warning disable 336   /* Suppress string concatenation across lines */
#pragma warning disable 228   /* Suppress illegal character warning */
static const char XTERM_static_frame[] =
        ANSI_CSI "?25l" /* Hide cursor */
        ANSI_CSI "H" /* Home */
        ANSI_CSI "?7l" /* Disable line wrapping */
#include "static_frame.ansi.inc.h"
        ;
#pragma warning pop

#pragma warning push
#pragma warning disable 228   /* Suppress illegal character warning */
static const char XTERM_dynamic_frame_template[] =
#include "dynamic_frame_template.ansi.inc.h"
        ;
#pragma warning pop

const inline char* XTERM_getStaticFrame(void) {
    return XTERM_static_frame;
}

size_t inline XTERM_getStaticFrameLength(void) {
    return sizeof (XTERM_static_frame);
}

static bit XTERM_showNAKneeded;

static inline bit XTERM_isShowNAKneeded(void) {
    return XTERM_showNAKneeded;
}

static char* XTERM_strcat_showCommandLine(char* renderBuf, const char* cmdLine) {
#pragma warning push
#pragma warning disable 336   /* Suppress string concatenation across lines */
    strcat(renderBuf,
            ANSI_SETCURSOR(XTERM_promptLeft, XTERM_promptRow) /* Move cursor (7,2) */
            ANSI_SETCOLOR("37;40") /* White on black background */
            ANSI_CSI ___mkstr(XTERM_maxPromptLength)"X" /* Erase characters */
            );
#pragma warning pop
    if (strlen(cmdLine) >= (XTERM_maxPromptLength)) {
        /* Show "more characters are there" sign */
        strcat(renderBuf, "\256");
        cmdLine = cmdLine + strlen(cmdLine) - (XTERM_maxPromptLength) + 2;
    }
    strcat(renderBuf, cmdLine);
    strcat(renderBuf, ANSI_CSI "?25h" /* Show cursor */);
    return renderBuf;
}

static char* XTERM_strcat_displayNAK(char* renderBuf) {
#pragma warning push
#pragma warning disable 336   /* Suppress string concatenation across lines */
    return strcat(renderBuf,
            ANSI_SETCURSOR(XTERM_NAKleft, XTERM_NAKrow) /* Move cursor (67,3) */
            ANSI_SETCOLOR("37;41") /* White on red background */
            " CMD ERROR ");
#pragma warning pop
}

static char* XTERM_strcpy_LEDassignment(char* renderBuf, uint8_t pinId) {
    if (pinId) {
        if (pinId == SSHOW_pinIsAnalog) {
            strcpy(renderBuf, "AD");
        } else {
            if (pinId < 100) {
        sprintf(renderBuf, "%2d", pinId);
            }
        }
    } else {
        strcpy(renderBuf, "xx");
    }
    return renderBuf;
}

size_t XTERM_renderDynamicFrame(char* renderBuf, size_t renderBufLength, const char* cmdLine) {
    *renderBuf = 0;
    char ledyellowsym[3];
    char ledorangesym[3];
    char ledgreensym[3];
    char ledredsym[3];
    sprintf(renderBuf, XTERM_dynamic_frame_template,
            XTERM_strcpy_LEDassignment(ledyellowsym, SSHOW_getInputPinForLED(PINS_LED_YELLOW)),
            XTERM_strcpy_LEDassignment(ledorangesym, SSHOW_getInputPinForLED(PINS_LED_ORANGE)),
            XTERM_strcpy_LEDassignment(ledgreensym, SSHOW_getInputPinForLED(PINS_LED_GREEN)),
            XTERM_strcpy_LEDassignment(ledredsym, SSHOW_getInputPinForLED(PINS_LED_RED)),
            (GLOBAL_getAcquisitionMode() == GLOBAL_acq_mode_single) ? 170 : 236);
    if (XTERM_isShowNAKneeded()) {
        XTERM_showNAK(false);
        XTERM_strcat_displayNAK(renderBuf);
    }
    XTERM_strcat_showCommandLine(renderBuf, cmdLine);
    renderBuf[renderBufLength - 1] = 0;
    return (strlen(renderBuf));
}

char* XTERM_strinsert_clientWindowPreamble(char* renderBuf, bool clearScreen) {
    if (GLOBAL_getOutputMode() == GLOBAL_output_mode_XTERM) {
        strcat(renderBuf, ANSI_SETCURSOR(XTERM_userWindowLeft, XTERM_userWindowTopRow) ANSI_SETCOLOR("0") ANSI_CSI "?25l"); /* Hide cursor */
        if (clearScreen) {
            strcat(renderBuf, ANSI_CSI "J");
        }
        renderBuf = renderBuf + strlen(renderBuf);
    }
    return renderBuf;
}

char* XTERM_strinsert_clientScrollingWindowPreamble(char* renderBuf, bool clearScreen) {
    if (GLOBAL_getOutputMode() == GLOBAL_output_mode_XTERM) {
        renderBuf = XTERM_strinsert_clientWindowPreamble(renderBuf, true);
        renderBuf = renderBuf + strlen(strcpy(renderBuf, ANSI_CSI "?7h"));
    }
    return renderBuf;
}

char* XTERM_strcat_enterXTERMmode(char* renderBuf) {
    XTERM_showNAK(false);
#pragma warning push
#pragma warning disable 336   /* Suppress string concatenation across lines */
    strcat(renderBuf, "\033c" /* reset terminal */
            ANSI_OSC "?PIC18F26K42 Click Analyzer" ANSI_ST /* Set window title */
            ANSI_CSI "?1000h" /* Enable mouse event reporting */
            ANSI_CSI "?1006h" /* Enable extended mouse reporting */
            ANSI_CSI "8;" ___mkstr(XTERM_screenHeight)";" ___mkstr(XTERM_screenWidth)"t" /* Set window size in characters */
            ANSI_CSI "?25l" /* Hide cursor */
            ANSI_CSI ___mkstr(XTERM_userWindowTopRow)";" ___mkstr(XTERM_userWindowBottomRow)"r" /* Define scrolling region */
            );
#pragma warning pop
    return renderBuf;
}

static void XTERM_addMouseEvent(const char* ANSIcode, bool isButtonPressed) {
    unsigned long mx, my, evt;
    evt = strtoul(ANSIcode, (char**) &ANSIcode, 10);
    ++ANSIcode;
    mx = strtoul(ANSIcode, (char**) &ANSIcode, 10);
    ++ANSIcode;
    my = strtoul(ANSIcode, NULL, 10);
    if (!isButtonPressed) {
        evt += MOUSE_EVT_release;
    }
    MOUSE_addEvent(mx, my, evt);
}

bool XTERM_executeANSIsequence(const char* ANSIcode) {
    char ANSIlastChar = ANSIcode[strlen(ANSIcode) - 1];
    if (*ANSIcode == '[') { /* Got ANSI_CSI */
        ++ANSIcode;
        switch (ANSIlastChar) {
            case 'n': /* Terminal functions OK, auto-detect successful */
                if (*ANSIcode == '0') {
                    static const char XTERM_command_startXTERMmode[] = {'S', 'E', 'T', CMD_char_SEPARATOR, 'O', 'U', 'T', 'P', 'U', 'T', CMD_char_SEPARATOR, 'X', 'T', 'E', 'R', 'M', 0};
                    CMD_startSingleCommand(XTERM_command_startXTERMmode, CMD_char_EOS);
                }
                break;

            case '~': /* Delete button pressed */
                if (*ANSIcode == '3') {
                    CMD_startSingleCommand("", CMD_char_EOS);
                }
                break;

            case 'M': /* Mouse button pressed */
            case 'm': /* Mouse button released */
                XTERM_addMouseEvent(ANSIcode + 1, (bool) (ANSIlastChar == 'M'));
                break;

            default:
                break;
        }
    }
    return true;
}

void inline XTERM_showNAK(bool isShowNAK) {
    XTERM_showNAKneeded = isShowNAK;
}

char* XTERM_strinsert_spinner(char* renderBuf) {
    static uint8_t LS_spinnerAnimPos;
    static const char LS_spinnerAnimChars[] = {180, 217, 193, 192, 195, 218, 194, 191};
    strcat(renderBuf, ANSI_SETCURSOR(XTERM_spinnerLeft, XTERM_spinnerTop) ANSI_SETCOLOR("0;1;44;36"));
    renderBuf = renderBuf + strlen(renderBuf);
    LS_spinnerAnimPos %= sizeof(LS_spinnerAnimChars);
    *renderBuf = LS_spinnerAnimChars[LS_spinnerAnimPos++];
    *++renderBuf = 0;
    return renderBuf;
}