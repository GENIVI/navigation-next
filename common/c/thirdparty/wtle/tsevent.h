/*****************************************************************************
*
*  tsevent.h - typedef for events
*
*  Copyright (C) 2007 Monotype Imaging Inc. All rights reserved.
*
*  Confidential Information of Monotype Imaging Inc.
*
****************************************************************************/

#ifndef TSEVENT_H
#define TSEVENT_H

TS_BEGIN_HEADER

/******************************************************************************
 *
 *  Enumeration of events.
 *
 *  Description:
 *
 *  <GROUP events>
 */
typedef enum
{
    TS_EVENT_NO_EVENT                           =   0,   /* used to indicate that it is not necessary to send an event message */

    TS_EVENT_LAYOUT_CONTROL                     = 100,
    TS_EVENT_LAYOUT_CONTROL_RECOMPOSE_NEEDED,           /* sent when a dictionary is added or removed */

    TS_EVENT_LAYOUT_OPTIONS                     = 200,
    TS_EVENT_LAYOUT_OPTIONS_RECOMPOSE_NEEDED,           /* sent when any of the following changes: an option in the TsLayoutOptionsTag enumeration (other than the draw options); line break model; default tab stop position; truncation character; hyphenation character */
    TS_EVENT_LAYOUT_OPTIONS_REDISPLAY_NEEDED,           /* sent when highlight mode, highlight background color, highlight text color, or any draw option changes */

    TS_EVENT_LAYOUT                             = 300,
    TS_EVENT_LAYOUT_RECOMPOSE_NEEDED,                   /* sent when the layout's TsLayoutControl, TsLayoutOptions, or TsText object changes; when the inline image callbacks change; or when the TsLayoutControl, TsLayOutOptions, TsText, or TsTextContainer objects send a message that requires recomposition */
    TS_EVENT_LAYOUT_REDISPLAY_NEEDED,                   /* sent when the TsText object or TsLayoutOptions object sends a message that requires redisplay */

    TS_EVENT_TEXT                               = 400,
    TS_EVENT_TEXT_INSERTION,
    TS_EVENT_TEXT_REMOVAL,
    TS_EVENT_TEXT_ATTRIBUTE_SHAPING_NEEDED,             /* an attribute changed that requires shaping to be redone (implies need for layout & redisplay) */
    TS_EVENT_TEXT_ATTRIBUTE_LAYOUT_NEEDED,              /* an attribute changed that requires line breaking, etc. to be redone (implies need for redisplay) */
    TS_EVENT_TEXT_ATTRIBUTE_REDISPLAY_NEEDED,           /* color, underline, strikethrough, overline attribute changed */

    TS_EVENT_TEXT_CONTAINER                     = 500,
    TS_EVENT_TEXT_CONTAINER_SIZE,                       /* container size has changed */
    TS_EVENT_TEXT_CONTAINER_MAX_LINES,                  /* the maximum number of lines allowed in the text container has changed */
    TS_EVENT_TEXT_CONTAINER_ANCHORED_OBJECT,            /* an anchored object has been added or removed from the text container */

    TS_EVENT_CLIENT                             = 600,
    TS_EVENT_CLIENT_INSERT_TEXT,                        /* text has been inserted into the client-owned text buffer */
    TS_EVENT_CLIENT_REMOVE_TEXT,                        /* text has been removed from the client-owned text buffer */

    TS_EVENT_MEMMGR                             = 700,
    TS_EVENT_MEMMGR_HEAP_FULL,                          /* sent by a TsMemMgr when a requested allocation would cause the amount of memory used to exceed the heap size  */
    TS_EVENT_MEMMGR_HEAP_RESIZE,                        /* sent by a TsMemMgr when a client has requested that the heap size be reduced and the new size is less than amount currently allocated */
    TS_EVENT_MEMMGR_HEAP_REDUCE                         /* sent by a TsMemMgr when client calls TsMemMgr_reduce */

} TsEvent;

/******************************************************************************
 *
 *  Prototype for the callback function that is called when
 *  something changes.
 *
 *  Parameters:
 *      observer    - [in] pointer to the object that is receiving the event message
 *      subject     - [in] the object that is producing the message
 *      event       - [in] the event that occurred that caused the message to be sent
 *      messagedata - [in] pointer to a structure that contains data pertinent
 *                          to the specific event that occurred
 *
 *  <GROUP events>
 */
typedef void (*TsEventCallback)(void *observer, void *subject, TsEvent event, void *messagedata);


/******************************************************************************
 *
 *  Structure that contains information about the observers of an object.
 *
 *  <GROUP events>
 */
typedef struct TsObserverInfo_
{
    void* observer;             /* pointer to WTLE object */
    TsEventCallback callback;   /* function to call when something changes */
} TsObserverInfo;

TS_END_HEADER

#endif /* TSEVENT_H */
