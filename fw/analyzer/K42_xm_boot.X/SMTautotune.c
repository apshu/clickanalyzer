#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include "SMTautotune.h"

static int_fast8_t SMTTUN_getOSCTUNE(void) {
    return OSCTUNE | OSCTUNEbits.HFTUN5 << 6 | OSCTUNEbits.HFTUN5 << 7;
}

static void SMTTUN_increase_sepeed(void) {
    int_fast8_t tuneVal = SMTTUN_getOSCTUNE();
    if (tuneVal < SMTTUN_OSCCON_ALLOWED_max) {
        OSCTUNE = (unsigned) ((tuneVal + 1)) & _OSCTUNE_TUN_MASK;
        __delay_ms(1);
    }
}

static void SMTTUN_decrease_sepeed(void) {
    int_fast8_t tuneVal = SMTTUN_getOSCTUNE();
    if (tuneVal > SMTTUN_OSCCON_ALLOWED_min) {
        OSCTUNE = (unsigned) ((tuneVal - 1)) & _OSCTUNE_TUN_MASK;
        __delay_ms(1);
    }
}

bool SMTTUN_tune(void) {
    uint_fast8_t iter = SMTTUN_ITERATIONS;
    do {
        SMT1_Initialize(); /* Start SMT */
        uint_fast8_t averages = 1 << SMTTUN_AVERAGES;
        uint32_t averagevalue = 0UL;
        do {
            while (!(SMT1PRAIF || SMT1IF)) continue;
            if (SMT1IF) {
                /* SMT timer overflow */
                return false;
            }
            SMT1PRAIF = 0; /* Reset interrupt flag */
            averagevalue += SMT1_GetCapturedPeriod(); /* Acumulate new period */
        } while (--averages);
        averagevalue >>= SMTTUN_AVERAGES; /* Do averaging */
        if (averagevalue > SMTTUN_SMT_EXPECTED_AVGVALUE_max) {
            SMTTUN_decrease_sepeed();
        } else if (averagevalue < SMTTUN_SMT_EXPECTED_AVGVALUE_min) {
            SMTTUN_increase_sepeed();
        }
    } while (--iter);
    return true;
} 