/* 
 * File:   pinhelper.h
 * Author: M91541
 *
 * Created on May 24, 2017, 3:17 PM
 */

#ifndef PINHELPER_H
#define	PINHELPER_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        char portName;
        uint8_t portBit;
        uint8_t isAnalog;
    } PINS_pindef_t;

    typedef enum {
        PINS_LED_YELLOW,
        PINS_LED_ORANGE,
        PINS_LED_GREEN,
        PINS_LED_RED,
    } PINS_output_t;

    PINS_pindef_t* PINS_pinnumToPinDef(uint8_t pinId);
    uint8_t PINS_pinDefToPinNum(PINS_pindef_t* thePin); /* 0 for error */
    void PINS_setPin(PINS_output_t thePin, bool isHigh);
    void PINS_fetchDigitalInputChanges(void);
    bool PINS_isDigitalInputChanged(PINS_pindef_t *thePin);
    size_t inline PINS_getNumPins(void);
    uint8_t PINS_pinToLinearBitnumber(PINS_pindef_t *thePin);
    void PINS_setPinModeAnalog(PINS_pindef_t *thePin, bool isAnalog);
    bool PINS_pinToADC(PINS_pindef_t *thePin);
    float PINS_measureVref(void);
    void PINS_setAllPinsAnalog(bool isAnalog);
    bool PINS_isPinAnalog(PINS_pindef_t *thePin);
    
#ifdef	__cplusplus
}
#endif

#endif	/* PINHELPER_H */

