/***************************************************************************** * * Filename:    lw_event_types.h * * Description: Public logworthy event types. * * Copyright 2011, QNX Software Systems. All Rights Reserved. * * You must obtain a written license from and pay applicable license fees to QNX
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

#ifndef LW_EVENT_TYPES_H_
#define LW_EVENT_TYPES_H_

/**
 * Forensics Logworthy Event Types
 *
 * Note: Comments in this file follow a specific format necessary to
 *       properly document each Event Type.
 *
 * Comment Format:
 *
 * #define LW_EVENT_EXAMPLE_DOCS
 *     ///< First line(s) are the brief synopsis describing the event
 *     ///< ...may continue multiple lines.
 *     ///< Severity: Serious
 *     ///< D1: Name A: Synopsis of Details1 field...
 *     ///<            ...may continue to next line
 *     ///< D2: Name B: Synopsis of Details2 field
 *     ///< D3: Name C: Synopsis of Details3 field
 *     ///< D4: Name D: Synopsis of Details4 field
 *
 * Further notes:
 *
 * Severity (IE. "severity of issue experienced by the end user"), is one of:
 *    Catastrophic  - Full system hardware reset
 *    Critical      - Process crash or other total component failure (includes radio/modem reset)
 *    Serious       - Error that seriously impacts user (likely to be top call/return
 *                    driver with end customers; EG. Call Drop, UI Lag > 800ms)
 *    Warning       - Error that has "minor" impact on user (user able to recover;
 *                    EG: USB connection failure solved by re-connecting)
 *    Unknown       - Unable to determine Severity level on the device; server-side
 *                    processing will be required to determine severity level.
 *                    Use of this severity level is strongly discouraged and is
 *                    primarily available for Radio Call Drop analysis.
 *    Debug         - Used for events that are either "Off by Default" or used strictly for
 *                    testing, experimentation and short term investigations.
 *
 * D1, D2, D3, D4:
 *  - Correspond to the four Details fields associated with the Logworthy Event
 *  - These fields, along with the Event Type, comprise the basice "signature" that
 *    uniquely identifies the Event.
 *  - Name A-D are labels for each Details field; these may be used as labels reports
 *  - Synopsis is given for each Details field... it is likely a technical description
 *  - Details fields are optional.  Do not specify any fields that are unused (this should
 *    also imply that the unused fields will always be set to zero).
 */
//@{

#define LW_EVENT_RESET_KERNEL                           0x00000100
    ///< Device reset triggered by catastrophic kernel error
    ///< Severity: Catastrophic
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_PROCESS_CRASH                          0x00000101
    ///< Process or application crash
    ///< Severity: Critical
    ///< D1: Signal: Signal that triggered the crash
    ///< D2: What/Why: (What << 16) | Why.
    ///<     The signal number, fault number, or exit status associated
    ///<     with the signal/fault that terminated the process.

#define LW_EVENT_USER_TRIGGERED                         0x00000102
    ///< User-triggered Logworthy aka "Bug Report".
    ///< Severity: Serious

#define LW_EVENT_TEST                                   0x00000103
    ///< Can be used by any process for testing or experimentation.
    ///< Please use this event when you are first trying out the
    ///< logworthy_event() API.  Note that this event type is used
    ///< extensively for QUIP testing purposes but can be used by
    ///< other components as well.
    ///< Severity: Debug
    ///< D1: User-specified 1: varies according to use
    ///< D2: User-specified 2: varies according to use
    ///< D3: User-specified 3: varies according to use
    ///< D4: User-specified 4: varies according to use

#define LW_EVENT_BLUETOOTH_HCI_TRANSPORT_ERROR          0x00000104
    ///< BlueTooth host-controller interface transport error occurred.  This causes the active
    ///< BlueTooth connection to be dropped.
    ///< Severity: Serious
    ///< D1: HCI Error code: One of HCI_TRANSERROR_* values.

#define LW_EVENT_QUIP_EXIT                              0x00000105
    ///< This event is used to signal the quip exited intentionally
    ///< Severity: Warning

#define LW_EVENT_QUIP_INVALID_EVENT_CREATION_TIME       0x00000106
    ///< Event creation time indicated an event in the future, and has been 'fixed' by setting it to the current time
    ///< Severity: Warning
    ///< D1: Reason: 0 Undefined, 1 RTC was unreliable, 2 event had an invalid time

#define LW_EVENT_INVALID_EVENT_TYPE                     0x00000107
    ///< When an invalid logworthy event type is passed into the logworthy API,
    ///< the library will intercept the invalid logworthy attempt, and instead
    ///< send a logworthy event indicating that a user attempted to send an
    ///< invalid logworthy.
    ///< Severity: Warning
    ///< D1: Invalid Type: The invalid QUIP event type sent to the logworthy API

#define LW_EVENT_NET_MULTI_NET_CONNECTED                0x00000108
    ///< Network packets may be routed to the wrong interface due to multiple
    ///< net_connected messages received for the same interface.  This is an
    ///< illegal use of the API.
    ///< Severity: Warning

#define LW_EVENT_BLUETOOTH_STACK_LOCK                   0x00000109
    ///< BlueTooth The Bluetooth stack had deadlocked. This likely means that the user will lose
    ///< BlueTooth Bluetooth connectivity and the stack will become extremely sluggish.
    ///< Severity: Serious

#define LW_EVENT_BLUETOOTH_ASSERT                       0x00000110
    ///< BlueTooth An assert has been called from the Bluetooth stack, this may invalidate internal
    ///< BlueTooth data and may show root cause for a core.
    ///< Severity: Serious

#define LW_EVENT_PROCESS_CRASH_DURING_SHUTDOWN          0x00000111
    ///< Process crash while device is shutting down
    ///< Severity: Serious
    ///< D1: Signal: Signal that triggered the crash
    ///< D2: What/Why: (What << 16) | Why.
    ///<     The signal number, fault number, or exit status associated
    ///<     with the signal/fault that terminated the process.

#define LW_EVENT_TEMP_INVESTIGATION                     0x00000112
    ///< Temporary investigation of an issue.  Can be used by any software component, similar to LW_EVENT_TEST.
    ///< Each software component defines its own meaning for the details fields.
    ///< Severity: Unknown
    ///< D1: User-specified 1: varies according to use
    ///< D2: User-specified 2: varies according to use
    ///< D3: User-specified 3: varies according to use
    ///< D4: User-specified 4: varies according to use

// Add new LW_EVENT_* types here

//@}

/**
 * Range of Event Types reserved for OEMs
 *
 * It is recommended to create a separate header file defining these events.
 */
#define LW_EVENT_OEM_RANGE_BEGIN 0x500
#define LW_EVENT_OEM_RANGE_END   0xfff
    
#endif /* LW_EVENT_TYPES_H_ */


#include <sys/srcversion.h>
__SRCVERSION( "$URL$ $Rev$" )
