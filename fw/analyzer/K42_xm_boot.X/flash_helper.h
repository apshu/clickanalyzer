/* 
 * File:   flash_helper.h
 * Author: M91541
 *
 * Created on August 1, 2017, 2:10 PM
 */

#ifndef FLASH_HELPER_H
#define	FLASH_HELPER_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        uint32_t flashAddress;
        uint8_t flashWriteBytes;
        uint8_t *dataBuffer;
    } flashStruct_t;
    
    void FH_initFlashStruct(flashStruct_t *data);
    bit FH_writeVerifyFlash(bool enableErase, flashStruct_t *wrData);
    uint32_t FH_getBootEndAddress(void);
    uint32_t FH_getAppEndAddress(void);
    bit FH_eraseAppSpace(void);
    bit FH_eraseStorageFLASHSpace();
    void FH_jumpToApp(uint16_t offset);
    
#ifdef	__cplusplus
}
#endif

#endif	/* FLASH_HELPER_H */

