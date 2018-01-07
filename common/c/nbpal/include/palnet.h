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

    @file     palnet.h
    @date     1/5/2009
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    The following functions provide platform-independent network I/O support.
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

/*! @{ */

#ifndef PALNET_H
#define	PALNET_H

#include "pal.h"

#define IPV4_DNS_RESULT_ADDRESSES   4
#define IPV6_DNS_RESULT_ADDRESSES   4
#define MAX_DNS_RESULT_ADDRESSES    (IPV4_DNS_RESULT_ADDRESSES + IPV6_DNS_RESULT_ADDRESSES)

/*! External reference for a PAL network connection.

    This typedef provides an opaque reference to a created PAL network connection.
*/
typedef struct PAL_NetConnection PAL_NetConnection;

/*! Current status of a PAL network connection. */
typedef enum
{
    PNCS_Undefined = 0,         /*!< Network connection object has not yet been created */
    PNCS_Created,               /*!< Network connection object has been created */
    PNCS_Initialized,           /*!< Network connection object has been initialized */
    PNCS_Resolving,             /*!< Network connection object is resolving the specified host name */
    PNCS_Connecting,            /*!< Network connection object is attempting to establish a
                                     connection with server*/
    PNCS_Connected,             /*!< Network connection object has established a connection */
    PNCS_Failed,                /*!< An attempted network connection has failed */
    PNCS_Error,                 /*!< An error has occurred on an established network connection */
    PNCS_Closing,               /*!< Network connection object is attempting to close */
    PNCS_Closed,                /*!< Network connection object has been closed */
    PNCS_ProxyAuthRequired,     /*!< The client must first authenticate itself with the proxy. */
    PNCS_PppStateChanged        /*!< PPP connection state is changed (BREW only) */
} PAL_NetConnectionStatus;

/*! Type of protocol for a PAL network connection. */
typedef enum
{
    PNCP_Undefined = 0, /*!< Undefined network connection protocol */
    PNCP_TCP,           /*!< TCP network connection protocol */
    PNCP_HTTP,          /*!< HTTP network connection protocol */
    PNCP_HTTPS,         /*!< HTTPS network connection protocol */
    PNCP_TCPTLS,        /*!< TCP connection with TLS */
} PAL_NetConnectionProtocol;

/*! PPP connection states. */
typedef enum {
   PNPS_PPP_Invalid = 0,
   PNPS_PPP_Opening,
   PNPS_PPP_Open,
   PNPS_PPP_Closing,
   PNPS_PPP_Closed,
   PNPS_PPP_Sleeping,
   PNPS_PPP_Asleep,
   PNPS_PPP_Waking
} PAL_NetPppState;

/*! Callback for PAL network connection status change notification.

This typedef defines the function signature required for the network status notification callback.

This callback will be called when a network connection's status has changed.
*/
typedef void (*PAL_NetConnectionStatusCallback)(
    void* userData,                 /*!< caller-supplied reference for this connection */
    PAL_NetConnectionStatus status	/*!< current PAL network connection status */
    );

/*! Callback for PAL network connection data sent notification.

   This typedef defines the function signature required for network data sent notification callback.
   
   This callback will be called when the connection has successfully sent the bytes referenced
   in the bytes parameter.  The referenced bytes will be freed upon return of this callback, so they
   should be duplicated if they are needed beyond the scope of this call.
*/
typedef void (*PAL_NetDataSentCallback)(
    void* userData,	    /*!< caller-supplied reference for this connection */
    const byte* bytes,  /*!< bytes sent */
    uint32 count        /*!< count of bytes sent */
);

/*! Callback for PAL network connection data received notification.

   This typedef defines the function signature required for network data received notification callback.

   This callback will be called when the connection has received the bytes referenced in the parameter.
   The referenced bytes will be freed upon return of this callback, so they should be copied if they
   are needed beyond the scope of this call.
*/
typedef void (*PAL_NetDataReceivedCallback)(
    void* userData,	    /*!< caller-supplied reference for this connection */
    const byte* bytes,  /*!< bytes received */
    uint32 count        /*!< count of bytes received */
);


/*! Callback for PAL network DNS result notification.

This typedef defines the function signature required for the network DNS result notification callback.

This callback will be called when there is a DNS resolution or error.
*/
typedef void (*PAL_NetDnsResultCallback)(
    void* userData,                 /*!< caller-supplied reference for this connection */
    PAL_Error errorCode,            /*!< PAL error code */
    const char* hostName,           /*!< host name */
    uint32 const* const* addresses, /*!< resolved addresses. First IPV4_DNS_RESULT_ADDRESSES addresses are IPv4, next IPV6_DNS_RESULT_ADDRESSES are IPv6*/
    uint32 addressCount             /*!< address count */
    );


/*! PAL HTTP network specific callback for http response status code notification.

   This typedef defines the function signature required for http network response status notification callback.
   This callback will be called upon receiving the http response code for the http request made to the server.
*/
typedef void (*PAL_NetHttpResponseStatusCallback)(
    PAL_Error errorCode,       /*!< PAL error code. */
    void*     userData,        /*!< caller-supplied reference for this connection */
    void*     requestData,     /*!< caller-supplied reference when request is made using PAL_NetHttpSend. It acts as response identifier (to a request) when multiple requests are made using PAL_NetHttpSend */
    uint32    contentLength    /*!< total number of bytes expected to be received in the HTTP response's body, or -1 if unknown */
);

/*! PAL HTTP network specific callback for data received notification.

   This typedef defines the function signature required for network data received notification callback.

   This callback will be called when the connection has received the bytes referenced in the parameter.
   The referenced bytes will be freed upon return of this callback, so they should be copied if they
   are needed beyond the scope of this call. If there's a error on request or received byte count is O, it would be the last call of callback for that particular request.
*/
typedef void (*PAL_NetHttpDataReceivedCallback)(
    void*       userData,        /*!< caller-supplied reference for this connection */
    void*       requestData,     /*!< caller-supplied reference when request is made using PAL_NetHttpSend. It acts as response identifier (to a request) when multiple requests are made using PAL_NetHttpSend */
    PAL_Error   errorCode,       /*!< PAL error code */
    const byte* bytes,           /*!< bytes received */
    uint32      count            /*!< count of bytes received */
);

/*! PAL HTTP network specific callback for HTTP response headers notification.

    This typedef defines the function signature required for HTTP network response headers callback.

    This callback will be called upon receiving the HTTP response headers for the HTTP request made to the server.

    @todo: It is simple to return entire string of response headers to the user as request headers (see
           PAL_NetHttpSend for details). Because it is harder to parse this string to field names and contents
           for current implementation of HTTP (simply use TCP). If some clients could directly get the field
           names and contents from local API. They should be concatenated as format:
           (field-name1): (field-content1)\r\n
           (field-name2): (field-content2)\r\n
           ...
           \r\n\r\n
*/
typedef void (*PAL_NetHttpResponseHeadersCallback)(
    void*       userData,       /*!< caller-supplied reference for this connection */
    void*       requestData,    /*!< caller-supplied reference when request is made using PAL_NetHttpSend. It acts as response identifier (to a request) when multiple requests are made using PAL_NetHttpSend */
    const char* responseHeaders /*!< Entire string of response headers */
);

/*
 *! TLS parameters
 */
typedef struct
{
    const char*  name;      /*!< CN value to be expected on the server's X509 certificate */
    uint32       override;  /*!< Trust override bitmap */
} PAL_NetTLSConfig;

#define  PAL_TLS_OVERRIDE_NONE          0          /*! Don't override anything, every aspects should be checked. */
#define  PAL_TLS_OVERRIDE_HOST          0x1        /*! Don't try to compare the names. */
#define  PAL_TLS_OVERRIDE_TRUST_ROOT    0x2        /*! Trust this certificate as a root during chaining operation. */
#define  PAL_TLS_OVERRIDE_EXPIRY        0x4        /*! Don't check the dates on the cert, always try to parse cert. */
#define  PAL_TLS_OVERRIDE_ALL           0xffffffff /*! Skip all checks at client side. */

/*
 * Copy PAL_NetTLSConfig structure
 */
#define COPY_TLSCONFIG(targetobject, sourceobject) do {   \
    nsl_memcpy(&(targetobject), &(sourceobject), sizeof(targetobject));  \
} while (0)

/*! Struct to define network connection callback info.

    This struct specifies desired network notification callbacks and user data for a PAL_NetConnection.
*/
typedef struct
{
    PAL_NetConnectionStatusCallback   netStatusCallback;          /*!< callback for network status notification, NULL if none desired */
    PAL_NetDataSentCallback           netDataSentCallback;        /*!< callback for network data sent notification, NULL if none desired */
    PAL_NetDataReceivedCallback       netDataReceivedCallback;    /*!< callback for network data received notification, NULL if none desired */
    PAL_NetDnsResultCallback          netDnsResultCallback;       /*!< callback for DNS result notification, NULL if none desired */
    PAL_NetHttpResponseStatusCallback netHttpResponseStatusCallback;  /*!< callback for http network response status notification, NULL if none desired. Invalid for TCP connection. */
    PAL_NetHttpDataReceivedCallback   netHttpDataReceivedCallback;  /*!< callback for http network data received notification, NULL if none desired. Invalid for TCP connection. */
    PAL_NetHttpResponseHeadersCallback  netHttpResponseHeadersCallback; /*!< callback for HTTP network response headers notification, NULL if none desired. Invalid for TCP connection. */
    void*                             userData;                   /*!< reference object to be returned with network notification callbacks */

    PAL_NetTLSConfig                  tlsConfig;
} PAL_NetConnectionConfig;

/*! Initialize platform-specific networking libraries and resources.

    This function, typically called at application start up, performs any necessary
    platform-specific library initialization.  PAL_NetShutdown should be called at
    application exit to free any runtime resources allocated by this call.  More than
    one call to this function before making a call to PAL_NetShutdown will have no effect.

    @return PAL error code

    @see PAL_NetShutdown
*/
PAL_DEC PAL_Error
PAL_NetInitialize(
    PAL_Instance* pal    /*!< PAL instance */
    );

/*! Terminate and release any platform-specific networking libraries and resources.

    This function, typically called at application exit, terminates and releases any
    platform-specific network libraries and resources initialized at startup via the
    PAL_NetInitialize function.  A call to this function without calling PAL_NetInitialize
    first will have no effect.

    @return PAL error code

    @see PAL_NetInitialize
*/
PAL_DEC PAL_Error
PAL_NetShutdown(
    PAL_Instance* pal    /*!< PAL instance */
    );

/*! Create and return a reference to a PAL network connection object.

    This function creates an instance of a PAL network connection object and returns
    a reference to it. Calls to set desired callbacks and to establish a connection
    to a host can be made using the reference pointer returned by this function.

    The network connection object created by this call should be eventually destroyed
    when no longer needed by calling PAL_NetDestroyConnection.

    @return PAL error code
    
    @see PAL_NetDestroyConnection
*/
PAL_DEC PAL_Error
PAL_NetCreateConnection(
    PAL_Instance* pal,			        /*!< PAL instance */
    PAL_NetConnectionProtocol protocol, /*!< desired protocol for the created PAL network connection */
    PAL_NetConnection** netConn         /*!< points to a pointer for the created PAL network connection */ 
    );

/*! Open a PAL network connection to the specified host.

    This function initiates a connection to the specified host and port asynchronously.  This
    call will fail immediately if the PAL_NetInitialize call has not been called at application
    startup or if the given PAL network connection object is already connected to a host.
    
    If the specified connection was created to use the HTTP or HTTPS protocol, a port value of zero
    may be specified, in which case the default port value for the protocol will be used.
    
    Because the network connection success or failure will be reported via a status notification
    callback, a PAL_NetSetConnectionStatusCallback should be specified in the PAL_NetConnectionConfig
    structure that is passed to this function.

    @return PAL error code

    @see PAL_NetCloseConnection
*/
PAL_DEC PAL_Error
PAL_NetOpenConnection(
    PAL_NetConnection* netConn,             /*!< PAL network connection */
    PAL_NetConnectionConfig* netConnConfig, /*!< callbacks and user data to be used for this connection */
    const char* hostName,	                /*!< host name to connect to */
    uint16 port				                /*!< port to connect to; for HTTP/HTTPS, a value of zero will use the default port for that protocol */
    );

/*! Sends bytes on the specified PAL network connection.

    This function queues up the given data to be sent asynchronously to the currently connected host.
    This call will fail immediately if the given PAL network connection object is not connected to a host.

    This function may be called on a network connection using TCP, HTTP, or HTTPS protocol.
    For a HTTP or HTTPS connection, a default "POST" verb will be used, and the the Accept header will
    specify all media types.  To specify these as well as the desired request object or additional headers,
    the PAL_NetHttpSend function should be used.

    Because the success or failure of this send will be reported asynchronously -- that is via the current
    send data callback (or the status notification callback if a network error occurs) -- callbacks should
    be set specified in the PAL_NetConnectionConfig structure when opening a connection.

    @see PAL_NetOpenConnection
    @see PAL_NetHttpSend

    @return PAL error code
*/
PAL_DEC PAL_Error
PAL_NetSend(
    PAL_NetConnection* netConn,	/*!< PAL network connection */
    const byte* bytes,          /*!< bytes to send */
    uint32 count			    /*!< count of bytes to send */
    );

/*! Send bytes on the specified PAL HTTP/HTTPS connection.

    This function queues up the given data to be sent asynchronously to the currently connected host via HTTP/HTTPS.
    This call will fail immediately if the given PAL network connection object is not connected or is a connection that
    is not using the HTTP or HTTPS protocol.

    Because the success or failure of this send will be reported asynchronously -- that is via the current
    send data callback (or the status notification callback if a network error occurs) -- callbacks should
    be set specified in the PAL_NetConnectionConfig when opening a connection.

    @see PAL_NetOpenConnection
    @see PAL_NetSend

    @return PAL error code
*/
PAL_DEC PAL_Error
PAL_NetHttpSend(
    PAL_NetConnection* netConn,     /*!< PAL network connection; must be created for HTTP/HTTPS protocol */
    const byte* bytes,              /*!< bytes to send */
    uint32 count,                   /*!< count of bytes to send */
    const char* verb,               /*!< request verb; if NULL, "POST" is used */
    const char* object,             /*!< request object; NULL if not needed */
    const char* acceptType,         /*!< client-allowed response type; if NULL all media types is used */
    const char* additionalHeaders,  /*!< additional headers to be added to request; NULL if none  */
    void*       requestData         /*!< Caller's reference for request; Reference is passed back to user in PAL_NetHttpResponseStatusCallback, PAL_NetHttpDataReceivedCallback and PAL_NetHttpResponseHeadersCallback. Can be NULL */
    );

/*! Get the current network connection object status.

    This function returns the current status of the given PAL network connection object.

    @return PAL error code
*/
PAL_DEC PAL_NetConnectionStatus
PAL_NetGetStatus(
    PAL_NetConnection* netConn  /*!< PAL network connection */
    );

/*! Closes the specified PAL network connection.

    This function synchronously closes the currently open PAL network connection.  Attempting
    to close a connection that is not currently open will have no effect.

    @return PAL error code

    @see PAL_NetOpenConnection
*/
PAL_DEC PAL_Error
PAL_NetCloseConnection(
    PAL_NetConnection* netConn  /*!< PAL network connection */
    );

/*! Destroys the specified PAL network connection object.

    This function destroys the specified PAL network connection object.  The specified network connection
    object pointer is invalid after this call is made.

    If the given PAL network connection object currently has a connection open, this call will close
    the connection before destroying the object.

    @return PAL error code

    @see PAL_NetCreateConnection
*/
PAL_DEC PAL_Error
PAL_NetDestroyConnection(
    PAL_NetConnection* netConn  /*!< PAL network connection to destroy; invalid after this call */
    );

/*! Returns PPP connection state (BREW only).

    @return PAL error code
*/
PAL_DEC PAL_Error
PAL_NetGetPppState(
    PAL_NetConnection* netConn,  /*!< PAL network connection */
    PAL_NetPppState* state       /*!< Output PPP state */
    );

PAL_DEC PAL_Error
PAL_NetIsProxyNeeded(
		const char* hostname /*!<>*/
		);

/*! Sets the username and password for the proxy.

    If user sets the same username and password are used currently, this
    function returns false, user needs to call this function again to set
    the different username or password. This logic is used to avoid the
    dead loop for the functions of PAL_NetSetProxyCredentials and
    PAL_NetProxyCredentialsCallbackFunction.

    @return TRUE if the new username or password is different with current,
            FALSE otherwise.
*/
PAL_DEC nb_boolean
PAL_NetSetProxyCredentials(
    const char* username,       /*!< Username to set for proxy */
    const char* password        /*!< Password to set for proxy */
    );

/*! Callback for proxy credentials required notification.

    This typedef defines the function signature required for the proxy
    credentials required notification callback.

    This callback will be called when proxy credentials are required.
*/
typedef void (*PAL_NetProxyCredentialsCallbackFunction)(
    const char* currentUsername,    /*!< Current username for proxy */
    const char* currentPassword,    /*!< Current password for proxy */
    void* userData                  /*!< User data */
    );

/*! Defines the notify callback to invoke and specify the user data to be
    supplied to the callback.
*/
typedef struct
{
    PAL_NetProxyCredentialsCallbackFunction callbackFunction;
    void*                                   callbackData;
} PAL_NetProxyCredentialsCallback;

/*! Add a callback for proxy credentials required notification.

    Both function pointer and user data are the signatures of callback. If
    they are both same with an existing callback, the parameter callback is
    ignored.

    @return None
*/
PAL_DEC void
PAL_NetAddProxyCredentialsCallback(PAL_NetProxyCredentialsCallback* callback);

/*! Remove a callback for proxy credentials required notification.

    Both function pointer and user data are the signature of callback. If
    they are both same with an existing callback, the callback will be
    removed.

    @return None
*/
PAL_DEC void
PAL_NetRemoveProxyCredentialsCallback(PAL_NetProxyCredentialsCallback* callback);

/*! @} */

#endif
