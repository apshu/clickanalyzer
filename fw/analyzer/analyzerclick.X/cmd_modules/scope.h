
#ifndef SCOPE_H
#define	SCOPE_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        float vrefVoltage;
        uint8_t ADCbits;
        uint8_t sampledPin;
        float sampleRate;
        uint16_t dataRecords[1];
    } SCOPE_payloadBuf_t;

    bool SCOPE_onCommand(const char* cmdLine);
    size_t inline SCOPE_getMaxNumberOfSamples();

#ifdef	__cplusplus
}
#endif

#endif	/* SCOPE_H */

