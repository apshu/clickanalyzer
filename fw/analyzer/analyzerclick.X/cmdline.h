/* 
 * File:   cmdline.h
 * Author: M91541
 *
 * Created on May 9, 2017, 10:07 AM
 */

#ifndef CMDLINE_H
#define	CMDLINE_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum {
    CMD_outBufferSize = 1100,
    CMD_inBufferSize = 100,
    CMD_inANSIbufferSize = 20,
} CMD_bufferSizes_t;

#define CMD_maxParamNameLength 15

    typedef struct {
        uint16_t crc16;
        uint16_t payloadCode;
        uint16_t numPayloadBytes;
    } CMD_header_t;

    typedef enum {
        CMD_char_EOS = ';', /* End of sequence char */
        CMD_char_SEPARATOR = ' ', /* Parameter separator char */
        CMD_char_ASSIGNMENT = '=', /* Parameter value assignment */
        CMD_char_NAK = '!', /* NAK character */
    } CMD_protocolChars_t;

    typedef enum {
        CMD_payload_is_LOGIC_SCOPE = 'L' | ('S' << 8),
        CMD_payload_is_GET_RESPONSE = 'G' | ('T' << 8),
        CMD_payload_is_DIGITAL_VOLTMETER = 'D' | ('V' << 8),
        CMD_payload_is_ANALOG_SCOPE = 'A' | ('S' << 8),
        CMD_payload_is_NAK = CMD_char_NAK | (CMD_char_NAK << 8),
    } CMD_payloadTypes_t;
    
    void CMD_taskIn(void);
    void CMD_taskOut(void);
    bool CMD_isNewCommand(void);
    void CMD_startSingleCommand(const char* commandline, char queChar);
    const char* CMD_getCommand(const char* commandPrompt);
    const char* CMD_getCommandPrompt(void);
    bool CMD_getParam(const char* commandPrompt, uint8_t paramNum, char* name, long* value);
    uint8_t* CMD_claimOutputBuffer(void);
    uint8_t* CMD_getOutputBufferAddress(void);
    size_t CMD_getOutputBufferSize(void);
    bool CMD_startSendOutputBuffer(); /* Does not start any actual transmission */
    bool CMD_isTransmissionComplete(void);
    bool CMD_sendNAK(void);
    void inline CMD_setNumPayloadBytes(size_t numPayloadBytes);
    size_t inline CMD_getNumPayloadBytes(void);
    void inline CMD_setPayloadType(CMD_payloadTypes_t payloadType);
    void CMD_abortSendOutputBuffer(void);
    void inline CMD_unlockOutputBuffer(void);
    inline void CMD_refreshFrame(void);
    
    void CMD_onMouse(uint16_t mouseX, uint16_t mouseY, uint16_t mouseEvent);
#ifdef	__cplusplus
}
#endif

#endif	/* CMDLINE_H */

