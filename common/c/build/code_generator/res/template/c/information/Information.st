Information(handler, parameters, information) ::= <<
/*!--------------------------------------------------------------------------

    @file     nb<handler.name;format="toLower">information.h
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

#ifndef <Define(handler, "Information")>
#define <Define(handler, "Information")>

#include "paltypes.h"

/*!
    @addtogroup nb<handler.name;format="toLower">information
    @{
*/


typedef struct NB_<handler.name>Information NB_<handler.name>Information;

NB_DEC NB_Error NB_<handler.name>InformationGetLocation(NB_<handler.name>Information* information, NB_Location* location);
NB_DEC NB_Error NB_<handler.name>InformationDestroy(NB_<handler.name>Information* information);


/*! @} */

#endif// <Define(handler,"Information")>

>>