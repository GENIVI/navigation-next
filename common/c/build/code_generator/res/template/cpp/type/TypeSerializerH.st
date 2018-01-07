TypeSerializerH(type) ::= <<
 /****************************************************************************/
/************       THIS IS GENERATED CODE. DO NOT MODIFY        ************/
/****************************************************************************/

/*!--------------------------------------------------------------------------
    @file     NBProtocol<type.name>Serializer.h
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

#ifndef <Define(type, "SERIALIZER")>
#define <Define(type, "SERIALIZER")>

#include "nbexp.h"
#include "NBProtocol<type.name>.h"
#include "TpsElement.h"

namespace protocol {
class <type.name>Serializer : public Base
{
public:
    <type.name>Serializer();
   ~<type.name>Serializer();

   static TpsElementPtr serialize(shared_ptr\<<type.name>\> obj<type.name>);
   static shared_ptr\<<type.name>\> deserialize(TpsElementPtr tp);

private:
};
}
#endif // <Define(type, "SERIALIZER")>
/*! @} */
>>

