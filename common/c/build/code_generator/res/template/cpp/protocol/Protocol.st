Protocol(protocol) ::= <<
/****************************************************************************/
/************       THIS IS GENERATED CODE. DO NOT MODIFY        ************/
/****************************************************************************/

/*!--------------------------------------------------------------------------
    @file     NBProtocol<protocol.name>Protocol.h
    @defgroup protocol

*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of 
    California. Use of this information by anyone other than     
    authorized employees of TeleCommunication Systems, is granted only 
    under a written non-disclosure agreement, expressly 
    prescribing the scope and manner of such use.           

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef <Define(protocol, "")>
#define <Define(protocol, "")>

#include "nbexp.h"
#include "Handler.h"

<protocol.handlers : { handler |
#include "NBProtocol<handler.request.type.name>.h"
#include "NBProtocol<handler.response.type.name>.h"

}>
namespace protocol {

class NB_DLL <protocol.name>Protocol {

public:
    <protocol.name>Protocol() {};
    virtual ~<protocol.name>Protocol(){};
    
    <protocol.handlers : { handler | 
virtual shared_ptr\<Handler\<<handler.request.type.name>, <handler.response.type.name>\> \> Get<handler.name>Handler() = 0;}>

};

}
#endif // <Define(protocol, "")>
/*! @} */ 
>>
