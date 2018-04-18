#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "string_helpers.h"

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

