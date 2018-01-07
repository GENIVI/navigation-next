/*****************************************************************************
 *
 * Filename:    quip.h
 *
 * Created:     August 25, 2011
 *
 * Description: Public QUIP base type declarations
  *
 * Copyright 2011, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable license fees to QNX
 * Software Systems before you may reproduce, modify or distribute this software,
 * or any work that includes all or part of this software.   Free development
 * licenses are available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *
 * This file may contain contributions from others.  Please review this entire
 * file for other proprietary rights or license notices, as well as the QNX
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/
 * for other information.
 *
 ****************************************************************************/

#ifndef QUIP_H_
#define QUIP_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @typedef _quip_event_creator_type_t
* Available Createor (App) ID Types.
*/
typedef enum
{
     QUIP_CREATOR_ID_TYPE_PROCESSNAME,
     QUIP_CREATOR_ID_TYPE_APPVERSION,
     QUIP_CREATOR_ID_TYPE_MAX
} _quip_event_creator_type_t;
typedef uint32_t quip_event_creator_type_t;

//---------------------------------------------------------------------------
/**
 * Structure containing various possible runtime types for a process
 */
typedef enum
{
    RUNTIME_UNKNOWN = 0,
    RUNTIME_NATIVE,
    RUNTIME_AIR,
    RUNTIME_WEBWORKS,
    RUNTIME_ANDROID,
    RUNTIME_PYTHON,
    RUNTIME_CASCADES,
    RUNTIME_NUM
}runtime_t;

/** @typdef quip_event_creator_t
* Generic application ID structure
*/
typedef struct _quip_event_creator_t {
    /** pointer to a null-terminated string containing the ID */
    const char *id;
    /** the runtime type of the process */
    runtime_t runtime_type;
    /** type of ID provided */
    quip_event_creator_type_t id_type;
    /** flag for a system component - 1 for true, 0 for false */
    int is_sys_component;
    /** flag for a developer-built app - 1 for true, 0 for false */
    int is_dev_build;
    /** version as in BAR file */
    const char *version;
    /** version ID as in BAR file */
    const char *version_id;
    /** application name as in BAR file */
    const char *name;
} quip_event_creator_t;

/** @typdef quip_eventId_t
 * A unique ID for each QUIP event
 */
typedef struct _quip_event_id_t {
    /** UUID raw bytes. */
    unsigned int id[4];
}quip_event_id_t;

/** @typedef quip_payload_type_t
 * Available Payload Types.
 */
typedef enum _quip_payload_type_t {
    QUIP_PAYLOAD_TYPE_NONE = 0,
    QUIP_PAYLOAD_TYPE_ACTIVATION,
    QUIP_PAYLOAD_TYPE_COMPONENTUPDATE,
    QUIP_PAYLOAD_TYPE_USERINFOUPDATE,
    QUIP_PAYLOAD_TYPE_HEARTBEAT,
    QUIP_PAYLOAD_TYPE_ANNOTATION,
    QUIP_PAYLOAD_TYPE_LOGWORTHY,
    QUIP_PAYLOAD_TYPE_COUNTERS,
    QUIP_PAYLOAD_TYPE_LOGWORTHYATTACHMENT,
    QUIP_PAYLOAD_TYPE_LOGWORTHYMEGA,
    QUIP_PAYLOAD_TYPE_MAX
}quip_payload_type_t;

/** @typedef quip_delivery_attr_t
 * Delivery Properties of an event.
 */
typedef struct _quip_delivery_attr_t {
    /** Time To Live, delete the file after ttl hours. */
    unsigned char ttl;
}quip_delivery_attr_t;

/** Tells the QUIP to always keep this event */
#define QUIP_DELIVERY_ATTR_TTL_NEVERDELETEBEFORESEND 0x00
/** Tells the QUIP to keep this event only until it is sent */
#define QUIP_DELIVERY_ATTR_TTL_DELETEIMMEDIATELY 0xFF

#ifdef __cplusplus
}
#endif

#endif /* QUIP_H_ */
