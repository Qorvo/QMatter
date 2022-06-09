/**
 * @file app_common.h
 *
 * Header to define the common application definitions
 *
 */

#ifndef _APP_COMMON_H_
#define _APP_COMMON_H_

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/


#define LED_INDICATOR_TOGGLE() HAL_LED_TGL_GRN()
#define LED_INDICATOR_ON()     HAL_LED_SET(GRN)
#define LED_INDICATOR_OFF()    HAL_LED_CLR(GRN)


#endif //_APP_COMMON_H_
