Handler(handler) ::= <<
/*!--------------------------------------------------------------------------

    @file     nb<handler.name;format="toLower">handler.h
*/
/*
    (C) Copyright 2009 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef <Define(handler, "Handler")>
#define <Define(handler, "Handler")>

#include "nbcontext.h"
#include "nbhandler.h"
#include "nb<handler.name;format="toLower">information.h"
#include "nb<handler.name;format="toLower">parameters.h"

/*!
    @addtogroup nb<handler.name;format="toLower">handler
    @{
*/


typedef struct NB_<handler.name>Handler NB_<handler.name>Handler;


NB_DEC NB_Error NB_<handler.name>HandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_<handler.name>Handler** handler);
NB_DEC NB_Error NB_<handler.name>HandlerDestroy(NB_<handler.name>Handler* handler);
NB_DEC NB_Error NB_<handler.name>HandlerStartRequest(NB_<handler.name>Handler* handler, NB_<handler.name>Parameters* parameters);
NB_DEC NB_Error NB_<handler.name>HandlerCancelRequest(NB_<handler.name>Handler* handler);
NB_DEC nb_boolean NB_<handler.name>HandlerIsRequestInProgress(NB_<handler.name>Handler* handler);
NB_DEC NB_Error NB_<handler.name>HandlerGet<handler.name>Information(NB_<handler.name>Handler* handler, NB_<handler.name>Information** information);


/*! @} */

#endif //<Define(handler,"Handler")>

>>