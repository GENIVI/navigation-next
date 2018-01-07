ParametersPrivate(handler, parameters) ::= <<
/*!--------------------------------------------------------------------------

    @file     <FileName(handler, "parametersprivate", "h")>
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

#ifndef <Define(handler,"PARAMETERSPRIVATE")>
#define <Define(handler,"PARAMETERSPRIVATE")>


#include "<handler.name;format="toLower">parameters.h"
#include "tpselt.h"


/*! 
    @addtogroup nb<handler.name;format="toLower">parameters
    @{
*/

tpselt NB_<handler.name>ParametersToTPSQuery(NB_<handler.name>Parameters* parameters);


/*! @} */

#endif // <Define(handler,"PARAMETERSPRIVATE")>

>>