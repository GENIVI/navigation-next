/*****************************************************************************
 *
 * Filename:    forensics.h
 *
 * Created:     July 22, 2011
 *
 * Description: Public API's for QUIP forensics logworthy events
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

#ifndef FORENSICS_H_
#define FORENSICS_H_

#include <stdint.h>
#include <quip.h>
#include <log_types.h> 

/**
 * Forensics API Version History
 *
 * 1.4 - Added asynchronous logworthy calls, disable file/blob fields, "appended" logworthies
 * 1.3 - Changes to crash logworthy details fields
 * 1.2 - QuipLogworthyAttachmentHeader added to a logworthy attachment event.
 * 1.1 - Details fields added to protocol buffers, legacy fields removed.
 * 1.0 - Initial
 */
#define FORENSICS_API_MAJOR			(1)
#define FORENSICS_API_MINOR			(4)

#if FORENSICS_API_MAJOR	>= 16
#error "FORENSICS_API_MAJOR is limitied to a maximum value of 15!"
#endif

#if FORENSICS_API_MINOR >= 1024
#error "FORENSICS_API_MINOR is limited to a maximum value of 4095"
#endif

#define FORENSICS_API_VERSION		( FORENSICS_API_MAJOR << 12 | FORENSICS_API_MINOR )

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Return codes for forensics library - Values from errno.h are used
 */
typedef int forensics_err_t;

/**
 * Logworthy Event Type - A subset of the Universal QUIP Event Type
 *
 * This includes Logworthy Event Types, and other Types to identify various
 * uses like BugReport, Counters, CCL, etc.)
 *
 * These values, along with descriptions and other meta data, are defined
 * and stored in a central dictionary.  A C header file is exported to make
 * the definitions available in the code.  Not available yet though.
 *
 * Note that this is similar to the old Quincy "SubMsg ID"
 */
typedef uint32_t forensics_event_t;

/**
 * Logworthy Event Details
 *
 * Details allow more granular clasification/catagorization of a Logworthy
 * Event.  This allows for differentiation of two events with same Logworthy Event Type
 * (forensics_event_t) based on varying root caues.
 */
typedef struct {
	uint32_t details1;  ///< Details Field 1
	uint32_t details2;  ///< Details Field 2
	uint32_t details3;  ///< Details Field 3
	uint32_t details4;  ///< Details Field 4
} forensics_details_t;

/** 
 * Event information struct passed back to a user callback on completion 
 * of an asynchronous API call.
 */
typedef struct
{
    /** Error code returned by the logworthy API.  ALWAYS check this. */
    forensics_err_t status;
    
	/** Global event ID for the logworthy event */
	quip_event_id_t logworthy_event_id;

    /** EventType of the event (specified in eventtype.h) */
	forensics_event_t event_type;

	/** Set to 1 if the event was throttled (no logworthy attachment sent) */
	uint8_t event_throttled;

} forensics_logworthy_event_info_t;

/**
 * Asynchronous logworthy callback function pointer type
 *
 * Allows the user to provide a notification mechanism to a process after an 
 * asynchronous logworthy event has completed.  The out_args parameter will
 * contain a copy of the output parameters obtained from the logworthy call
 * itself, including the return status of the logworthy call, and the event
 * header/payload event IDs.
 */
typedef void (*forensics_logworthy_callback_t)( forensics_logworthy_event_info_t *event_info );

/**
 * @mainpage QUIP Forensics Logworthy API
 *  @section function_sec Functions
 *    forensics_logworthy() - Trigger a forensics logworthy event
 *    forensics_logworthy_async() - Trigger an asynchronous logworthy event 
 *                                  in its own thread
 */
/**
 * Trigger a Forensics Logworthy Event
 *
 * Resulting functionality depends on security/permissions settings.
 * For example:
 * <ul>
 * <li>End Customer (default): add an entry to device history (logs not captured)
 * <li>RIM User or Partner: capture logs, send to QUIP Cloud for analysis; device
 *     history is also updated
 * </ul>
 *
 * @param on_by_default TRUE means that Logworthy Event will be triggered; FALSE
 *        means that Logworthy will NOT be triggered unless someone has configured
 *        the device at runtime to trigger it (locally through a diagnostics app
 *        or remotely through and admin console).  This enables the developer to
 *        instrument the code, but at a later time enable the Logworthy to
 *        facilitate investigation or analysis that should not impact all users.
 *        One example would be to remotely enable a Logworthy Event for a specific
 *        beta group.
 *
 * @param event_type The type of event (eg. seg fault or call drop or reset)
 *
 * @param details Optional pointer to event details;  Detials field provides
 *        info helpful for classifying and comparing two Logworthy Events with the
 *        same eventType.
 *
 * @param log_sets NULL-terminated list of Bones Log Sets.  Bones has an
 *        internal definition for what logs are to be included in each Set.  Note
 *        that the union of these Sets and logList is taken to produce the final
 *        group of logs that are captured and included as part of the Logworthy
 *        Event Payload.
 *
 * @param logs NULL-terminated list of specific logs to include in the logworthy.
 *        Most users should not use this parameter; it is intended for specialized
 *        use in specialized situations.  Instead, the preference is to
 *        use logSetList, which will automagically provide good set of logs without
 *        expert knowledge of what logs to choose... or FOREKNOWLEDGE regarding
 *        new applicable logs that may become available in the future. Note
 *        that the final list of logs included in the Logworthy Event Payload is a
 *        union of what is specified by logSetList and logList (overlap will be
 *        correctly handled, though may indicate that that specifying logList was
 *        unnecessary).
 *
 * @param blob Legacy parameter - disabled
 *
 * @param blob_size Legacy parameter - disabled
 *
 * @param file_path_list Legacy parameter - disabled
 */
forensics_err_t
forensics_logworthy( uint8_t on_by_default,
                forensics_event_t event_type,
                forensics_details_t *details,
                forensics_log_set_t *log_sets,
                forensics_log_t *logs,
                void *blob,
                int blob_size,
                char **file_path_list
              );

    
/**
 * Trigger a Forensics Logworthy Event on its own Thread
 *
 * Resulting functionality depends on security/permissions settings.
 * For example:
 * <ul>
 * <li>End Customer (default): add an entry to device history (logs not captured)
 * <li>RIM User or Partner: capture logs, send to QUIP Cloud for analysis; device
 *     history is also updated
 * </ul>
 *
 * @param on_by_default TRUE means that Logworthy Event will be triggered; FALSE
 *        means that Logworthy will NOT be triggered unless someone has configured
 *        the device at runtime to trigger it (locally through a diagnostics app
 *        or remotely through and admin console).  This enables the developer to
 *        instrument the code, but at a later time enable the Logworthy to
 *        facilitate investigation or analysis that should not impact all users.
 *        One example would be to remotely enable a Logworthy Event for a specific
 *        beta group.
 *
 * @param event_type The type of event (eg. seg fault or call drop or reset)
 *
 * @param details Optional pointer to event details;  Detials field provides
 *        info helpful for classifying and comparing two Logworthy Events with the
 *        same eventType.
 *
 * @param log_sets NULL-terminated list of Bones Log Sets.  Bones has an
 *        internal definition for what logs are to be included in each Set.  Note
 *        that the union of these Sets and logList is taken to produce the final
 *        group of logs that are captured and included as part of the Logworthy
 *        Event Payload.
 *
 * @param logs NULL-terminated list of specific logs to include in the logworthy.
 *        Most users should not use this parameter; it is intended for specialized
 *        use in specialized situations.  Instead, the preference is to
 *        use logSetList, which will automagically provide good set of logs without
 *        expert knowledge of what logs to choose... or FOREKNOWLEDGE regarding
 *        new applicable logs that may become available in the future. Note
 *        that the final list of logs included in the Logworthy Event Payload is a
 *        union of what is specified by logSetList and logList (overlap will be
 *        correctly handled, though may indicate that that specifying logList was
 *        unnecessary).
 *
 * @param callback Pointer to the callback function to be called once the logworthy 
 *        event has been completed.  If NULL, no callback will be generated.
 *
 * @return 0 if the logworthy thread was created successfully, -1 on error starting
 *         asynchronous call.  Note - the logworthy event return code is only passed
 *         to the callback function.
 */
int
forensics_logworthy_async(uint8_t on_by_default,
                            forensics_event_t event_type,
                            forensics_details_t *details,
                            forensics_log_set_t *log_sets,
                            forensics_log_t *logs,
                            forensics_logworthy_callback_t callback );

/**
 * Append additional logworthy data to a previous logworthy event.
 *
 * @param parent_event Pointer to a quip_event_id_t object containing the unique
 *        ID of an existing logworthy event's header.  This is typically captured
 *        in the forensics_output_t parameter passed to an asynchronous logworthy's
 *        callback function.
 *
 * @param log_sets NULL-terminated list of Bones Log Sets.  Bones has an
 *        internal definition for what logs are to be included in each Set.  Note
 *        that the union of these Sets and logList is taken to produce the final
 *        group of logs that are captured and included as part of the Logworthy
 *        Event Payload.
 *
 * @param logs NULL-terminated list of specific logs to include in the logworthy.
 *        Most users should not use this parameter; it is intended for specialized
 *        use in specialized situations.  Instead, the preference is to
 *        use logSetList, which will automagically provide good set of logs without
 *        expert knowledge of what logs to choose... or FOREKNOWLEDGE regarding
 *        new applicable logs that may become available in the future. Note
 *        that the final list of logs included in the Logworthy Event Payload is a
 *        union of what is specified by logSetList and logList (overlap will be
 *        correctly handled, though may indicate that that specifying logList was
 *        unnecessary).
 *
 * @return Return code from the logworthy event.  Corresponds to the standard POSIX
 *         errno values. 
 */
forensics_err_t
forensics_logworthy_append(   quip_event_id_t *parent_event,
                              forensics_log_set_t *log_sets,
                              forensics_log_t *logs );                            

/**
 * Asynchronously append additional logworthy data to a previous logworthy event.
 *
 *a This call creates its own thread to generate the logworthy event, and will call 
 * the user-supplied callback on completion of the event.
 *
 * @param parent_event Pointer to a quip_event_id_t object containing the unique
 *        ID of an existing logworthy event's header.  This is typically captured
 *        in the forensics_output_t parameter passed to an asynchronous logworthy's
 *        callback function.
 *
 * @param log_sets NULL-terminated list of Bones Log Sets.  Bones has an
 *        internal definition for what logs are to be included in each Set.  Note
 *        that the union of these Sets and logList is taken to produce the final
 *        group of logs that are captured and included as part of the Logworthy
 *        Event Payload.
 *
 * @param logs NULL-terminated list of specific logs to include in the logworthy.
 *        Most users should not use this parameter; it is intended for specialized
 *        use in specialized situations.  Instead, the preference is to
 *        use logSetList, which will automagically provide good set of logs without
 *        expert knowledge of what logs to choose... or FOREKNOWLEDGE regarding
 *        new applicable logs that may become available in the future. Note
 *        that the final list of logs included in the Logworthy Event Payload is a
 *        union of what is specified by logSetList and logList (overlap will be
 *        correctly handled, though may indicate that that specifying logList was
 *        unnecessary).
 * @return 0 on successful kickoff of asynchronous logworthy thread, -1 on error to
 *        create asynchronous logworthy thread.
 */
int
forensics_logworthy_append_async(   quip_event_id_t *parent_event,
                                    forensics_log_set_t *log_sets,
                                    forensics_log_t *logs,
                                    forensics_logworthy_callback_t callback );                            
                            
                            
#ifdef __cplusplus
}
#endif

#endif /* FORENSICS_H_ */

#include <sys/srcversion.h>
__SRCVERSION( "$URL$ $Rev$" )
