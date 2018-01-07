InformationPrivate(handler, parameters, information) ::= <<
/*!--------------------------------------------------------------------------

    @file     <FileName(handler, "information", "h")>
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

#ifndef <Define(handler,"INFORMATIONPRIVATE")>
#define <Define(handler,"INFORMATIONPRIVATE")>

#include "nb<handler.name;format="toLower">information.h"
#include "tpselt.h"


/*!
    @addtogroup nb<handler.name;format="toLower">information
    @{
*/

NB_Error NB_<handler.name>InformationCreate(NB_Context* context, tpselt reply, NB_<handler.name>Information** information);

/*! @} */

#endif // <Define(handler,"INFORMATIONPRIVATE")>
>>