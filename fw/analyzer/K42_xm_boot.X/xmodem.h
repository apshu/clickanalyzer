/* 
 * File:   xmodem.h
 * Author: M91541
 *
 * Created on August 3, 2017, 3:41 PM
 */

#ifndef XMODEM_H
#define	XMODEM_H

#ifdef	__cplusplus
extern "C" {
#endif

#define XM_MAXRETRIES 32

    void XM_resetState(void);
    bit XM_getNextChunk(void);
    uint8_t *XM_getData(int16_t *pktLen);
    void XM_abortTransfer(void);
    
#ifdef	__cplusplus
}
#endif

#endif	/* XMODEM_H */

