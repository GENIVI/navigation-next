InformationC(handler, parameters, information) ::= <<
/*!--------------------------------------------------------------------------

    @file     nb<handler.name;format="toLower">information.c
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */


#include "nbcontextprotected.h"
#include "<FileName(information.type.name, "", "h")>"
#include "<FileName(information.type.name, "private", "h")>"
#include "nbutilityprotected.h"
#include "<information.wireName;format="dashToUnderscore">.h"
#include "datautil.h"


struct NB_<information.type.name>
{
    NB_Context*         context;
    NB_Location        location;
};


NB_Error
NB_<information.type.name>Create(NB_Context* context, tpselt reply, NB_<information.type.name>** information)
{
    NB_<information.type.name>* pThis = 0;
    NB_Error err = NE_OK;
    data_util_state* dataState = 0;
    <information.wireName> replyData = { { { 0 } } };

    if (!context || !reply || !information)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;
    dataState = NB_ContextGetDataState(context);

    err = data_<information.wireName>_from_tps(dataState, &replyData, reply);
    err = err ? err : SetNIMLocationFromLocation(&pThis->location, dataState, &replyData.location);
    data_<information.wireName>_free(dataState, &replyData);

    if (!err)
    {
        *information = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}

<information.type.fields: {field | 

NB_DEF NB_Error
NB_<information.type.name>Get<field.name;format="toUpperFirst">(NB_<information.type.name>* pThis, NB_<field.type.name>* <field.name>)
{
    if (!pThis || !<field.name>)
    {
        return NE_INVAL;
    \}

    *<field.name> = pThis-><field.name>;

    return NE_OK;
\}

}>


NB_DEF NB_Error
NB_<handler.name>InformationDestroy(NB_<handler.name>Information* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    nsl_free(pThis);

    return NE_OK;
}


/*! @} */

>>