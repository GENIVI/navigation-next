Handler(handler) ::= <<
/****************************************************************************/
/************       THIS IS GENERATED CODE. DO NOT MODIFY        ************/
/****************************************************************************/

/*!--------------------------------------------------------------------------
    @file     Tps<handler.name>Handler.h
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


#ifndef <Define(handler, "")>
#define <Define(handler, "")>

extern "C"
{
  #include "nbcontext.h"
  #include "cslnetwork.h"
}

#include "Handler.h"
#include "AsyncCallback.h"
#include "NBProtocol<handler.request.type.name>.h"
#include "NBProtocol<handler.response.type.name>.h"
#include "NBProtocol<handler.request.type.name>Serializer.h"
#include "NBProtocol<handler.response.type.name>Serializer.h"
#include "NBProtocolInvocationContext.h"
#include "NBProtocolInvocationContextSerializer.h"

typedef struct _<handler.name>HandlerQuery
{
    NB_NetworkQuery     networkQuery;
    void*               pThis;
}<handler.name>HandlerQuery;

using namespace nbmap;
namespace protocol
{

typedef shared_ptr\<AsyncCallbackWithRequest\<shared_ptr\<<handler.request.type.name> >, shared_ptr\<<handler.response.type.name> > > > <handler.name>CallbackPtr;

class OffBoard<handler.name>Handler : public Handler\<<handler.request.type.name>,<handler.response.type.name>\>
{
public:
    OffBoard<handler.name>Handler(NB_Context *context);
    ~OffBoard<handler.name>Handler();
    void Request(shared_ptr\<<handler.request.type.name>\> pParameters,<handler.name>CallbackPtr pCallback);
    void Cancel(void);
    bool IsInProgress();
    bool SetInvocationContext(InvocationContextSharedPtr invocationContext = InvocationContextSharedPtr());

private:
    NB_Context*                 m_context;

    shared_ptr\<<handler.request.type.name> > m_parameters;
   <handler.name>CallbackPtr   m_callback;

    <handler.name>HandlerQuery  m_Query;
    bool            m_inProgress;

    static void StaticNetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
    void NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
    static void StaticNetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);
};
}
#endif
/*! @} */
>>
