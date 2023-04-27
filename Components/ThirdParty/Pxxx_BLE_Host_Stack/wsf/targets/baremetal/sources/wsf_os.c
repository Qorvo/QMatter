/*************************************************************************************************/
/*!
 *  \file   wsf_os.c
 *
 *  \brief  Software foundation OS main module.
 *
 *  Copyright (c) 2009-2019 Arm Ltd. All Rights Reserved.
 *
 *  Copyright (c) 2019-2021 Packetcraft, Inc.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 * Copyright (c) 2021, Qorvo Inc
 *
 *
 *
 * This software is owned by Qorvo Inc
 * and protected under applicable copyright laws.
 * It is delivered under the terms of the license
 * and is intended and supplied for use solely and
 * exclusively with products manufactured by
 * Qorvo Inc.
 *
 *
 * THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 * CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 * IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 * LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * QORVO INC. SHALL NOT, IN ANY
 * CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 * FOR ANY REASON WHATSOEVER.
 */
/*************************************************************************************************/

#ifdef __IAR_SYSTEMS_ICC__
#include <intrinsics.h>
#endif
#include <string.h>
#include "wsf_types.h"
#include "wsf_os.h"
#include "wsf_assert.h"
#include "wsf_trace.h"
#include "wsf_timer.h"
#include "wsf_queue.h"
#include "wsf_buf.h"
#include "wsf_msg.h"
#include "wsf_cs.h"
#include "pal_frc.h"
#include "pal_sys.h"

/**************************************************************************************************
  Compile time assert checks
**************************************************************************************************/

WSF_CT_ASSERT(sizeof(uint8_t) == 1);
WSF_CT_ASSERT(sizeof(uint16_t) == 2);
WSF_CT_ASSERT(sizeof(uint32_t) == 4);

/**************************************************************************************************
  Macros
**************************************************************************************************/

/* maximum number of event handlers per task */
#ifndef WSF_MAX_HANDLERS
#define WSF_MAX_HANDLERS                          9
#endif

#if WSF_MAX_HANDLERS > 0x1F
#error: "WSF_MAX_HANDLERS cannot be larger as 0x0F"
#endif // WSF_MAX_HANDLERS > 0x0F

#if WSF_OS_DIAG == TRUE
#define WSF_OS_SET_ACTIVE_HANDLER_ID(id)          (WsfActiveHandler = id);
#else
#define WSF_OS_SET_ACTIVE_HANDLER_ID(id)
#endif /* WSF_OS_DIAG */
#define MAX_WSF_DISPATCHER_LOOPS 5

/*! \brief OS serivice function number */
#define WSF_OS_MAX_SERVICE_FUNCTIONS                  3

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief  Task structure */
typedef struct
{
  wsfEventHandler_t     handler[WSF_MAX_HANDLERS];
  wsfEventMask_t        handlerEventMask[WSF_MAX_HANDLERS];
  wsfQueue_t            msgQueue;
  volatile wsfTaskEvent_t taskEventMask;  /* The variable used in the critical section should have a type volatile */
  uint8_t               numHandler;
} wsfOsTask_t;

/*! \brief  OS structure */
typedef struct
{
  wsfOsTask_t           task;
  WsfOsIdleHandler_t    idleHandler[WSF_OS_MAX_SERVICE_FUNCTIONS];
  uint8_t               numIdle;
} wsfOs_t;

/**************************************************************************************************
  Local Variables
**************************************************************************************************/

/*! \brief  OS context. */
wsfOs_t wsfOs;

#if WSF_OS_DIAG == TRUE
/*! Active task handler ID. */
wsfHandlerId_t WsfActiveHandler;
#endif /* WSF_OS_DIAG */

extern uint8_t wsfCsNesting;

/*************************************************************************************************/
/*!
 *  \brief  Lock task scheduling.
 */
/*************************************************************************************************/
void WsfTaskLock(void)
{
  WsfCsEnter();
}

/*************************************************************************************************/
/*!
 *  \brief  Unock task scheduling.
 */
/*************************************************************************************************/
void WsfTaskUnlock(void)
{
  WsfCsExit();
}

/*************************************************************************************************/
/*!
 *  \brief  Set an event for an event handler.
 *
 *  \param  handlerId   Handler ID.
 *  \param  event       Event or events to set.
 */
/*************************************************************************************************/
void WsfSetEvent(wsfHandlerId_t handlerId, wsfEventMask_t event)
{
  WSF_CS_INIT(cs);

  WSF_ASSERT(WSF_HANDLER_FROM_ID(handlerId) < WSF_MAX_HANDLERS);

  WSF_TRACE_INFO2("WsfSetEvent handlerId:%u event:%u", handlerId, event);

  WSF_CS_ENTER(cs);
  wsfOs.task.handlerEventMask[WSF_HANDLER_FROM_ID(handlerId)] |= event;
  wsfOs.task.taskEventMask |= WSF_HANDLER_EVENT;
  WSF_CS_EXIT(cs);
}

/*************************************************************************************************/
/*!
 *  \brief  Set the task used by the given handler as ready to run.
 *
 *  \param  handlerId   Event handler ID.
 *  \param  event       Task event mask.
 */
/*************************************************************************************************/
void WsfTaskSetReady(wsfHandlerId_t handlerId, wsfTaskEvent_t event)
{
  /* Unused parameter */
  (void)handlerId;

  WSF_CS_INIT(cs);

  WSF_CS_ENTER(cs);
  wsfOs.task.taskEventMask |= event;
  WSF_CS_EXIT(cs);
}

void WsfTaskClearReady(wsfHandlerId_t handlerId, wsfTaskEvent_t event)
{
  /* Unused parameter */
  (void)handlerId;

  WSF_CS_INIT(cs);

  WSF_CS_ENTER(cs);
  wsfOs.task.taskEventMask &= (~event);
  WSF_CS_EXIT(cs);

  /* clear event in OS */
}

/*************************************************************************************************/
/*!
 *  \brief  Return the message queue used by the given handler.
 *
 *  \param  handlerId   Event handler ID.
 *
 *  \return Task message queue.
 */
/*************************************************************************************************/
wsfQueue_t *WsfTaskMsgQueue(wsfHandlerId_t handlerId)
{
  /* Unused parameter */
  (void)handlerId;

  return &(wsfOs.task.msgQueue);
}

/*************************************************************************************************/
/*!
 *  \brief  Set the next WSF handler function in the WSF OS handler array.  This function
 *          should only be called as part of the stack initialization procedure.
 *
 *  \param  handler    WSF handler function.
 *
 *  \return WSF handler ID for this handler.
 */
/*************************************************************************************************/
wsfHandlerId_t WsfOsSetNextHandler(wsfEventHandler_t handler)
{
  wsfHandlerId_t handlerId = wsfOs.task.numHandler++;

  WSF_ASSERT(handlerId < WSF_MAX_HANDLERS);

  wsfOs.task.handler[handlerId] = handler;

  return handlerId;
}

/*************************************************************************************************/
/*!
 *  \brief  Check if WSF is ready to sleep.  This function should be called when interrupts
 *          are disabled.
 *
 *  \return Return TRUE if there are no pending WSF task events set, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t WsfOsReadyToSleep(void)
{
  return (wsfOs.task.taskEventMask == 0);
}

/*************************************************************************************************/
/*!
 *  \brief  Execute idle tasks.
 *
 *  \return Return TRUE idle operations are active, FALSE otherwise.
 */
/*************************************************************************************************/
//static inline bool_t wsfOsRunIdleTasks(void)
//{
//  bool_t activeFlag = FALSE;
//
//  for (unsigned int i = 0; i < wsfOs.numIdle; i++)
//  {
//    if (wsfOs.idleHandler[i])
//    {
//      activeFlag |= wsfOs.idleHandler[i]();
//    }
//  }
//
//  return activeFlag;
//}

/*************************************************************************************************/
/*!
*  \brief  Initialize OS control structure.
*
*  \return None.
*/
/*************************************************************************************************/
void WsfOsInit(void)
{
  memset(&wsfOs, 0, sizeof(wsfOs));
  PalSysInit();
}

/*************************************************************************************************/
/*!
 *  \brief  Event dispatched.  Designed to be called repeatedly from infinite loop.
 */
/*************************************************************************************************/
uint32_t WsfOsIdleCount = 0;
extern uint8_t wsfCsNesting;

void WsfOsDispatcher(void)
{
  wsfOsTask_t       *pTask;
  void              *pMsg;
  wsfTimer_t        *pTimer;
  wsfEventMask_t    eventMask;
  wsfTaskEvent_t    taskEventMask;
  wsfHandlerId_t    handlerId;
  uint8_t           i;
  uint16_t          NumLoops = 0;

  WSF_CS_INIT(cs);

  pTask = &wsfOs.task;

  /* get and then clear task event mask */
  WSF_CS_ENTER(cs);
  taskEventMask = pTask->taskEventMask;
  pTask->taskEventMask = 0;
  WSF_CS_EXIT(cs);

  if (taskEventMask & WSF_MSG_QUEUE_EVENT)
  {
    /* clear the flag as soon as possible to allow it to be set from the ISR context */
    WsfTaskClearReady(WSF_INVALID_TASK_ID, WSF_MSG_QUEUE_EVENT);
    /* handle msg queue */
    while ((pMsg = WsfMsgDeq(&pTask->msgQueue, &handlerId)) != NULL)
    {
      WSF_ASSERT(handlerId < WSF_MAX_HANDLERS);
      WSF_OS_SET_ACTIVE_HANDLER_ID(handlerId);
      (*pTask->handler[handlerId])(0, pMsg);
      WsfMsgFree(pMsg);
      if (++NumLoops >= MAX_WSF_DISPATCHER_LOOPS) /* Wondering if would be better to decide this based on time instead of NumLoops */
      {
        /* We don't expect to be in a critical section or task lock. If not, we cannot return to
         * gpSched since this might lead to data corruption when new wsf events are created. */
        WSF_ASSERT(wsfCsNesting == 0);
        /* Some events may not have been processed, restoring flags */
        pTask->taskEventMask = taskEventMask;
        return;
      }
    }
    /* All events have been processed, removing local flags */
    taskEventMask &= (~WSF_MSG_QUEUE_EVENT);
  }

  if (taskEventMask & WSF_TIMER_EVENT)
  {
    /* clear the flag as soon as possible to allow it to be set from the ISR context */
    WsfTaskClearReady(WSF_INVALID_TASK_ID, WSF_TIMER_EVENT);
    /* service timers */
    while ((pTimer = WsfTimerServiceExpired(0)) != NULL)
    {
      WSF_ASSERT(pTimer->handlerId < WSF_MAX_HANDLERS);
      WSF_OS_SET_ACTIVE_HANDLER_ID(pTimer->handlerId);
      (*pTask->handler[pTimer->handlerId])(0, &pTimer->msg);
      if (++NumLoops >= MAX_WSF_DISPATCHER_LOOPS)
      {
        WSF_ASSERT(wsfCsNesting == 0); /* We don't expect to be in a critical section or task lock */
        /* Some events may not have been processed, restoring flags */
        pTask->taskEventMask = taskEventMask;
        return;
      }
    }
    /* All events have been processed, removing local flags */
    taskEventMask &= (~WSF_TIMER_EVENT);
  }
  if (taskEventMask & WSF_HANDLER_EVENT)
  {
    /* clear the flag as soon as possible to allow it to be set from the ISR context */
    WsfTaskClearReady(WSF_INVALID_TASK_ID, WSF_HANDLER_EVENT);
    /* service handlers */
    for (i = 0; i < WSF_MAX_HANDLERS; i++)
    {
      if ((pTask->handlerEventMask[i] != 0) && (pTask->handler[i] != NULL))
      {
        WSF_CS_ENTER(cs);
        eventMask = pTask->handlerEventMask[i];
        pTask->handlerEventMask[i] = 0;
        WSF_OS_SET_ACTIVE_HANDLER_ID(i);
        WSF_CS_EXIT(cs);

        (*pTask->handler[i])(eventMask, NULL);
      }
    }
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Register idle task routine.
 *
 *  \param  cback   Idle task.
 */
/*************************************************************************************************/
void WsfOsRegisterIdleTask(WsfOsIdleHandler_t cback)
{
  wsfOs.idleHandler[wsfOs.numIdle++] = cback;
}

/*************************************************************************************************/
/*!
 *  \brief  Baremetal main loop.
 */
/*************************************************************************************************/
void WsfOsEnterMainLoop(void)
{
  //QORVO main loop is used instead
}
