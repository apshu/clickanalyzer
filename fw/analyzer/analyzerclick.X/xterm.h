/* 
 * File:   xterm.h
 * Author: M91541
 *
 * Created on June 12, 2017, 1:11 PM
 */

#ifndef XTERM_H
#define	XTERM_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define ANSI_ESC                          "\033"
#define ANSI_BEL                          "\007"
#define ANSI_CSI                          ANSI_ESC "["
#define ANSI_OSC                          ANSI_ESC "]"
#define ANSI_ST                           ANSI_ESC "\\"
#define ANSI_SETCOLOR(color_str)          ANSI_CSI color_str "m"
#define ANSI_SETCURSOR(col,row)           ANSI_CSI ___mkstr(row) ";" ___mkstr(col) "H"

#include "xterm_gui.h"    
    
    size_t XTERM_renderDynamicFrame(char* renderBuf, size_t renderBufLength, const char* cmdLine);
    char* XTERM_strinsert_clientWindowPreamble(char* renderBuf, bool clearScreen);
    char* XTERM_strinsert_clientScrollingWindowPreamble(char* renderBuf, bool clearScreen);
    char* XTERM_strcat_enterXTERMmode(char* renderBuf);
    bool XTERM_executeANSIsequence(const char* ANSIcode);
    void inline XTERM_showNAK(bool isShowNAK);
    const inline char* XTERM_getStaticFrame(void);
    size_t inline XTERM_getStaticFrameLength(void);
    char* XTERM_strinsert_spinner(char* renderBuf);
    
#ifdef	__cplusplus
}
#endif

#endif	/* XTERM_H */

