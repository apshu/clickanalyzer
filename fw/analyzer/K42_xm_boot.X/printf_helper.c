#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "mcc_generated_files/mcc.h"
#include "printf_helper.h"

void flashTableHack(void) {
    asm("DB 13,10,'X','b','o','o','t',':','A','P','P','s','i','g',' ','f','a','i','l',0");
    asm("DB 13,10,'X','b','o','o','t',':','A','P','P','C','R','C',' ','f','a','i','l',0");
    asm("DB 13,10,'X','b','o','o','t',':','B','o','o','t','b','a','u','d',0");
    asm("DB 13,10,10,'X','b','o','o','t',':','G','o','t','o','b','o','o','t',0");
    asm("DB 13,10,'X','b','o','o','t',':','V','e','r','s','i','o','n',' ','1','.','0',13,10,'X','b','o','o','t',':','W','a','i','t',' ','X','M','O','D','E','M','>',13,10,0");
    asm("DB 0");
}

void strOut(uint8_t idx) {
    UART1_Initialize();
    asm("GLOBAL _flashTableHack");
    WREG = idx;
    TBLPTR = (uint32_t) flashTableHack;
    asm("repeat: TBLRD *+");
    asm("MOVF TABLAT, W");
    asm("BNZ txordrop"); /* nonzero byte, transmit or repeat */
    asm("MOVF strOut@idx, F, c");
    asm("BZ exit");
    asm("DECF strOut@idx, F, c"); /* Zero byte, decrement index */
    asm("TBLRD *+"); /* Get next byte */
    asm("MOVF TABLAT, W");
    asm("BNZ txordrop");
    asm("exit: RETURN");
    asm("txordrop:");
    asm("TSTFSZ strOut@idx, c"); /* check index */
    asm("BRA repeat"); /* idx is nonzero, we skip this */
    asm("CALL _UART1_Write"); /* idx is zero, send the byte */
    asm("BRA repeat");
}
#pragma regsused strOut TBLPTRL, TBLPTRH, TBLPTRU, TABLAT, STATUS, WREG


