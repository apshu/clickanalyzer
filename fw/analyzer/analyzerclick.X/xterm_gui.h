/* 
 * File:   xterm_gui.h
 * Author: M91541
 *
 * Created on June 29, 2017, 8:09 AM
 */

#ifndef XTERM_GUI_H
#define	XTERM_GUI_H

#ifdef	__cplusplus
extern "C" {
#endif

#define XTERM_screenWidth               101
#define XTERM_ToolbarHeight             3
#define XTERM_userWindowHeight          32
#define XTERM_promptRow                 2
#define XTERM_promptLeft                7
#define XTERM_userWindowLeft            1
#define XTERM_statusbarRow              3   //XTERM_ToolbarHeight
#define XTERM_NAKrow                    3   //XTERM_statusbarRow
#define XTERM_NAKleft                   87  //XTERM_screenWidth - 13
#define XTERM_userWindowRight           101 //XTERM_screenWidth
#define XTERM_userWindowTopRow          4   //XTERM_ToolbarHeight + 1
#define XTERM_userWindowBottomRow       35  //XTERM_userWindowHeight + XTERM_userWindowTopRow - 1
#define XTERM_screenHeight              35  //(XTERM_userWindowHeight + XTERM_ToolbarHeight)
#define XTERM_promptRight               98  //XTERM_screenWidth - 2
#define XTERM_maxPromptLength           92  //XTERM_promptRight - XTERM_promptLeft + 1
#define XTERM_ledYellowLeft             11  
#define XTERM_ledYellowTopRow           3   //XTERM_statusbarRow
#define XTERM_ledOrangeLeft             25  
#define XTERM_ledOrangeTopRow           3   //XTERM_statusbarRow
#define XTERM_ledGreenLeft              38  
#define XTERM_ledGreenTopRow            3   //XTERM_statusbarRow
#define XTERM_ledRedLeft                49  
#define XTERM_ledRedTopRow              3   //XTERM_statusbarRow
#define XTERM_helpIconLeft              97
#define XTERM_helpIconTopRow            1
#define XTERM_buttonYellowLeft          4  
#define XTERM_buttonYellowRight         12  
#define XTERM_buttonYellowTopRow        3   //XTERM_statusbarRow
#define XTERM_buttonOrangeLeft          18  
#define XTERM_buttonOrangeRight         26  
#define XTERM_buttonOrangeTopRow        3   //XTERM_statusbarRow
#define XTERM_buttonGreenLeft           32  
#define XTERM_buttonGreenRight          39  
#define XTERM_buttonGreenTopRow         3   //XTERM_statusbarRow
#define XTERM_buttonRedLeft             45  
#define XTERM_buttonRedRight            50  
#define XTERM_buttonRedTopRow           3   //XTERM_statusbarRow
#define XTERM_buttonAcquLeft            65  
#define XTERM_buttonAcquRight           70  
#define XTERM_buttonAcquTopRow          3   //XTERM_statusbarRow
#define XTERM_buttonTitleLeft           41
#define XTERM_buttonTitleRight          58  
#define XTERM_buttonTitleTopRow         3   //XTERM_statusbarRow
#define XTERM_spinnerLeft               72  //XTERM_buttonAcquRight+2
#define XTERM_spinnerTop                3   //XTERM_statusbarRow
#define XTERM_LS_firstRow               (XTERM_userWindowTopRow + 2)
#define XTERM_SCOPE_firstRow            (XTERM_userWindowTopRow + 5)
#define XTERM_DVM_firstRow              (XTERM_userWindowTopRow + 2)
#if defined( CIRCUITBOARD ) && ( CIRCUITBOARD > 0 )
#define XTERM_DVM_magnitudeLeft         15
#else
#error "Please define circuit board type and/or include hwresources.h"
#endif                    
#define XTERM_DVM_magnitudeRight        98
#define XTERM_DVM_magnitudeLength       (XTERM_DVM_magnitudeRight - XTERM_DVM_magnitudeLeft)
#ifdef	__cplusplus
}
#endif

#endif	/* XTERM_GUI_H */

