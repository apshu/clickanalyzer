/* 
 * File:   hwresources.h
 * Author: M91541
 *
 * Created on May 11, 2017, 1:45 PM
 */

#ifndef HWRESOURCES_H
#define	HWRESOURCES_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define PCB_clickboard 1
#define PCB_gumstick 2
#define PCB_gumstick_alt_UART 3
    
#if (CIRCUITBOARD <= 0) || (CIRCUITBOARD > 3)
#undef CIRCUITBOARD
#endif
    
#ifndef CIRCUITBOARD
#warning "CIRCUITBOARD not specified, using default"
#define CIRCUITBOARD PCB_clickboard
#endif
    
    /* command line and PC communications processor */
#define CMD_UARTID 1
#define CMD_DMAID 2

    /* Task scheduler */
#define TS_TMRID 6
    
    /* Logic scope */
#define LS_NCOID 1
#define LS_DMAID 1
    
    /* Reset all */
#define RST_TMRID 2    /* This is the commchip haerthbeat watching timer */
    
/* Helper functions */
#define __MACRO_reg(peripheralid, pre, post) pre##peripheralid##post
#define _MACRO_reg(peripheralid, pre, post) __MACRO_reg(peripheralid, pre, post)
#define __MACRO_regpre(peripheralid, pre) pre##peripheralid
#define _MACRO_regpre(peripheralid, pre) __MACRO_regpre(peripheralid, pre)
    
#define __LS_NCOreg(tmrid, pre, post) pre##tmrid##post
#define _LS_NCOreg(tmrid, pre, post) __LS_NCOreg(tmrid, pre, post)
#define LS_NCOreg(pre, post) _LS_NCOreg(LS_NCOID, pre, post)

#define __LS_DMAreg(uartid, pre, post) pre##uartid##post
#define _LS_DMAreg(uartid, pre, post) __LS_DMAreg(uartid, pre, post)
#define LS_DMAreg(pre, post) _LS_DMAreg(LS_DMAID, pre, post)
    
#define __CMD_UARTreg(uartid, pre, post) pre##uartid##post
#define _CMD_UARTreg(uartid, pre, post) __CMD_UARTreg(uartid, pre, post)
#define CMD_UARTreg(pre, post) _CMD_UARTreg(CMD_UARTID, pre, post)

#define __CMD_DMAreg(uartid, pre, post) pre##uartid##post
#define _CMD_DMAreg(uartid, pre, post) __CMD_DMAreg(uartid, pre, post)
#define CMD_DMAreg(pre, post) _CMD_DMAreg(CMD_DMAID, pre, post)

#define __TS_TMRreg(tmrid, pre, post) pre##tmrid##post
#define _TS_TMRreg(tmrid, pre, post) __TS_TMRreg(tmrid, pre, post)
#define TS_TMRreg(pre, post) _TS_TMRreg(TS_TMRID, pre, post)

#define RST_TMRreg(pre, post) _MACRO_reg(RST_TMRID, pre, post)
#define RST_TMRregpre(pre) _MACRO_regpre(RST_TMRID, pre)

#ifdef	__cplusplus
}
#endif

#endif	/* HWRESOURCES_H */

