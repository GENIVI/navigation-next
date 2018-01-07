TpsProtocolCpp(protocol) ::= <<
/****************************************************************************/
/************       THIS IS GENERATED CODE. DO NOT MODIFY        ************/
/****************************************************************************/

/*!--------------------------------------------------------------------------
    @file     Tps<protocol.name>Protocol.cpp
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

#include "Tps<protocol.name>Protocol.h"
<protocol.handlers : { handler |
#include "Tps<handler.name>Handler.h"

}>

using namespace protocol;

Tps<protocol.name>Protocol::Tps<protocol.name>Protocol(NB_Context* context):m_context(context)
{
}

Tps<protocol.name>Protocol::~Tps<protocol.name>Protocol()
{
}

<protocol.handlers : { handler |
shared_ptr\<Handler\<<handler.request.type.name>, <handler.response.type.name>\> \> Tps<protocol.name;format="toUpperFirst">Protocol::Get<handler.name>Handler()
{
    return shared_ptr\<Handler\<<handler.request.type.name>, <handler.response.type.name>\> \>(new OffBoard<handler.name>Handler(m_context));
\}
}>

/*! @} */
>>