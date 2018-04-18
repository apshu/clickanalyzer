/* 
 * File:   timer_helper.h
 * Author: M91541
 *
 * Created on June 26, 2017, 4:38 PM
 */

#ifndef TIMER_HELPER_H
#define	TIMER_HELPER_H

#ifdef	__cplusplus
extern "C" {
#endif

#define TMRHELPER_TMRIS_NCO 128

    float TMRxToHz(uint8_t tmrID, float TMRfreqHz);

#ifdef	__cplusplus
}
#endif

#endif	/* TIMER_HELPER_H */

