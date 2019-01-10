#include <stdlib.h>
#include <string.h>
#include "../hwresources.h"
#include "../mcc_generated_files/mcc.h"
#include "../tasks.h"
#include "../cmdline.h"
#include "reset_all.h"
#include "../global_settings.h"

static void inline RST_executeDeviceReset(void) {
    GIE = 0;
    abort();
    while (1) continue; /* Wait watchdog reset */
}

/* returns true if new char may go into the queue */
bool RST_onNewChar(char newChar) {
    if (RST_cmd_rebootDevice == newChar) { /* If reset char anywhere in the command, do reset */
        RST_executeDeviceReset();
    }
    if (newChar == RST_cmd_resetCommand) { /* If reset command char anywhere in the command, force stop command */
        TS_setTaskFunc(NULL);
        TS_setExitFunc(NULL);
        return false;
    }
    if (newChar == RST_cmd_stop) {
        GLOBAL_setStopRequest(true);
        return false;
    }
    return true;
}

static void RST_jumpToBootloader(void) {
    DMA1CON0 = DMA2CON0 = 0; /* stop DMA */
    while (!(U1TXMTIF && U2TXMTIF)) { /* Wait for UART TX complete */
        continue;
    }
    GIE = 0;
    STKPTR = 1;
    TOS = 0UL;
    asm("RETURN");
}

/* Parse command line and return true if entering RST mode command accepted */
bool RST_onCommand(const char* cmdLine) {
    if (!cmdLine) {
        /* NULL pointer, something went wrong... */
        RST_executeDeviceReset();
    }
    if (!strcmp(CMD_getCommand(cmdLine), "GOTOBOOTLOADER")) {
        TS_setTaskFunc(NULL);
        TS_setExitFunc(NULL);
        RST_jumpToBootloader();
    }
    return false;
}

void __interrupt(irq(RST_TMRregpre(IRQ_TMR)), base(IVT1_BASE_ADDRESS), low_priority) RST_Hearthbeat_WatchdogExpired_ISR(void) {
    /* This ISR is executed only when hearthbeat from commchip stops */
    GIE = 0;
    __delay_ms(1); /* Wait for any pending UART transaction */
    RST_executeDeviceReset(); /* Do the reset */
}