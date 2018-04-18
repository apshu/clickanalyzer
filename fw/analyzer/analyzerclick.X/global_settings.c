#include <xc.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "hwresources.h"
#include "global_settings.h"
#include "cmdline.h"
#include "xterm.h"
#include "mouse.h"

static GLOBAL_acq_mode_t GLOBAL_mode_acq;
static GLOBAL_output_mode_t GLOBAL_output_mode = GLOBAL_output_mode_JSON;
static uint8_t* GLOBAL_samplingBufferPtr;
static uint16_t GLOBAL_samplingBufferNumBytes = 0;
static uint8_t GLOBAL_smallSamplingBuffer[215];
static bit GLOBAL_isStopCommandRequest;

GLOBAL_acq_mode_t inline GLOBAL_getAcquisitionMode() {
    return GLOBAL_mode_acq;
}

GLOBAL_output_mode_t inline GLOBAL_getOutputMode() {
    return GLOBAL_output_mode;
}

bool GLOBAL_setOutputMode(GLOBAL_output_mode_t newOutputMode) {
    switch (newOutputMode) {
        case GLOBAL_output_mode_binary:
        case GLOBAL_output_mode_XTERM:
        case GLOBAL_output_mode_JSON:
            GLOBAL_output_mode = newOutputMode;
            return true;

        default:
            break;
    }
    return false;
}

uint8_t* GLOBAL_claimSamplingBuffer() {
    switch (GLOBAL_getOutputMode()) {
        case GLOBAL_output_mode_binary:
            GLOBAL_samplingBufferPtr = CMD_claimOutputBuffer();
            break;

        case GLOBAL_output_mode_JSON:
        case GLOBAL_output_mode_XTERM:
            GLOBAL_samplingBufferPtr = GLOBAL_smallSamplingBuffer;
            break;

        default:
            return NULL;
    }
    return GLOBAL_samplingBufferPtr;
}

size_t GLOBAL_getMaxSampleBufBytes(void) {
    switch (GLOBAL_getOutputMode()) {
        case GLOBAL_output_mode_binary:
            return CMD_getOutputBufferSize();

        case GLOBAL_output_mode_JSON:
        case GLOBAL_output_mode_XTERM:
            return sizeof (GLOBAL_smallSamplingBuffer);

        default:
            break;
    }
    return 0;
}

void inline GLOBAL_setSampleBufByteSize(size_t numBytes) {
    GLOBAL_samplingBufferNumBytes = numBytes;
} /* Store number of samples */

size_t inline GLOBAL_getSampleBufByteSize() {
    return (GLOBAL_samplingBufferNumBytes);
} /* Get number of samples set */

void inline GLOBAL_setAcquisitionMode(GLOBAL_acq_mode_t newMode) {
    GLOBAL_mode_acq = newMode;
}

void GLOBAL_toggleAcquisitionMode(void) {
    GLOBAL_setAcquisitionMode((GLOBAL_mode_acq == GLOBAL_acq_mode_single) ? GLOBAL_acq_mode_continous : GLOBAL_acq_mode_single);
}

void GLOBAL_onMouse(uint16_t mouseX, uint16_t mouseY, uint16_t mouseEvent) {
    if ((mouseX >= XTERM_buttonAcquLeft) && (mouseX <= XTERM_buttonAcquRight) && (mouseY == XTERM_buttonAcquTopRow) && (mouseEvent & MOUSE_EVT_release)) {
        GLOBAL_toggleAcquisitionMode();
        CMD_refreshFrame();
    } else {
        if ((mouseX == XTERM_spinnerLeft) && (mouseY == XTERM_spinnerTop)) {
            GLOBAL_setStopRequest(true);
        }
    }
}

inline bit GLOBAL_isStopRequested(void) {
    return GLOBAL_isStopCommandRequest;
}

inline void GLOBAL_setStopRequest(bool isStopNeeded) {
    GLOBAL_isStopCommandRequest = isStopNeeded;
}