#include <xc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "string_helpers.h"

extern const uint8_t DIA_MicrochipEUI[16] __at(DIA_MUI);

char* strReplaceAllChar(char* str, char find, char replace) {
    if (str && find) {
        char *current_pos = strchr(str, find);
        while (current_pos) {
            *current_pos = replace;
            current_pos = strchr(current_pos, find);
        }
    }
    return str;
}

#define MICRO "\xE6" /* Or use "u" */

char* floatToEng(float number, char* renderBuf) {
    const char* prefix = "";
    if (number > 0.0) {
        if (number < 1.0) {
            number *= 1000.0;
            if (number < 1.0) {
                number *= 1000.0;
                if (number < 1.0) {
                    number *= 1000.0;
                    prefix = "n";
                } else {
                    prefix = MICRO;
                }
            } else {
                prefix = "m";
            }
        } else {
            if (number >= 1000.0) {
                number /= 1000.0;
                if (number >= 1000.0) {
                    number /= 1000.0;
                    prefix = "M";
                } else {
                    prefix = "k";
                }
            }
        }
    }
    sprintf(renderBuf, "%.1f%s", number, prefix);
    return renderBuf;
}

char* getSerialnumberString(char* renderBuf) {
    return toBase26(DIA_MicrochipEUI, 12, renderBuf);
}

char* toBase26(const uint8_t *data, size_t numDataBytes, char* renderBuf) {
    while (numDataBytes--) {
        *renderBuf++ = ((*data) & 0x0FU) + 'A';
        *renderBuf++ = ((uint8_t) (*data++ >> 4U) & 0x0FU) + 'A';
    }
    *renderBuf = 0;
    return renderBuf;
}
