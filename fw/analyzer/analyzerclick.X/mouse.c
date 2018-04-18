#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "mouse.h"
#include "cmdline.h"
#include "pinhelper.h"
#include "cmd_modules/signal_show.h"
#include "global_settings.h"

static const MOUSE_onMouseEventFunc MOUSE_eventHandlers[] = {
    CMD_onMouse,
    SSHOW_onMouse,
    GLOBAL_onMouse,
};

static struct {
    uint16_t mx;
    uint16_t my;
    uint16_t evt;
} MOUSE_eventQue[2];

uint8_t MOUSE_eventQueWrPtr;
uint8_t MOUSE_eventQueRdPtr;

void MOUSE_addEvent(uint16_t x, uint16_t y, uint16_t eventCode) {
    MOUSE_eventQue[MOUSE_eventQueWrPtr].mx = x;
    MOUSE_eventQue[MOUSE_eventQueWrPtr].my = y;
    MOUSE_eventQue[MOUSE_eventQueWrPtr].evt = eventCode;
    uint8_t nextQuePtr = (MOUSE_eventQueWrPtr + 1) % (sizeof (MOUSE_eventQue) / sizeof (MOUSE_eventQue[0]));
    if (nextQuePtr != MOUSE_eventQueRdPtr) {
        /* No FIFO queue overrun happens */
        MOUSE_eventQueWrPtr = nextQuePtr;
    }
}

static void MOUSE_sendEvent(uint16_t mx, uint16_t my, uint16_t eventCode) {
    uint8_t i = sizeof (MOUSE_eventHandlers) / sizeof (MOUSE_eventHandlers[0]);
    for (; i > 0; --i) {
        MOUSE_eventHandlers[i - 1](mx, my, eventCode);
    }
}

void MOUSE_task(void) {
    if (MOUSE_eventQueRdPtr != MOUSE_eventQueWrPtr) {
        MOUSE_sendEvent(MOUSE_eventQue[MOUSE_eventQueRdPtr].mx, MOUSE_eventQue[MOUSE_eventQueRdPtr].my, MOUSE_eventQue[MOUSE_eventQueRdPtr].evt);
        ++MOUSE_eventQueRdPtr;
        MOUSE_eventQueRdPtr %= sizeof (MOUSE_eventQue) / sizeof (MOUSE_eventQue[0]);
    }
}
