/* 
 * File:   reset_all.h
 * Author: M91541
 *
 * Created on May 9, 2017, 12:12 PM
 */

#ifndef RESET_ALL_H
#define	RESET_ALL_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        RST_cmd_stop = '|', /* Stop pending transaction */
        RST_cmd_resetCommand = '@', /* Hard stop current operation */
        RST_cmd_rebootDevice = '#', /* Shutdown and restart. */

    };

    bool RST_onCommand(const char* cmdLine);
    bool RST_onNewChar(char newChar);

#ifdef	__cplusplus
}
#endif

#endif	/* RESET_ALL_H */

