/*****************************************************************************
 * lw_event_types_rim.h
 *
 * Description: Public Logworthy Event Types.
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

#ifndef LW_EVENT_TYPES_RIM_H_
#define LW_EVENT_TYPES_RIM_H_

/**
 * Forensics Logworthy Event Types (OEM-specific)
 *
 * This file defines all RIM-specific Event Types
 *
 * A types must be between the OEM ramge:
 *    LW_EVENT_OEM_RANGE_BEGIN 0x500
 *    LW_EVENT_OEM_RANGE_END   0xfff
 *
 * See svn/repos/product/trunk/lib/forensics/public/lw_event_types.h
 *  - Event Types provided in the base QNX platform
 *  - Instructions for formatting comments
 *
 * Proposal for carving up this Event Type range:
 *
 * 0x500-0x9ff BBX System (platform services & drivers in internal-source)
 * 0xa00-0xaff Radio - Cellular (includes Quincy 3GPP 1000-1999 and CDMA 2000-2999 codes)
 *                   - Will include Call Drop and SIM Card Errors
 *                   - Will include Call Drops for GAN, IMS, even though WiFi may be used
 *                   - Would NOT include VoIP call drops (WiFi only)
 * ???? Radio - WLAN (Quincy Codes were 4000-4999) TODO: To be confirmed
 * ???? Radio - MWAF (Quincy Codes were 6000-6999) TODO: To be confirmed
 * ???? Radio - CTRL (Quincy Codes were 7000-7999) TODO: To be confirmed
 *
 * Note: Comments in this file follow a specific format necessary to
 *       properly document each Event Type.
 *
 * Comment Format:
 *
 * Please READ this First!
 *   http://wikis.rim.net/display/bifdap/Forensics+Logworthy+API+-+Getting+Started
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
 *    Critical      - Process crash or other component failure (includes radio/modem reset)
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

#define LW_EVENT_BBX_SYSTEM_RANGE_BEGIN                   0x00000500

// Events for RIM-specific drivers, services, etc. are defined here.

#define LW_EVENT_RESET_UNKNOWN                            0x00000500
    ///< Hardware reset occurred but reset detection code was unable to determine the cause.
    ///< Severity: Unknown
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_GRAPHICS_GPU_FAILURE                     0x00000501
    ///< GPU failure has been detected. Graphics non-functional.
    ///< Severity: Critical
    ///< D1: GPU Device Type: Enumeration of GPU Device Types (see hardware-specific source code for details).
    ///< D2: GPU Error Cause: Enumeration of GPU Error Causes (see hardware-specific source code for details).
    ///< D3: Failure Counter: Count of the number of GPU failures encountered since the device was loaded.

#define LW_EVENT_RESET_WATCHDOG                           0x00000502
    ///< Apps core watchdog reset the device, likely because the device software is unresponsive.
    ///< Severity: Catastrophic
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_USER_FORCED                        0x00000503
    ///< User forced a device reset by pressing the two side volume keys (or "+" volume
    ///< and power key for winchester), likely because the device was unresponsive
    ///< (examples: UI frozen, device not coming out of sleep, etc.).
    ///< Severity: Catastrophic
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_USER_REQUESTED                     0x00000504
    ///< User requested a device reset device via the normal "Restart"/"Turn Off" dialog box UI.
    ///< Severity: Unknown
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_SW_REQUESTED                       0x00000505
    ///< A critical software component found an irrecoverable error which can only be fixed via a device reset.
    ///< Note this indicates a shutdown triggered through the Core Services PPS API.
    ///< Severity: Unknown
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_CFP                                0x00000506
    ///< Reset triggered by "cfp" software loading tool (via usbmux).  Not an error.
    ///< Severity: Debug
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_OTA_SW_UPGRADE                     0x00000507
    ///< User requested over-the-air software upgrade, which reset the device during the
    ///< installation procedure.  Not an error.
    ///< Severity: Debug
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_POWERLOSS                          0x00000508
    ///< Device powered up from a state of having no power at all.  IE. battery removed/inserted
    ///< OR battery was dead and charger just plugged in OR internal 'kill switch' triggered.
    ///< Severity: Unknown
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_PMIC_WD                            0x00000509
    ///< Device reset because device failed to kick the PMIC watchdog.
    ///< Severity: Catastrophic
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_HW_FAULT                           0x0000050a
    ///< Device reset because of a hardware fault indicated by the ASIC or PMIC.
    ///< Severity: Catastrophic
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_SMMU_FAULT                               0x0000050b
    ///< SMMU fault has been detected. Potentially fatal condition depending on faulting device.
    ///< Severity: Serious
    ///< D1: HW-specific code: (see hardware-specific source code for details).
    ///< D2: HW-specific code: (see hardware-specific source code for details).
    ///< D3: HW-specific code: (see hardware-specific source code for details).
    ///< D4: HW-specific code: (see hardware-specific source code for details).

#define LW_EVENT_DISPLAY_FRAME_TRANSFER_TIMEOUT           0x0000050c
    ///< Display data transfer timeout has been detected. This will cause visible corruption
    ///< on the display.
    ///< Severity: Serious
    ///< D1: HW-specific code: (see hardware-specific source code for details).
    ///< D2: HW-specific code: (see hardware-specific source code for details).
    ///< D3: HW-specific code: (see hardware-specific source code for details).
    ///< D4: HW-specific code: (see hardware-specific source code for details).

#define LW_EVENT_DISPLAY_DATA_UNDERFLOW                   0x0000050d
    ///< Data underflow to the display panel is detected. This might cause visible
    ///< corruption on the display.
    ///< Severity: Serious
    ///< D1: HW-specific code: (see hardware-specific source code for details).
    ///< D2: HW-specific code: (see hardware-specific source code for details).
    ///< D3: HW-specific code: (see hardware-specific source code for details).
    ///< D4: HW-specific code: (see hardware-specific source code for details).

#define LW_EVENT_BATTERY_RAPID_DRAIN                      0x0000050e
    ///< Rapid battery drain detected while device "not in use" (screen off).
    ///< The active rapid battery drain is LW_EVENT_BATTERY_ACTIVE_RAPID_DRAIN
    ///< Severity: Serious

#define LW_EVENT_THERMAL_HOT                              0x0000050f
    ///< Thermal framework sensor has exceeded its maximum temperature threshold.
    ///< Severity: Serious
    ///< D1: Sensor index: Temperature sensor index that is overheating.

#define LW_EVENT_RESET_THERMAL_HW_SHORT                   0x00000510
    ///< Thermal framework detected a hardware fault on the board (likely a short
    ///< circuit) and shut down the device.
    ///< Severity: Catastrophic
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_I2C_ARBITRATION_LOSS                     0x00000511
    ///< I2C error due to arbitration loss.  I2C clock line may be pulled low.
    ///< Severity: Serious
    ///< D1: Error Status Flags: Software driver error status flags
    ///< D2: Actual Error Code: Software driver error code
    ///< D3: Slave Address: Uniquely identifies the I2C client
    ///< D4: Bus Number: Uniquely identifies the I2C bus used

#define LW_EVENT_I2C_BUSY                                 0x00000512
    ///< Timeout error occurred while slave was waiting for I2C bus to become inactive.
    ///< Severity: Serious
    ///< D1: Error Status Flags: Software driver error status flags
    ///< D2: Actual Error Code: Software driver error code
    ///< D3: Slave Address: Uniquely identifies the I2C client
    ///< D4: Bus Number: Uniquely identifies the I2C bus used

#define LW_EVENT_I2C_NACK                                 0x00000513
    ///< I2C NACK was not received in response to the the initial slave address byte.
    ///< Severity: Serious
    ///< D1: Error Status Flags: Software driver error status flags
    ///< D2: Actual Error Code: Software driver error code
    ///< D3: Slave Address: Uniquely identifies the I2C client
    ///< D4: Bus Number: Uniquely identifies the I2C bus used

#define LW_EVENT_I2C_BUS_ERROR                            0x00000514
    ///< I2C error due to a bus error that is not one of arbitration loss,
    ///< busy error, or NACK.
    ///< Severity: Serious
    ///< D1: Error Status Flags: Software driver error status flags
    ///< D2: Actual Error Code: Software driver error code
    ///< D3: Slave Address: Uniquely identifies the I2C client
    ///< D4: Bus Number: Uniquely identifies the I2C bus used

#define LW_EVENT_BLUETOOTH_DEPRECATED_01                  0x00000515
    ///< Deprecated Bluetooth software error.
    ///< When the internal stack lock cannot be acquired within some time,
    ///< the operation fails and might cause internal problems of which we'd
    ///< like to know. Other than the details as specified below, the stack's
    ///< internal debug stats and the thread map for io-bluetooth will be
    ///< dumped (as a BLOB) to filename bt_stack_stats.txt.
    ///< NOTE: blob will be deprecated soon and we will need to create a
    ///< separate log log type for bt_stack_stats.txt.
    ///< Severity: Serious
    ///< D1: Thread ID Current Attempt: Thread ID trying to acquire the lock
    ///< D2: Thread ID Last Success: Thread ID of the last successful lock

#define LW_EVENT_RESET_DETECTION_ERROR                    0x00000516
    ///< A hardware reset occurred but the reset cause detection code encountered
    ///< an internal error and needs to be updated/fixed.
    ///< Severity: Catastrophic

#define LW_EVENT_CAMERA_CAPTURE_FAILURE                   0x00000517
    ///< Camera capture failed; user may be able to re-try taking a picture
    ///< Severity: Serious
    ///< D1: Camera Unit: front/back/usb
    ///< D2: Error Code: Error returned from camera controller
    ///< D3: Operation: Start/Stop/Process
    ///< D4: Capture Mode: Picture/Video/Timewarp/Retroactive

#define LW_EVENT_CAMERA_VIEWFINDER_FAILURE                0x00000518
    ///< Attempt to open or close camera view finder failed.
    ///< Severity: Critical
    ///< D1: Camera Unit: front/back/usb
    ///< D2: Error Code: Error returned from camera controller
    ///< D3: Operation: Start/Stop
    ///< D4: Shooting Mode: video/picture

#define LW_EVENT_CAMERA_OPEN_FAILURE                      0x00000519
    ///< Attempt to open camera failed
    ///< Severity: Critical
    ///< D1: Camera Unit: front/back/usb
    ///< D2: Error Code: Error returned from camera controller

#define LW_EVENT_CAMERA_SAVE_FAILURE                      0x0000051a
    ///< Attempt to save image capture or video failed; user may be able to re-try
    ///< the operation.
    ///< Severity: Serious
    ///< D1: Camera Unit: front/back/usb
    ///< D2: Error Code: Error returned from camera controller
    ///< D3: Camera Mode: Picture/Video/Timewarp

#define LW_EVENT_CAMERA_SETTING_FAILURE                   0x0000051b
    ///< Attempt to change camera settings failed.
    ///< Severity: Warning
    ///< D1: Camera Mode: Video/Picture
    ///< D2: Camera Shooting Mode: Burst/IS/Timewarp
    ///< D3: Scene Mode: Action/Whiteboard/Night
    ///< D4: Ratio/Size: Specifies ratio if Details 1 is Picture; specifies
    ///<     Size if Details 1 is Video.

#define LW_EVENT_CAMERA_LIGHT_FAILURE                     0x0000051c
    ///< Attempt to turn camera/video light on or off failed.
    ///< Severity: Warning
    ///< D1: Camera Unit: front/back/usb
    ///< D2: Error Code: Error returned from camera controller
    ///< D3: Camera Mode: Picture/Video
    ///< D4: Operation: On/Off

#define LW_EVENT_AUDIO_PCM_LOGGING_USER_REQUESTED         0x0000051d
    ///< Initiated io-audio PCM logging snapshot
    ///< Severity: Debug

#define LW_EVENT_RESET_DEVELOPER                          0x0000051e
    ///< The shutdown utililty or the shutdown script was called directly by the developer through telnet/serial/ssh
    ///< Severity: Debug
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_NFC_UNEXPECTED_HW_CONFIG                 0x0000051f
    ///< NFC running on HW that has not been brought up by the NFC driver team
    ///< This means NFC driver's fallback auto-detection worked, and is a signal
    ///< to NFC driver folks to add official support
    ///< Severity: Debug

#define LW_EVENT_NFC_FIRMWARE_UPDATE_FAILURE              0x00000520
    ///< Errors occurred updating NFC controller's firmware.  This is typically
    ///< a timeout during firmware update.
    ///< Severity: Warning

#define LW_EVENT_NFC_DEPRECATED_HW                        0x00000521
    ///< We're running on HW that's no longer supported
    ///< Ex:  NFC RF tuning that is no longer supported, old NFC controller,
    ///< etc....
    ///< Severity: Warning

#define LW_EVENT_NFC_DRIVER_FAILURE                       0x00000522
    ///< Critical failure in NFCA Layer.  We cannot continue without a process
    ///< restart.
    ///< Severity: Critical
    ///< D1: Line: Line number of failure
    ///< D2: File: Hash of file containing failure
    ///< D3: NFCA Function: Hash of NFCA function where failure occurs

#define LW_EVENT_NFC_DRIVER_UNEXPECTED_STATE              0x00000523
    ///< Something unexpected happened in NFCA, but we can continue.  Notice
    ///< to NFC Driver team to follow up
    ///< Severity: Warning
    ///< D1: Line: Line number of failure
    ///< D2: File: Hash of file containing failure
    ///< D3: NFCA Function: Hash of NFCA function where unexpected error occurs

#define LW_EVENT_NFC_VENDOR_API_ERROR_CODE_MAJOR           0x00000524
    ///< Critical failure when calling into vendor code.  Vendor API
    ///< call returned an error code that we can not recover from.
    ///< Severity: Critical
    ///< D1: Line: Line number of failure
    ///< D2: File: Hash of file containing failure
    ///< D3: Vendor API: Hash of vendor API name

#define LW_EVENT_NFC_VENDOR_API_ERROR_CODE_MINOR           0x00000525
    ///< Unexpected failure when calling into vendor code.  Vendor API
    ///< call returned an error code that we were not expecting, but we can
    ///< continue.
    ///< Severity: Warning
    ///< D1: Line: Line number of failure
    ///< D2: Line: Hash of file containing failure
    ///< D3: Vendor API: Hash of vendor API name

#define LW_EVENT_NFC_VENDOR_ASSERTION                     0x00000526
    ///< Assertion in vendor code.  Vendor code fails an assert() statement
    ///< We can not continue.
    ///< Severity: Critical
    ///< D1: Line: Line number of failure
    ///< D2: File: Hash of file containing failure
    ///< D3: Vendor Function: Hash of vendor function where failure occurred.

#define LW_EVENT_NFC_VENDOR_PORTING_FAILURE               0x00000527
    ///< Critical failure in support code
    ///< call returned an error code that we can not recover from.
    ///< Severity: Critical
    ///< D1: Line: Line number of failure
    ///< D2: File: Hash of file containing failure
    ///< D3: Function: Hash of function containing failure

#define LW_EVENT_NFC_VENDOR_PORTING_UNEXPECTED_STATE      0x00000528
    ///< Unexpected failure when calling into vendor code.  Vendor API
    ///< call returned an error code that we were not expecting, but we can
    ///< continue.
    ///< Severity: Warning
    ///< D1: Line: Line number of failure
    ///< D2: File: Hash of file containing failure
    ///< D3: Function: Hash of function containing failure

#define LW_EVENT_NFC_CONTROLLER_COMMUNICATION_ERROR       0x00000529
    ///< Unexpected issue when talking to NFC controller - unexpected i2c retry
    ///< bad CRC/LRC code on frame read over i2c, etc...
    ///< We can continue.
    ///< Severity: Warning
    ///< D1: I2C Failure: Sub-type of i2c failure (i2c retry, bad LRC, ...)


#define LW_EVENT_DISPLAY_DSI_ERROR                        0x00000530
    ///< DISPLAY DSI This will cause the driver to reset the DSI host and
    ///< recover. It will cause visible corruption on the display.
    ///< Severity: Serious
    ///< D1: HW-specific code: (see hardware-specific source code for details).
    ///< D2: HW-specific code: (see hardware-specific source code for details).
    ///< D3: HW-specific code: (see hardware-specific source code for details).
    ///< D4: HW-specific code: (see hardware-specific source code for details).

#define LW_EVENT_DISPLAY_READ_WRITE_FAILURE               0x00000531
    ///< Display read/write over the DSI failed.
    ///< This may put the display under unknown state and cause visible corruption
    ///< on the display.
    ///< Severity: Serious
    ///< D1: Display ID: Identifies which display (0-Primary)
    ///< D2: HW-specific code: (see hardware-specific source code for details).
    ///< D3: HW-specific code: (see hardware-specific source code for details).
    ///< D4: HW-specific code: (see hardware-specific source code for details).

#define LW_EVENT_ROTATOR_ERROR                            0x00000532
    ///< Display rotator block reported error.
    ///< This will cause the rotator to fail. The image will not displayed.
    ///< Severity: Serious
    ///< D1: HW-specific code: (see hardware-specific source code for details).
    ///< D2: HW-specific code: (see hardware-specific source code for details).
    ///< D3: HW-specific code: (see hardware-specific source code for details).
    ///< D4: HW-specific code: (see hardware-specific source code for details).

#define LW_EVENT_ROTATOR_TIMEOUT                          0x00000533
    ///< Display rotator block timeout detected. This will cause visible corruption on the
    ///< display
    ///< Severity: Serious
    ///< D1: HW-specific code: (see hardware-specific source code for details).
    ///< D2: HW-specific code: (see hardware-specific source code for details).
    ///< D3: HW-specific code: (see hardware-specific source code for details).
    ///< D4: HW-specific code: (see hardware-specific source code for details).

#define LW_EVENT_DISPLAY_AXI_READ_WRITE_FAILURE           0x00000534
    ///< Display mixer reported AXI read/write failue. This will cause visible display
    ///< corruption.
    ///< Severity: Serious
    ///< D1: HW-specific code: (see hardware-specific source code for details).
    ///< D2: HW-specific code: (see hardware-specific source code for details).
    ///< D3: HW-specific code: (see hardware-specific source code for details).
    ///< D4: HW-specific code: (see hardware-specific source code for details).

#define LW_EVENT_DISPLAY_ERROR_UNKNOWN                    0x00000535
    ///< Unknown error detected on display. This will need to be looked at to
    ///< identify the exact failure.
    ///< Severity: Unknown
    ///< D1: HW-specific code: (see hardware-specific source code for details).
    ///< D2: HW-specific code: (see hardware-specific source code for details).
    ///< D3: HW-specific code: (see hardware-specific source code for details).
    ///< D4: HW-specific code: (see hardware-specific source code for details).

#define LW_EVENT_GRAPHICS_GPU_RECOVERY                    0x00000536
    ///< GPU failure was detected and recovery was successful.
    ///< Severity: Serious
    ///< D1: GPU Device Type: Enumeration of GPU Device Types (see hardware-specific source code for details).
    ///< D2: GPU Error Cause: Enumeration of GPU Error Causes (see hardware-specific source code for details).
    ///< D3: Failure Counter: Count of the number of GPU failures encountered since the device was loaded.

#define LW_EVENT_USB_ERROR                                0x00000537
    ///< USB connection failure.  User may need to unplug/replug USB cable or
    ///< reboot the device to restore USB connectivity.
    ///< Severity: Serious
    ///< D1: Error Source: Where is the error being reported from.
    ///< D2: Error Flags: What error was encountered
    ///< D3: Data 1: Context specific data
    ///< D4: Data 2: Context specific data

#define LW_EVENT_PROCESS_TERMINATION_REQUEST_FAILED       0x00000538
    ///< Navigator requested that a process or application terminate in order
    ///< to free up memory.  The process/app did not fulfill the request.
    ///< Severity: Serious

#define LW_EVENT_RESET_BOOTROM_UPGRADE                    0x00000539
    ///< A reset occured in order to upgrade the bootrom.
    ///< Severity: Debug
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_NP_PPS_FAILURE                           0x0000053a
    ///< The NowPlaying service exited due to a PPS failure.  Media playback may be temporarily interrupted.
    ///< Severity: Serious
    ///< D1: NP code: NP error code indicating the particular PPS failure
    ///< D2: PPS fd: file descriptor being accessed
    ///< D3: errno: stdlib err number

#define LW_EVENT_RESET_CRITICAL_PROCESS_CRASH             0x0000053b
    ///< A critical process crashed and the sanity-monitor reset the device.
    ///< Severity: Catastrophic
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_BATTERY_DRAINED                    0x0000053c
    ///< Software detected the battery has drained below the minimum threshold and shutdown the device.
    ///< Severity: Unknown
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_LGMON_UI_LAG                             0x0000053d
    ///< A UI lag greater than defined lag threshold is detected by LGMON.
    ///< Severity: Serious

#define LW_EVENT_LGMON_FRAMES_DROPPED                     0x0000053e
    ///< When LGMON detects consecutive frame drops greater than preset number of slots.
    ///< Severity: Serious

#define LW_EVENT_SENSOR_I2C_RETRY                         0x0000053f
    ///< Sensor had to retry communicating on the I2C bus due to a failure.
    ///< Severity: Warning
    ///< D1: Slave Address: I2C Bus address of the slave that experienced the error
    ///< D2: Register Address: Internal address slave that we were trying to access
    ///< D3: Transaction Type/Error Code: (transaction_type << 8) | (i2c_devctl_error_code)
    ///< D4: Count: Count of occurences of I2C errors during last throttling period that were dropped

#define LW_EVENT_SENSOR_I2C_RECOVERY_FAILURE              0x00000540
    ///< When an i2c error occurs that is not recovered after all retries and bus recovery.
    ///< After this occurs, you can no longer expect sensor data from most/all sensors and
    ///< other parts sharing same bus might be impossible to communicate with.
    ///< Severity: Serious

#define LW_EVENT_PHONESERVICE_STALE_CALL                  0x00000541
    ///< The phone is in an idle state but there are stale PPS call objects found.
    ///< This could cause the Phone UI and other clients to be out of sync and
    ///< potentially unusable.
    ///< Severity: Serious

#define LW_EVENT_NFC_UNKNOWN_CONTROLLER                   0x00000542
    ///< Firmware could not be updated because the NFC controller version is unrecognized
    ///< Severity: Warning

#define LW_EVENT_RESET_BATTERY_SLUMP                      0x00000543
    ///< Software detected the battery slump and shutdown the device.
    ///< Severity: Unknown
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_SLOG2_REGISTRATION_FAILURE               0x00000544
    ///< Unable to register with the slog2 API. Logging is likely not functional.
    ///< Severity: Warning

#define LW_EVENT_NET_DEPRECATED_01                        0x00000545
    ///< Deprecated event id.
    ///< Severity: Unknown

#define LW_EVENT_RESET_BATTERY_REMOVED_WITH_USB           0x00000546
    ///< Battery was removed while the USB was connected.
    ///< Severity: Debug
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_BATTERY_TOO_LOW_TO_BOOT            0x00000547
    ///< Battery too low to boot past IFS
    ///< Severity: Debug
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_POWER_SOURCE_UNPLUGGED             0x00000548
    ///< Charge_monitor detected power source unplugged while it was in IFS.
    ///< Severity: Debug
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_NO_BATTERY                         0x00000549
    ///< Battery not inserted into the device
    ///< Severity: Debug
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_WLAN_CALIBRATION                   0x00000550
    ///< Reset by WLAN Calibration script
    ///< Severity: Debug
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_BATTERY_TEMP_OUT_OF_RANGE          0x00000551
    ///< Battery temperature is out of range
    ///< Severity: Catastrophic
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_WATCHDOG_HOG                       0x00000552
    ///< Apps core watchdog task or bark isr reset the device because some process(es) hog the system.
    ///< Severity: Catastrophic
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_NFC_CONTROLLER_EXCEPTION                 0x00000553
    ///< NFC indicated an unexpected controller exception
    ///< Exception codes are vendor specific.  Inside Secure (Open NFC/SecuRead)
    ///< is currently our only vendor
    ///< Severity: Critical
    ///< D1:  Exception Code:  For Inside soln, bit 31 indicates whether NAL (1) or Open NFC (0) captured the exception

#define LW_EVENT_NFC_CONTROLLER_CRASH                     0x00000554
    ///< NFC stack indicated an unexpected controller reset or crash
    ///< Severity: Critical

#define LW_EVENT_RESET_INITIAL_SANITY_FAILED              0x00000555
    ///< Sanity-monitor reset device because it was insane at boot.
    ///< Severity: Catastrophic
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_BATTERY_ERROR                      0x00000556
    ///< ChargeMonitor shutdown because of battery system error reason.
    ///< Severity: Catastrophic
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_CHARGEMONITOR_UNKNOWN_ERROR        0x00000557
    ///< ChargeMonitor shutdown because of unknown reason
    ///< Severity: Catastrophic
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_DESKTOP                            0x00000558
    ///< Desktop initiated a reset
    ///< Severity: Debug
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_BATTERY_ACTIVE_RAPID_DRAIN               0x00000559
    ///< Rapid battery drain detected while device "in use" (screen on).
    ///< The in-active rapid drain case is LW_EVENT_BATTERY_RAPID_DRAIN
    ///< Severity: Serious

#define LW_EVENT_BATTERY_FUEL_GAUGE_DISCREPANCY           0x0000055a
    ///< Fuel Gauge discrepancy between voltage and percent capacity
    ///< Severity: Warning

#define LW_EVENT_RESET_SECURITY_WIPE                      0x0000055b
    ///< factorywipe.sh executed to do Security Wipe
    ///< Severity: Debug
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_RESET_BATTERY_INSERTED_WITH_USB          0x0000055c
    ///< Battery was inserted while USB was connected
    ///< Severity: Debug
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_AUDIO_CODEC_FLL_FAILURE                  0x0000055d
    ///< The FLL on the external audio codec failed to lock.  The user may
    ///< experience a delay in the start of audio or degraded audio.
    ///< Severity: Serious
    ///< D1: Count: Number of unsuccessful FLL lock failures.

#define LW_EVENT_AUDIO_FRAME_SYNC_FAILURE                 0x0000055e
    ///< MCBSP frame sync error detected.  The user may experience a delay in
    ///< the start of audio or degraded audio.
    ///< Severity: Serious
    ///< D1: Count: Number of sync attempts before failing.

#define LW_EVENT_AUDIO_LATENCY_DETECTED                   0x0000055f
    ///< Detected that the audio system is running at an incorrect speed.
    ///< Audio is running more than 2 times faster or slower. The user will
    ///< experience degraded audio.
    ///< Severity: Serious
    ///< D1: Latency Detected: Too slow (0) or too fast (1).

#define LW_EVENT_AUDIO_SERIAL_FAILURE                     0x00000560
    ///< The number of serial errors exceeded threshold.  The user will
    ///< experience degraded audio.
    ///< Severity: Serious
    ///< D1: Count: Number of errors detected.

#define LW_EVENT_AUDIO_DPLL_FAILURE                       0x00000561
    ///< The DPLL clock on the host processor fails to lock.  The user will
    ///< experience degraded audio.
    ///< Severity: Serious
    ///< D1: DPLL state: Whether the DPLL failed on (1) or off (0).

#define LW_EVENT_DIAG_NETWORK_TRAFFIC_SAMPLE              0x00000562
    ///< If a user opts in, network traffic and associated logs are captured periodically.
    ///< Severity: Debug
    ///< D1:  Sample Period:  Number of hours between samples

#define LW_EVENT_SENSOR_HARDWARE_TIMEOUT                  0x00000563
    ///< Sensor hardware data streaming timed out for some unexpected reason.
    ///< System automatically attempts to recover, but no guarantee of success.
    ///< Severity: Serious
    ///< D1: Hardware Type: Sensor type that timed out

#define LW_EVENT_SECURITY_INTEGRITY_CHECK_FAIL            0x00000564
    ///< Security integrity check indicates tampering
    ///< Severity: Serious
    ///< D1: Failure Reason: Code indicating the specific security check that failed

#define LW_EVENT_RESET_PMIC_TFT_WATCHDOG                  0x00000565
    ///< Device reset because device failed to kick the PMIC TFT (Thermal Fault Tolerance) watchdog.
    ///< Severity: Catastrophic
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_PHONESERVICE_EMERGENCY_CALL              0x00000566
    ///< Emergency call was made.  Emergency calls are expected to be very rare so
    ///< when one occurs, there is a high probability that it may be spurious.
    ///< Severity: Debug

#define LW_EVENT_BATTERY_VOLTAGE_HIGH_SLUMP               0x00000567
    ///< Battery voltage slump unexpectedly occurred at high voltage level (>=3.9V).   Should never happen.  Could be a problem with battery contacts or other hardware issue.
    ///< Severity: Critical
    ///< D1: Voltage Threshold (mV):  Voltage was greater than or equal to this value when the slump occurred

#define LW_EVENT_BATTERY_VOLTAGE_LOW_SLUMP                0x00000568
    ///< Battery was very low, resulting in a voltage slump.  User is likely running device at low power close to threshold where the device may need to be shut down (voltage < 3.9V).
    ///< Severity: Debug
    ///< D1: Voltage Threshold (mV):  Voltage was less than this value when the slump occurred

#define LW_EVENT_VIDEO_PROCESSOR_FAILURE                  0x00000569
    ///< Video processor failure has been detected and may be auto-restarted.
    ///< Severity: Critical
    ///< D1: Video Processor Type: 0-Unknown, 1-QCOM Venus

#define LW_EVENT_GEOLOC_LOCP_ERROR                        0x0000056a
    ///< Location provider reported an error to location manager.  May impact impact ability to obtain location.
    ///< Severity: Warning
    ///< D1: Location Provider: Location provider class mask

#define LW_EVENT_GEOLOC_LOCP_BAD_DATA                     0x0000056b
    ///< Location provider reported. May have minor impact on availability of location data.
    ///< Severity: Warning
    ///< D1: Location Provider: Location provider class mask

#define LW_EVENT_GEOLOC_LOCP_UNRESPONSIVE                 0x0000056c
    ///< Location provider process went away as detected by location manager. May impact impact ability to obtain location.
    ///< Severity: Warning
    ///< D1: Location Provider: Location provider class mask

#define LW_EVENT_GEOLOC_AIDING_ERROR                      0x0000056d
    ///< Aiding framework encountered an error, GNSS will not be aided. May result in slower time to first location fix.
    ///< Severity: Warning

#define LW_EVENT_GEOLOC_GNSS_COMM_ERROR                   0x0000056e
    ///< GNSS failed to communicate with hardware.  Location services may be impacted.
    ///< Severity: Serious

#define LW_EVENT_GEOLOC_LM_BAD_CLIENT_DATA                0x0000056f
    ///< Location manager received malformed data in an API request from a client.  Application will possibly not receive location response.
    ///< Severity: Warning

#define LW_EVENT_AUDIO_CTRL_INIT_FAILURE                  0x00000570
///< Failure in audio driver during initialization.  No audio will work.
///< Severity: Serious
///< D1: Init Error Code: Maps to specific location where audio init error occurred
///< D2: Lib Error Code: Error code returned by failed function or library API call.

#define LW_EVENT_AUDIO_DSP_ERROR                          0x00000571
///< Audio DSP returned an error. Audio may not work properly until device is reset.
///< Severity: Serious
///< D1: CSD IOCTL Command: The CSD IOCTL command that failed
///< D2: CSD IOCTL Error: Error number defined in csd_status.h

#define LW_EVENT_SENSOR_UNEXPECTED_STATE                  0x00000572
    ///< A sensor may appear unavailable to client applications, or sensor data stream
    //< may be stalled or corrupted.  Sensors include: accelerometer, magnetometer,
    //< gyroscope, light sensor, face-detection sensor, pressure sensor or holster sensor.
    //< Caused by an unexpected programmatic condition encountered in the sensor subsystem
    //< (for example, null-pointer check failed) which may have manifested itself as an API error.
    ///< Severity: Serious
    ///< D1: Module Hash: Hash of file name where assert occurred
    ///< D2: Module Line Number: Line number in source file where assert occurred
    ///< D3: Optional: Assert specific
    ///< D4: Optional: Assert specific

#define LW_EVENT_GRAPHICS_GPU_SMMU_FAULT                  0x00000573
    ///< GPU SMMU fault was detected.
    ///< Severity: Warning
    ///< D1: GPU Device Type: Enumeration of GPU Device Types (see hardware-specific source code for details).
    ///< D2: GPU Error Cause: Enumeration of GPU Error Causes (see hardware-specific source code for details).

#define LW_EVENT_RESET_SANITY_CHECKING                    0x00000574
    ///< SanityChecking marked system as insane and reset device.
    ///< Severity: Catastrophic
    ///< D1: PMIC Reset Reason: HW-specific reset code provided by the PMIC
    ///< D2: ASIC Reset Reason: HW-specific reset code provided by the ASIC

#define LW_EVENT_VPN_SUBCOMPONENT_TERMINATION_FAILURE     0x00000575
    ///< VPN manager could not terminate a subcomponent process.  VPN connectivity may be affected until device reset.
    ///< Severity: Serious
    ///< D1: Subprocess Type: Identifier of process which could not be terminated.

#define LW_EVENT_MAPS_APP_UI_ALERT_NULL                   0x00000576
    ///< An intended dialog may not have been shown to the user because a notification to display a NULL dialog was received.
    ///< Severity: Warning

#define LW_EVENT_MAPS_APP_LAUNCH_FAILED                   0x00000577
    ///< Unable to start the Maps app due to an initialization error.
    ///< Severity: Serious
    ///< D1: Init Failure Type: 1 - Controller Manager create failed; 2 -Show UI failed

#define LW_EVENT_MAPS_APP_UNITS_CHANGE_FAILED             0x00000578
    ///< User attempt to change units of measure but the operation failed (eg. kilometers to miles).
    ///< Severity: Warning

#define LW_EVENT_MAPS_APP_LANGUAGE_FILES_DOWNLOAD_FAILED  0x00000579
    ///< Audio for turn by turn navigation in Maps app may not work due to failure to download the language file set.
    ///< Severity: Serious
    ///< D1: File Set Error Type: 1 - fileset status get; 2 - fileset get

#define LW_EVENT_MAPS_APP_REGIONAL_DISTANCE_FORMAT_ERROR  0x0000057A
    ///< Unrecognized regional distance format. It is neither mph nor kph.  User impact - distances may not be shown correctly.
    ///< Severity: Serious

#define LW_EVENT_MAPS_APP_3D_NAVIGATION_VIEW_INIT_FAILED  0x0000057B
    ///< Unable to display the 3D navigation view for the user due to EGL initialization failure.
    ///< Severity: Serious
    ///< D1: Init Step: Indicates which EGL initialization step failed

#define LW_EVENT_MAPS_APP_UI_QML_FAILURE                  0x0000057C
    ///< Some portion of the UI failed to render due to a Cascades QML Document error.
    ///< Severity: Serious
    ///< D1: QML Error Type: 1 - QmlDocument create failure; 2 - Property read failure; 3 - Init failure

#define LW_EVENT_MAPS_APP_LOCATION_SERVICES_SETTING_CORRUPT   0x0000057D
    ///< The user setting to enable/disable Location Services is corrupt and cannot be read.  Unable to turn the setting on or off.
    ///< Severity: Serious

#define LW_EVENT_MAPS_APP_MAP_RENDERING_FAILURE           0x0000057E
    ///< Unable to render a map due to an EGL error encountered while creating the map.
    ///< Severity: Serious
    ///< D1: RenderEngine Step: Indicate which step RenderEngine EGL initial failed.

#define LW_EVENT_MAPS_APP_UI_TASKQUEUEMANAGER_FAILURE     0x0000057F
    ///< Some portions of the UI fail to render, severely limiting functionality of the Maps app due to failure to create the UI task queue.
    ///< Severity: Serious

#define LW_EVENT_BBX_SYSTEM_RANGE_END                     0x000005ff

#define LW_EVENT_WLAN_RANGE_BEGIN                         0x00000900

#define LW_EVENT_WLAN_DRV_FW_RECOVERY                     0x00000900
    ///< Generated when the WLAN chipset code indicates a firmware recovery has occurred.
    ///< Severity: Critical
    ///< D1: Recovery Type: Type of firmware recovery.

#define LW_EVENT_WLAN_MNGR_EMC_FAIL                       0x00000965
    ///< Generated when wlan_mngr fails to determine if the device is in EMC mode and allows WLAN to power on.
    ///< Severity: Serious
    ///< D1: Return Code: Code returned by wlan_mngr.

#define LW_EVENT_WLAN_MNGR_MISSED_DISCONNECT              0x00000966
    ///< Wlan_mngr, while in connected state, detected a new connection event before seeing a disconnect event.
    ///< Severity: Warning

#define LW_EVENT_WLAN_MNGR_MEM_CORRUPT                    0x00000967
    ///< Memory corruption detected, which will likely lead to a process crash or other unexpected behaviour.  The Corruption Type indicates where corruption was detected.
    ///< Severity: Serious
    ///< D1: Corruption Type: Enumeration indicating where the corruption was detected (eg. which struct or variable).

#define LW_EVENT_WLAN_MNGR_CONNECTIVITY_LOST_PKT_MNT_TIMEOUT        0x00000968
    ///< Wifi connectivity was lost, wlan mngr has detected that no packets have been sent or received for an extended period of time. Wlan Mngr will attempt to resolve the issue by toggling wifi power.  If this does not resolve the issue the user may need to reset their device.
    ///< Severity: Warning
    ///< D1: P1 Count: number of P1 periods hit without an increase in traffic counts.
    ///< D2: P2 Count: number of P2 periods hit without an increase in traffic counts.
    ///< D3: P3 Count: number of P3 periods hit without an increase in traffic counts.
    ///< D4: Radio status: 0 - radio reset, 1 - radio turned off

#define LW_EVENT_WLAN_MNGR_CONSECUTIVE_SCAN_TIMEOUTS      0x00000969
    ///< Detected consecutive scan timeouts. The device could enter a state where the firmware never returns scan results. No WiFi networks will be found.
    ///< Severity: Serious

#define LW_EVENT_WLAN_RANGE_END                           0x000009ff

#define LW_EVENT_CELLULAR_RANGE_BEGIN                     0x00000a00

#define LW_EVENT_CELLULAR_MODEM_CRASH                     0x00000a00
    ///< Modem has crashed.
    ///< Severity: Catastrophic
    ///< D1: Reason Hash: Hash of reason string?

#define LW_EVENT_CELLULAR_CALL_DROP                       0x00000a01
    ///< Phone call setup failure or call drop.  Includes drops due to network congestion.
    ///< Severity: Serious
    ///< D1: Not Used: Not used
    ///< D2: Cause: 0-Unknown, 1-Network Congestion
    ///< D3: Error Hash: Hash of modem error string?

#define LW_EVENT_CELLULAR_CALL_RELEASE_FAILURE            0x00000a02
    ///< Call disconnected without proper release from network.  (Formerly Quincy LW1017.)
    ///< Severity: Warning
    ///< D1: Error Hash: Hash of modem error string?

#define LW_EVENT_CELLULAR_NO_SERVICE                      0x00000a03
    ///< No cellular network service with no PLMN search.
    ///< Severity: Serious
    ///< D1: Error Hash: Hash of modem error string?

#define LW_EVENT_CELLULAR_DEPRECATED_01                   0x00000a04
    ///< Deprecated event id, to be reused after discussion.
    ///< Severity: Unknown

#define LW_EVENT_CELLULAR_SIM_CARD_ERROR                  0x00000a05
    ///< UICC (SIM) Card error.  Card has read error and recovery actions may be attempted.
    ///< Severity: Warning
    ///< D1: UICC State: I.E. The PPS UICC State
    ///< D2: RRM UICC State: Custom per RRM
    ///< D3: Modem UICC State: Custom per RRM/modem
    ///< D4: Modem UICC Error Code: Custom per RRM/modem

#define LW_EVENT_CELLULAR_SIM_CARD_UNUSABLE               0x00000a06
    ///< UICC (SIM) Card fatal error.  Card is not usable/recognized.
    ///< Severity: Serious
    ///< D1: UICC State: I.E. The PPS UICC State
    ///< D2: RRM UICC State: Custom per RRM
    ///< D3: Modem UICC State: Custom per RRM/modem
    ///< D4: Modem UICC Error Code: Custom per RRM/modem

#define LW_EVENT_CELLULAR_AUDIO_QUALITY_USER_REQUESTED    0x00000a07
    ///< User reports poor audio quality during cellular voice call
    ///< Severity: Serious

#define LW_EVENT_CELLULAR_DATA_QCT_DSS_NET_IFACE_FAIL_OPEN_SESSION       0x00000a08
    ///< Configure QCT interface failed when opening data session
    ///< Severity: Serious
    ///< D1: QCT DSS Error: DSS error value

#define LW_EVENT_CELLULAR_DATA_QCT_DSS_NET_LIB_FAIL                      0x00000a09
    ///< Network library failure when opening data session
    ///< Severity: Serious
    ///< D1: QCT DSS Error: DSS error value

#define LW_EVENT_CELLULAR_DATA_QCT_DSS_NET_IFACE_FAIL_CLOSE_SESSION      0x00000a0a
    ///< Configure QCT interface failed when closing data session
    ///< Severity: Warning
    ///< D1: QCT DSS Error: DSS error value

#define LW_EVENT_CELLULAR_NETWORK_REJECT                  0x00000a0b
    ///< This event is triggered by a failure of a NAS signaling procedure.
    ///< The logworthy trigger is modem platform specific based on the underlying
    ///< technology and its limitations/features.
    ///< Severity: Unknown
    ///< D1: Network Reject Cause: non-zero if available
    ///< D2: Service Domain: 0=unknown, 1=CS, 2=PS, 3=CS+PS

#define LW_EVENT_CELLULAR_THERMAL_MITIGATION              0x00000a0c
    ///< Device temperature is high and user will now experience degraded connectivity
    ///< and/or SOS.
    ///< Severity: Warning
    ///< D1: Mitigation State: Thermal mitigation state modem is entering

#define LW_EVENT_CELLULAR_CALL_SETUP_FAILURE              0x00000a0d
    ///< Phone call setup failure or call drop.  Includes drops due to network congestion.
    ///< Severity: Serious
    ///< D1: Unused: Deprecated
    ///< D2: Cause: 0-Unknown, 1-Network Congestion
    ///< D3: Error Hash: Hash of modem error string?

#define LW_EVENT_CELLULAR_DSP_RESET                       0x00000a0e
    ///< The DSP processor has been restarted. This is not always a problem, but sometimes.
    ///< Severity: Unknown
    ///< D1: sub cause : Modem or technology specific sub-type code
    ///< D2: connection state: active CS or PS connection

#define LW_EVENT_CELLULAR_IMS_REGISTRATION_FAILURE        0x00000a0f
    ///< The logworthy would be raised if there is an issue related to IMS registration.
    ///< Severity: Warning
    ///< D1: Failure Type: 0-Unknown, 1-Attach Reject, 2-Detach before registration successful, 3-IMS registration failure

#define LW_EVENT_CELLULAR_DATA_STALL                      0x00000a10
    ///< Detection of downlink cellular data stalls for > 5 seconds.
    ///< Cellular data connections stop working while shown to be connected.
    ///< Severity: Serious
    ///< D1: Cellular Interface: Unit number of cellular network interface that detected the stall

#define LW_EVENT_CELLULAR_RADIO_OFF_FAILURE               0x00000a11
    ///< Radioctrl was blocked turning the cell radio OFF at an unexpected point
    ///< after the radio OFF process was allowed to proceed initially.
    ///< The radioctrl will be in a powering_off state which renders data services
    ///< OFF and in a net_disconnecting purgatory state.
    ///< Severity: Serious
    ///< D1: Who/why blocked power off: 0 - unknown, 1 powermon, 2 - RRM call failed
    ///< D2: Power state change cause: radioctrl_state.power_state_change_cause value

#define LW_EVENT_CELLULAR_MODEM_INIT_FAILURE              0x00000a12
    ///< Modem failed to initialize or boot.
    ///< No cellular service is available.
    ///< Severity: Catastrophic
    ///< D1: Cause:
    ///<     Cause values for STE modem:
    ///<     0-Unknown/unspecified
    ///<     1-UART serial port failure
    ///<     2- power up modem device failure
    ///<     3- HSI port failure
    ///<     4- load firmware failure
    ///<     5- EMRDY not received for 2 minutes
    ///<     6- Modem RAM test failure

#define LW_EVENT_CELLULAR_IMS_NETWORK_ATTACH_FAILURE      0x00000a13
    ///< Failed to attach to the IMS-PDN, which would make the device unable to proceed with IMS registration 
    ///< and access IMS services over cellular.
    ///< Severity: Warning
    ///< D1: Bearer Error code: RRM_RET_CAUSE as received in /pps/services/cds/bm/# (Bearer ID) PPS object.

#define LW_EVENT_CELLULAR_IMS_NETWORK_INITIATED_FAILURE   0x00000a14
    ///< Network initiated termination of the IMS connection, which would result in disruption of current 
    ///< IMS services over cellular. 
    ///< Severity: Warning
    ///< D1: Bearer Error code: RRM_RET_CAUSE as received in /pps/services/cds/bm/# (Bearer ID) PPS object. 

// Note about LW_EVENT_CELLULAR_TEMPORARY_XX
// Following are temporary event IDs for temporary investigations in cellular modem.
// Initially 8 Event Types are reserved. If more are required, they can be added.
// This list is growing in reverse order.
//
// Each radio stack (RCT, STE, Qualcomm) can overlap use of these Event Types because
// the Creator ID will be unique and can be used to differentiate.

#define LW_EVENT_CELLULAR_TEMPORARY_08                    0x00000af8
    ///< Temporary Cellular Issue 08.  This is used to detect modem-specific issues
    ///< such as verifying that a bug fix in a certain region of the world.
    ///< Once verified, this Logworthy Event Type is discontinued from use and later
    ///< recycled for a similar purpose.
    ///< Severity: Warning

#define LW_EVENT_CELLULAR_TEMPORARY_07                    0x00000af9
    ///< Temporary Cellular Issue 07.  This is used to detect modem-specific issues
    ///< such as verifying that a bug fix in a certain region of the world.
    ///< Once verified, this Logworthy Event Type is discontinued from use and later
    ///< recycled for a similar purpose.
    ///< Severity: Warning

#define LW_EVENT_CELLULAR_TEMPORARY_06                    0x00000afa
    ///< Temporary Cellular Issue 06.  This is used to detect modem-specific issues
    ///< such as verifying that a bug fix in a certain region of the world.
    ///< Once verified, this Logworthy Event Type is discontinued from use and later
    ///< recycled for a similar purpose.
    ///< Severity: Warning

#define LW_EVENT_CELLULAR_TEMPORARY_05                    0x00000afb
    ///< Temporary Cellular Issue 05.  This is used to detect modem-specific issues
    ///< such as verifying that a bug fix in a certain region of the world.
    ///< Once verified, this Logworthy Event Type is discontinued from use and later
    ///< recycled for a similar purpose.
    ///< Severity: Warning

#define LW_EVENT_CELLULAR_TEMPORARY_04                    0x00000afc
    ///< Temporary Cellular Issue 04.  This is used to detect modem-specific issues
    ///< such as verifying that a bug fix in a certain region of the world.
    ///< Once verified, this Logworthy Event Type is discontinued from use and later
    ///< recycled for a similar purpose.
    ///< Severity: Warning

#define LW_EVENT_CELLULAR_TEMPORARY_03                    0x00000afd
    ///< Temporary Cellular Issue 03.  This is used to detect modem-specific issues
    ///< such as verifying that a bug fix in a certain region of the world.
    ///< Once verified, this Logworthy Event Type is discontinued from use and later
    ///< recycled for a similar purpose.
    ///< Severity: Warning

#define LW_EVENT_CELLULAR_TEMPORARY_02                    0x00000afe
    ///< Temporary Cellular Issue 02.  This is used to detect modem-specific issues
    ///< such as verifying that a bug fix in a certain region of the world.
    ///< Once verified, this Logworthy Event Type is discontinued from use and later
    ///< recycled for a similar purpose.
    ///< Severity: Warning

#define LW_EVENT_CELLULAR_TEMPORARY_01                    0x00000aff
    ///< Temporary Cellular Issue 01.  This is used to detect modem-specific issues
    ///< such as verifying that a bug fix in a certain region of the world.
    ///< Once verified, this Logworthy Event Type is discontinued from use and later
    ///< recycled for a similar purpose.
    ///< Severity: Warning

#define LW_EVENT_CELLULAR_RANGE_END                       0x00000aff

//@}

#endif /* LW_EVENT_TYPES_H_ */

