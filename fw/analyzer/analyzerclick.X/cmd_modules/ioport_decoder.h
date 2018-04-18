/* 
 * File:   ioport_decoder.h
 * Author: M91541
 *
 * Created on May 15, 2017, 2:41 PM
 */

#ifndef IOPORT_DECODER_H
#define	IOPORT_DECODER_H

#ifdef	__cplusplus
extern "C" {
#endif

#if IO_LED_GREEN_TRIS == "TRISAbits.TRISA1"
#define IO_LED_GREEN_PORTLETTER() 'A'
#endif
    
#ifdef	__cplusplus
}
#endif

#endif	/* IOPORT_DECODER_H */

