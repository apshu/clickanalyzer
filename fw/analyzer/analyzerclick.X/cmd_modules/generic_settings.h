/* 
 * File:   generic_settings.h
 * Author: M91541
 *
 * Created on May 11, 2017, 2:48 PM
 */

#ifndef GENERIC_SETTINGS_H
#define	GENERIC_SETTINGS_H

#ifdef	__cplusplus
extern "C" {
#endif

    bool CMDGET_onCommand(const char* cmdLine);
    bool CMDSET_onCommand(const char* cmdLine);

#ifdef	__cplusplus
}
#endif

#endif	/* GENERIC_SETTINGS_H */

