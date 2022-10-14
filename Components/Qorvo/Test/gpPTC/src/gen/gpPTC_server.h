/*
 *   Copyright (c) 2016, GreenPeak Technologies
 *
 *   This file contains the implementation of the gpPTC API protocol.
 *
 *   gpPTC
 *   gpPTC server definition
 *
 *                ,               This software is owned by GreenPeak Technologies
 *                g               and protected under applicable copyright laws.
 *               ]&$              It is delivered under the terms of the license
 *               ;QW              and is intended and supplied for use solely and
 *               G##&             exclusively with products manufactured by
 *               N#&0,            GreenPeak Technologies.
 *              +Q*&##
 *              00#Q&&g
 *             ]M8  *&Q           THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 *             #N'   Q0&          CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 *            i0F j%  NN          IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 *           ,&#  ##, "KA         LIMITED TO, IMPLIED WARRANTIES OF
 *           4N  NQ0N  0A         MERCHANTABILITY AND FITNESS FOR A
 *          2W',^^ `48  k#        PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *         40f       ^6 [N        GREENPEAK TECHNOLOGIES B.V. SHALL NOT, IN ANY
 *        jB9         `, 0A       CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 *       ,&?             ]G       INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 *      ,NF               48      FOR ANY REASON WHATSOEVER.
 *      EF                 @
 *     0!                         $Header$
 *    M'   GreenPeak              $Change$
 *   0'         Technologies      $DateTime$
 *  F
 */


#ifndef _GPPTC_SERVER_H_
#define _GPPTC_SERVER_H_

/*****************************************************************************
 *                    Parameter checking stubs for requests
 *****************************************************************************/


/*****************************************************************************
 *                    Public Function Declarations
 *****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void gpPTC_InitServer(void);
void gpPTC_DeInitServer(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_GPPTC_SERVER_H_

