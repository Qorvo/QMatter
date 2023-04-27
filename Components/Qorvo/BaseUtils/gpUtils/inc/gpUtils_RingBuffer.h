/*
 *   Copyright (c) 2020, Qorvo Inc
 *
 *   This software is owned by Qorvo Inc
 *   and protected under applicable copyright laws.
 *   It is delivered under the terms of the license
 *   and is intended and supplied for use solely and
 *   exclusively with products manufactured by
 *   Qorvo Inc.
 *
 *
 *   THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 *   CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 *   IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 *   LIMITED TO, IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *   QORVO INC. SHALL NOT, IN ANY
 *   CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 *   INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 *   FOR ANY REASON WHATSOEVER.
 *
 *   $Header$
 *   $Change$
 *   $DateTime$
 */

#ifndef _GP_UTILS_RING_BUFFER_H
#define _GP_UTILS_RING_BUFFER_H

/*******************************************************************************
 *                      Include Files
 ******************************************************************************/

#include "global.h"

/*******************************************************************************
 *                    Defines
 ******************************************************************************/

/*
 * Declare a static ring buffer variable. This defines a circular buffer that
 * can be used with the other macro's in this file.
 *
 * @param name: name of the static variable
 * @param type: type of the buffer elements
 * @param size: number of elements (usable elements is one less than this parameter)
 */
#define GP_UTILS_RING_BUFFER(name, type, size)  typedef struct gpRingBuffer_##name {            \
    size_t start;                                                                               \
    size_t end;                                                                                 \
    type data[size];                                                                            \
} name##_t;                                                                                     \
static name##_t name = {0};

/*
 * Check whether ring buffer is full.
 *
 * @param buffer: ring buffer declared with GP_RING_BUFFER
 * @returns: true if the buffer is full, false otherwise
 */
#define gpUtils_RingBufferFull(buffer)   ( ((buffer.end + 1) %                                  \
                        number_of_elements(buffer.data)) == buffer.start )

/*
 * Check whether ring buffer is empty.
 *
 * @param buffer: ring buffer
 */
#define gpUtils_RingBufferEmpty(buffer)  ( buffer.end == buffer.start )

/*
 * Remove least-recently added item from ring buffer.
 *
 * @param buffer: ring buffer declared with GP_RING_BUFFER
 */
#define gpUtils_RingBufferDiscardTail(buffer)   do {                                            \
    if (!gpUtils_RingBufferEmpty(buffer))                                                       \
    {                                                                                           \
        buffer.start = (buffer.start + 1) % number_of_elements(buffer.data);                    \
    }                                                                                           \
} while(false)

/*
 * Add (copy) an element in the ring buffer (if not full)
 *
 * @param buffer: ring buffer declared with GP_RING_BUFFER
 * @param element: element to copy into buffer
 */
#define gpUtils_RingBufferPut(buffer, element)   do {                                           \
    if (!gpUtils_RingBufferFull(buffer))                                                        \
    {                                                                                           \
        buffer.data[buffer.end] = element;                                                      \
        buffer.end = (buffer.end + 1) % number_of_elements(buffer.data);                        \
    }                                                                                           \
} while(false)

/*
 * Advance the ring buffer last item index (if not full).
 *
 * This advances the ring buffer end index, and is intended to be used
 * in cases where the caller wants to set the element directly, such as:
 *
 * GP_RING_BUFFER_NEW(buffer).foo = 42;
 * GP_RING_BUFFER_ADVANCE(buffer)
 *
 * The above code could be written less efficiently as:
 *
 * myElementType element;
 * element.foo = 42;
 * GP_RING_BUFFER_PUT(buffer, element);
 *
 * @param buffer: ring buffer declared with GP_RING_BUFFER
 */
#define gpUtils_RingBufferAdvance(buffer)    do {                                               \
    if (!gpUtils_RingBufferFull(buffer))                                                        \
    {                                                                                           \
        buffer.end = (buffer.end + 1) % number_of_elements(buffer.data);                        \
    }                                                                                           \
} while(false)

/*
 * Access the head of the ring buffer (the least recently added element).
 *
 * @param buffer: ring buffer declared with GP_RING_BUFFER
 */
#define gpUtils_RingBufferHead(buffer) buffer.data[buffer.start]

/*
 * Access the element that would be added by calling GP_RING_BUFFER_PUT.
 * Care must be taken not to write to this element if the buffer is full.
 *
 * @param buffer: ring buffer declared with GP_RING_BUFFER
 */
#define gpUtils_RingBufferNew(buffer) buffer.data[buffer.end]

/*
 * Reset ring buffer. After this, the ring buffer is empty.
 *
 * @param buffer: ring buffer declared with GP_RING_BUFFER
 */
#define gpUtils_RingBufferReset(buffer)  do {                                                   \
    buffer.start = buffer.end = 0;                                                              \
} while(false)

/*******************************************************************************
 *                    Type Definitions
 ******************************************************************************/

typedef struct gpRingBuffer {
    size_t start;
    size_t end;
    size_t elementSize;
    void* data;
} gpRingBuffer_t;

/******************************************************************************
 *                    Public Function Definitions
 ******************************************************************************/


#endif /* _GP_UTILS_RING_BUFFER_H */
