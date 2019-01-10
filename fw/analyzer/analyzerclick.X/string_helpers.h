/* 
 * File:   string_helpers.h
 * Author: M91541
 *
 * Created on June 12, 2017, 12:28 PM
 */

#ifndef STRING_HELPERS_H
#define	STRING_HELPERS_H

#ifdef	__cplusplus
extern "C" {
#endif

char* strReplaceAllChar(char* str, char find, char replace);
char* floatToEng(float number, char* renderBuf);
char* getSerialnumberString(char* renderBuf);
char* toBase26(const uint8_t *data, size_t numDataBytes, char* renderBuf);

#ifdef	__cplusplus
}
#endif

#endif	/* STRING_HELPERS_H */

