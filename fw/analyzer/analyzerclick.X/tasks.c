#include <stdlib.h>
#include <stdint.h>

#include "hwresources.h"
#include "mcc_generated_files/mcc.h"
#include "pinhelper.h"
#include "cmdline.h"
#include "tasks.h"
#include "mouse.h"

#include "cmd_modules/reset_all.h"
#include "cmd_modules/logic_scope.h"
#include "cmd_modules/scope.h"
#include "cmd_modules/generic_settings.h"
#include "cmd_modules/signal_show.h"
#include "cmd_modules/dvm.h"

static TS_taskFunc TS_clientExitFunc = NULL;
static TS_taskFunc TS_clientTaskFunc = NULL;
static TS_taskFunc TS_clientOnTickFunc = NULL;
static unsigned int TS_subState;

static enum {
    TS_init,
    TS_wait,
    TS_tick,
    TS_newCmd,
} TS_state = TS_init;

static void TS_initialize(void) {
    TS_TMRreg(TMR, _StopTimer)();
    TS_TMRreg(TMR, _Initialize)();
    TS_TMRreg(TMR, _StartTimer)();
    TS_setExitFunc(NULL);
    TS_setTaskFunc(NULL);
    TS_state = TS_wait;
}

static const TS_onCommandFunc commandHandlers[] = {
    RST_onCommand,
    LS_onCommand,
    SCOPE_onCommand,
    DVM_onCommand,
    CMDGET_onCommand,
    CMDSET_onCommand,
    SSHOW_onCommand,
};

void TS_task(void) {
    switch (TS_state) {
        case TS_init:
            TS_initialize();
            break;
        case TS_wait:
            /* Hardwired task functions: */
            /* Command line input processing */
            CMD_taskIn();
            if (CMD_isNewCommand()) {
                TS_state = TS_newCmd;
                TS_subState = 0;
            } else {
                if (TS_TMRreg(TMR, IF)) {
                    TS_TMRreg(TMR, IF) = 0;
                    TS_TMRreg(TMR, _Reload());
                    TS_state = TS_tick;
                }
            }

            break;
        case TS_tick:
            /* Window watchdog clear */
            if (TS_clientOnTickFunc) {
#pragma warning push
#pragma warning disable 1471   /* temporarily disable indirect function call via a NULL pointer ignored */
                TS_clientOnTickFunc();
#pragma warning pop
            }
            /* Hardwired tick functions:*/
            /* signal show */
            SSHOW_onTick();
            TS_state = TS_wait;
            break;
        case TS_newCmd:
            if (TS_subState < sizeof (commandHandlers) / sizeof (commandHandlers[0])) {
                if (commandHandlers[TS_subState](CMD_getCommandPrompt())) {
                    TS_state = TS_wait;
                    CMD_refreshFrame();
                } else {
                    ++TS_subState;
                }
            } else {
                CMD_sendNAK();
                TS_state = TS_wait;
            }
            break;
        default:
            TS_state = TS_init;
    }
    if (TS_clientTaskFunc) {
        TS_clientTaskFunc();
    }
    /* Hardwired task functions: */
    /* Mouse events processing */
    MOUSE_task();
    /* Command line output processing */
    CMD_taskOut();
}

void TS_setTaskFunc(TS_taskFunc taskFunc) {
    TS_clientTaskFunc = taskFunc;
}

void TS_setOnTickFunc(TS_taskFunc tickFunc) {
    TS_clientOnTickFunc = tickFunc;
}

void TS_setExitFunc(TS_taskFunc onExitFunc) {
    if (TS_clientExitFunc != onExitFunc) {
        if (TS_clientExitFunc) {
            TS_clientExitFunc(); /* Call the exit function if changed */
        }
        TS_clientExitFunc = onExitFunc;
    }
}
