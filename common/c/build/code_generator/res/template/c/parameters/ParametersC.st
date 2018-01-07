ParametersC(handler, parameters) ::= <<
/*!--------------------------------------------------------------------------

    @file     <FileName(parameters.type.name, "", "c")>
    @defgroup nbparams Parameters

    This API is used to create Parameters objects.  Parameters objects are used to create and
    configure Handler objects to retrieve information from the server
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
#include "<FileName(parameters.type.name, "", "h")>"
#include "<FileName(parameters.type.name, "private", "h")>"
#include "<parameters.wireName;format="dashToUnderscore">.h"
#include "datautil.h"


struct NB_<parameters.type.name> {
    NB_Context*                 context;
    <parameters.type.fields: { field | <TypeToTypeName(field.type)> <field.name>;
    }>
    data_<parameters.wireName> query;
};


NB_DEF NB_Error
NB_<parameters.type.name>Create(NB_Context* context, <parameters.type.fields: { field | <TypeToTypeName(field.type)><if(!field.type.simple)>*<endif> <field.name>, }>NB_<parameters.type.name>** parameters)
{
    NB_<parameters.type.name>* pThis = 0;
    data_util_state* dataState = 0;
    NB_Error err = NE_OK;

    if (!point || !parameters)
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

    dataState = NB_ContextGetDataState(pThis->context);
    err = data_<parameters.wireName>_init(dataState, &pThis->query);

    <parameters.type.fields: { field | <parameters/ParametersCSetField("parameters/ParametersCSetField", field,[],"pThis->query.",";")>}>
	
    data_string_set(dataState, &pThis->query.position.variant , "point");

    if (!err)
    {
        *parameters = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}


NB_DEF NB_Error
NB_<parameters.type.name>Destroy(NB_<parameters.type.name>* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_<parameters.wireName;format="dashToUnderscore">_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    nsl_free(pThis);

    return NE_OK;
}

tpselt NB_<parameters.type.name>ToTPSQuery(NB_<handler.name>Parameters* pThis)
{
    return data_<parameters.wireName;format="dashToUnderscore">_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
}

>>