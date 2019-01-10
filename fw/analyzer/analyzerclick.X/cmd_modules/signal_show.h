/* 
 * File:   signal_show.h
 * Author: M91541
 *
 * Created on May 11, 2017, 4:32 PM
 */

#ifndef SIGNAL_SHOW_H
#define	SIGNAL_SHOW_H

#ifdef	__cplusplus
extern "C" {
#endif

#define SSHOW_defaultOffDelayTicks() (200)  /* Monostable period */
#define SSHOW_pinIsAnalog (100)    
    
    bool SSHOW_onCommand(const char* cmdLine);
    void SSHOW_onTick(void);
    uint8_t SSHOW_getInputPinForLED(PINS_output_t outPin);
    void SSHOW_onMouse(uint16_t mouseX, uint16_t mouseY, uint16_t mouseEvent);
    
#ifdef	__cplusplus
}
#endif

#endif	/* SIGNAL_SHOW_H */

