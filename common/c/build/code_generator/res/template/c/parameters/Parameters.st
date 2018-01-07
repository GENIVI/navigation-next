Parameters(handler, parameters) ::= <<
/*!--------------------------------------------------------------------------

    @file     nb<handler.name;format="toLower">parameters.h
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

#ifndef <Define(handler,"Parameters")>
#define <Define(handler,"Parameters")>

#include "paltypes.h"
#include "navpublictypes.h"


/*!
    @addtogroup nb<handler.name;format="toLower">parameters
    @{
*/

typedef struct NB_<handler.name>Parameters NB_<handler.name>Parameters;

NB_DEC NB_Error NB_<handler.name>ParametersCreate(NB_Context* context, NB_LatitudeLongitude* point, nb_boolean routable, NB_<handler.name>Parameters** parameters);
NB_DEC NB_Error NB_<handler.name>ParametersCreateWithScale(NB_Context* context, NB_LatitudeLongitude* point, nb_boolean routable, double scale, NB_<handler.name>Parameters** parameters);
NB_DEC NB_Error NB_<handler.name>ParametersDestroy(NB_<handler.name>Parameters* parameters);

/*! @} */

#endif

>>
