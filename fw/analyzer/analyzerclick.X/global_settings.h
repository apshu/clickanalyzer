/* 
 * File:   global_settings.h
 * Author: M91541
 *
 * Created on June 1, 2017, 5:00 PM
 */

#ifndef GLOBAL_SETTINGS_H
#define	GLOBAL_SETTINGS_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        GLOBAL_acq_mode_single,
        GLOBAL_acq_mode_continous,
    } GLOBAL_acq_mode_t;

    typedef enum {
        GLOBAL_output_mode_binary,
        GLOBAL_output_mode_JSON,
        GLOBAL_output_mode_XTERM,
        GLOBAL_output_mode_INVALID,
    } GLOBAL_output_mode_t;
    
    GLOBAL_acq_mode_t inline GLOBAL_getAcquisitionMode();
    GLOBAL_output_mode_t inline GLOBAL_getOutputMode();
    bool GLOBAL_setOutputMode(GLOBAL_output_mode_t newOutputMode);
    void inline GLOBAL_setSampleBufByteSize(size_t numBytes); /* Store number of samples */
    size_t inline GLOBAL_getSampleBufByteSize(); /* Get number of samples set */
    uint8_t* GLOBAL_claimSamplingBuffer();  /* Buffer for samples */
    size_t GLOBAL_getMaxSampleBufBytes(void);  /* Total sample bytes */
    void GLOBAL_toggleAcquisitionMode(void);
    void inline GLOBAL_setAcquisitionMode(GLOBAL_acq_mode_t newMode);
    inline bit GLOBAL_isStopRequested(void);
    inline void GLOBAL_setStopRequest(bool isStopNeeded);
    
    void GLOBAL_onMouse(uint16_t mouseX, uint16_t mouseY, uint16_t mouseEvent);
#ifdef	__cplusplus
}
#endif

#endif	/* GLOBAL_SETTINGS_H */

