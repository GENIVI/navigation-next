/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!--------------------------------------------------------------------------

    @file abdoxygengroups.h
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*!

@defgroup abnetwork Network
Provides NAVBuilder server connectivity


@defgroup abnetworkconfiguration Network Configuration
@ingroup abnetwork
Functions necessary to define a network configuration used to communicate with a NAVBuilder server

@defgroup abdatastore Data Store
Functions necessary to store, retrieve, and delete data byte streams on the server

@defgroup abdatastoreparameters Data Store Parameters
@ingroup  abdatastore
The DataStoreParameters object defines the operation and parameters for a Data Store server request

@defgroup abdatastorehandler Data Store Handler
@ingroup  abdatastore
The DataStoreHandler object sends a Data Store request to a server, receives the reply,
and provides a way to retrieve the results

@defgroup abdatastoreinformation Data Store Information
@ingroup  abdatastore
The DataStoreInformation object is a result of a Data Store request


@defgroup abshare Share
Functions necessary to send place messages

@defgroup abshareparameters Share Parameters
@ingroup  abshare
The ShareParameters object defines the information needed for a send place message request

@defgroup absharehandler Share Handler
@ingroup  abshare
The ShareHandler object sends a Share request to a server, receives the reply,
and provides a way to retrieve the results

@defgroup abshareinformation Share Information
@ingroup  abshare
The ShareInformation object is a result of a Share request


@defgroup abspeech Speech Recognition
Functions necessary to utilize the speech recognition engine

@defgroup abspeechrecognitionparameters Speech Recognition Parameters
@ingroup  abspeech
The SpeechRecognitionParameters object defines the information for a Speech Recognition request

@defgroup abspeechrecognitionhandler Speech Recognition Handler
@ingroup  abspeech
The SpeechRecognitionHandler object sends a Speech Recognition request to a server, receives the reply,
and provides a way to retrieve the results

@defgroup abspeechrecognitioninformation Speech Recognition Information
@ingroup  abspeech
The SpeechRecognitionInformation object is a result of a Speech Recognition request


@defgroup abspeechstats Speech Recognition Statistics
@ingroup abspeech
Functions necessary to provide statistical information for the speech recognition engine

@defgroup abspeechstatisticsparameters Speech Statistics Parameters
@ingroup  abspeechstats
The SpeechStatisticsParameters object defines the information needed to submit speech recognition information

@defgroup abspeechstatisticshandler Speech Statistics Handler
@ingroup  abspeechstats
The SpeechStatisticsHandler object sends a Speech Statistics request to a server, receives the reply,
and provides a way to retrieve the results

@defgroup abspeechstatisticsinformation Speech Statistics Information
@ingroup  abspeechstats
The SpeechStatisticsInformation object is a result of a Speech Statistics request


@defgroup absync Synchronization
Functions necessary to synchronize client and server databases

@defgroup absyncparameters Synchronization Parameters
@ingroup  absync
The SynchronizationParameters object defines the information needed to submit a Synchronization request

@defgroup absynchandler Synchronization Handler
@ingroup  absync
The SynchronizationHandler object sends a Synchronization request to a server, receives the reply,
and provides a way to retrieve the results

@defgroup absyncinformation Synchronization Information
@ingroup  absync
The SynchronizationInformation object is a result of a Synchronization request


@defgroup absyncstatus Synchronization Status
@ingroup absync
Functions necessary to request the current status of server databases for synchronization

@defgroup absyncstatusparameters Synchronization Status Parameters
@ingroup  absyncstatus
The SynchronizationStatusParameters object defines the information needed for a Synchronization Status request

@defgroup absyncstatushandler Synchronization Status Handler
@ingroup  absyncstatus
The SynchronizationStatusHandler object sends a Synchronization Status request to a server, receives the reply,
and provides a way to retrieve the results

@defgroup absyncstatusinformation Synchronization Status Information
@ingroup  absyncstatus
The SynchronizationStatusInformation object is a result of a Synchronization Status request


@defgroup abfileset File Set
Functions necessary to request a file set from the server

@defgroup abfilesetparameters File Set Parameters
@ingroup  abfileset
The FileSetParameters object defines the information necessary to make a request for file sets

@defgroup abfilesethandler File Set Handler
@ingroup  abfileset
The FileSetHandler object sends a File Set request to a server, receives the reply,
and provides a way to retrieve the results

@defgroup abfilesetinformation File Set Information
@ingroup  abfileset
The FileSetInformation object is a result of a File Set request


@defgroup abfilesetstatus File Set Status
@ingroup abfileset
Functions necessary to request the current status of file sets on the server

@defgroup abfilesetstatusparameters File Set Status Parameters
@ingroup  abfilesetstatus
The FileSetParameters object defines the information needed to request the status of file sets on the server

@defgroup abfilesetstatushandler File Set Status Handler
@ingroup  abfilesetstatus
The FileSetHandler object sends a File Set Status request to a server, receives the reply,
and provides a way to retrieve the results

@defgroup abfilesetstatusinformation File Set Status Information
@ingroup  abfilesetstatus
The FileSetInformation object is a result of a File Set Status request


@defgroup abprofile Profile
Functions necessary to request profile settings from the server

@defgroup abprofileparameters Profile Parameters
@ingroup  abprofile
The ProfileParameters object defines the information needed to request profile settings from the server

@defgroup abprofilehandler Profile Handler
@ingroup  abprofile
The ProfileHandler object sends a Profile request to a server, receives the reply,
and provides a way to retrieve the results

@defgroup abprofileinformation Profile Information
@ingroup  abprofile
The ProfileInformation object is a result of a Profile request


@defgroup abservermessage Server Message
Functions necessary to request server messages from the server

@defgroup abservermessageparameters Server Message Parameters
@ingroup  abservermessage
The ServerMessageParameters object defines the information needed to request server messages

@defgroup abservermessagehandler Server Message Handler
@ingroup  abservermessage
The MessageHandler object sends a Server Message request to a server, receives the reply,
and provides a way to retrieve the results

@defgroup abservermessageinformation Server Message Information
@ingroup  abservermessage
The MessageInformation object is a result of a Server Message request

@defgroup abqalog QA Logging
Functions necessary to create and upload QA Log files

@defgroup abqaloginstance QA Logging
@ingroup  abqalog
The QaLog object is used to create and configure QA Logging for a session

@defgroup abqalogparameters QA Log Parameters
@ingroup  abqalog
The QaLogParameters object defines the network and file to upload

@defgroup abqaloghandler QA Log Handler
@ingroup  abqalog
The QaLogHandler object uploads a QA Log File to a server, receives the reply,
and provides a way to retrieve the results

@defgroup abqaloginformation QA Log Information
@ingroup  abqalog
The QaLogInformation object is a result of a QA Log Upload request

@defgroup abauthentication Authentication
Functions necessary to ...

@defgroup abauthenticationhandler Authentication Handler
@ingroup  abauthentication
The AuthenticationHandler object sends a request to a server, receives the reply

@defgroup abauthenticationinformation Authentication Information
@ingroup  abauthentication
The AuthenticationInformation object is a result of a Authentication request

@defgroup abauthenticationparameters Authentication Parameters
@ingroup  abauthentication
The AuthenticationParameters object defines the information ...

@defgroup abversion Version
Provides version and copyright information about the component

@defgroup abserverversion Server Version
Retrieves version information about the Atlasbook server

@defgroup abserverversionparameters Server Version Parameters
@ingroup  abserverversion
The ServerVersionParameters object defines the type of version information to retrieve

@defgroup abserverversionhandler Server Version Handler
@ingroup  abserverversion
The ServerVersionHandler object sends a request to a server, receives the reply
and provides a way to retrieve the results

@defgroup abserverversioninformation Server Version Information
@ingroup  abserverversion
The ServerVersionInformation object contains the server version

@defgroup ablicense License
Functions necessary to query and apply license to a product.

@defgroup ablicensehandler License Handler
@ingroup  ablicense
The License Handler object sends a request to a server, receives the reply

@defgroup ablicenseinformation License Information
@ingroup  ablicense
The License Information object is a result of a License request

@defgroup ablicenseparameters License Parameters
@ingroup  ablicense
The License Parameters object defines the information for License Handler

*/
