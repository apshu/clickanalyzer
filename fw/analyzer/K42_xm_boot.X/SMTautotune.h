/* 
 * File:   SMTautotune.h
 * Author: M91541
 *
 * Created on December 11, 2017, 3:46 PM
 */

#ifndef SMTAUTOTUNE_H
#define	SMTAUTOTUNE_H

#ifdef	__cplusplus
extern "C" {
#endif


#define SMTTUN_SMT_EXPECTED_AVGVALUE_min    (170UL) /* Minimum SMT value to accept */
#define SMTTUN_SMT_EXPECTED_AVGVALUE_max    (171UL) /* Maximum SMT value to accept */
#define SMTTUN_OSCCON_ALLOWED_max           (10)    /* OSCTUN drift max [<= 31] */
#define SMTTUN_OSCCON_ALLOWED_min           (-10)   /* OSCTUN drift min [<= -32] */
#define SMTTUN_ITERATIONS                   (50)    /* Number of iterations [<255] */
#define SMTTUN_AVERAGES                     (4)     /* Average calucalation, calcualtions are 1<<SMTTUN_AVERAGES [<8] */
    bool SMTTUN_tune(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SMTAUTOTUNE_H */

