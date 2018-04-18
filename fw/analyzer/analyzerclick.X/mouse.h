/* 
 * File:   mouse.h
 * Author: M91541
 *
 * Created on June 28, 2017, 3:20 PM
 */

#ifndef MOUSE_H
#define	MOUSE_H

#ifdef	__cplusplus
extern "C" {
#endif
    typedef void (*MOUSE_onMouseEventFunc)(uint16_t, uint16_t, uint16_t);   /* mouseX, mouseY, eventCode */

    typedef enum {
        MOUSE_EVT_BTN_left = 0,
        MOUSE_EVT_BTN_middle = 1,
        MOUSE_EVT_BTN_right = 2,
        MOUSE_EVT_BTN_wheelup = 0x40,
        MOUSE_EVT_BTN_wheeldown = 0x41,
    } MOUSE_event_t;
    
    typedef enum {
        MOUSE_EVT_drag = 0x20,
        MOUSE_EVT_shift = 0x04,
        MOUSE_EVT_ctrl = 0x10,
        MOUSE_EVT_press = 0x00,
        MOUSE_EVT_release = 0x80,
    } MOUSE_event_modifier_t;
    
#define MOUSE_eventBurronsOnly(eventcode) (eventcode & (MOUSE_EVT_BTN_left | MOUSE_EVT_BTN_middle | MOUSE_EVT_BTN_right | MOUSE_EVT_BTN_wheelup | MOUSE_EVT_BTN_wheeldown))
    void MOUSE_addEvent(uint16_t x, uint16_t y, uint16_t eventCode);
    void MOUSE_task(void);

#ifdef	__cplusplus
}
#endif

#endif	/* MOUSE_H */

