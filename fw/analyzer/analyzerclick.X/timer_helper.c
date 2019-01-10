#include <xc.h>
#include <stdint.h>
#include <stdlib.h>
#include "timer_helper.h"

static inline uint8_t TevenCKPStoDIV(uint8_t CKPSval) {
    return (1U << CKPSval);
}

static inline uint8_t TevenOUTPStoDIV(uint8_t OUTPSval) {
    return (OUTPSval + 1U);
}

static inline uint16_t TevenPRtoDIV(uint8_t PRval) {
    return (PRval + 1U);
}

typedef struct {
    volatile T2TMRbits_t TxTMR;
    volatile uint8_t TxPR;
    volatile T2CONbits_t TxCON;
    volatile T2HLTbits_t TxHLT;
    volatile T2CLKbits_t TxCLK;
    volatile T2RSTbits_t TxRST;
} TMRevenSFR_t;

float TMRxToHz(uint8_t tmrID, float TMRfreqHz) {
    TMRevenSFR_t* Teven = NULL;
    switch (tmrID) {
        case 2:
            Teven = (TMRevenSFR_t*) & T2TMR;
            break;
        case 4:
            Teven = (TMRevenSFR_t*) & T4TMR;
            break;
        case 6:
            Teven = (TMRevenSFR_t*) & T6TMR;
            break;
        case 1 + TMRHELPER_TMRIS_NCO:
            if (NCO1CONbits.EN) {
                return ((long double)TMRfreqHz * (long double) NCO1INC) / 1048576.0;
            }
            break;
        default:
            return 0.0;
    }
    if (Teven) {
        return (TMRfreqHz / (1.0 * TevenCKPStoDIV(Teven->TxCON.CKPS) * TevenOUTPStoDIV(Teven->TxCON.OUTPS) * TevenPRtoDIV(Teven->TxPR)));
    }
    return 0.0;
}
