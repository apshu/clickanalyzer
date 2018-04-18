/* 
 * File:   logic_scope.h
 * Author: M91541
 *
 * Created on May 9, 2017, 1:33 PM
 */

#ifndef LOGIC_SCOPE_H
#define	LOGIC_SCOPE_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {

        struct {
            uint8_t pinmapEntries;
            uint8_t pinmap[14];
        } pinmapInfo;
        uint16_t dataRecords[1];
    } LS_payloadBuf_t;

    bool LS_onCommand(const char* cmdLine);
    size_t inline LS_getMaxNumberOfSamples();

#ifdef	__cplusplus
}
#endif

#endif	/* LOGIC_SCOPE_H */

