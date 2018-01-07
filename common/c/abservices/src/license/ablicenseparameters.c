/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!--------------------------------------------------------------------------

    @file     ablicenseparameters.c
    @defgroup abparams   Parameters

    This API is used to create Parameters objects.  Parameters objects are used to create and
    configure Handler objects to retrieve information from the server
*/
/*
    See file description in header file.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/


#include "ablicenseparameters.h"
#include "ablicenseparametersprivate.h"
#include "nbcontextprotected.h"
#include "data_license_query.h"
#include "data_license_reply.h"
#include "abqalog.h"

/*! @{ */

static void LicenseParametersLogRequest(AB_LicenseParameters* pThis);

struct AB_LicenseParameters
{
    NB_Context*           context;
    data_license_query    query;
};


/* See header file for description. */ 
AB_DEF NB_Error
AB_LicenseParametersCreate(NB_Context* context, AB_LicenseParameters** parameters)
{
    AB_LicenseParameters* pThis = 0;
    NB_Error err = NE_OK;

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    err = data_license_query_init(NB_ContextGetDataState(context), &pThis->query);

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

/* See header file for description. */ 
AB_DEF NB_Error
AB_LicenseParametersDestroy(AB_LicenseParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_license_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    nsl_free(pThis);

    return NE_OK;
}

/* See header file for description. */ 
AB_DEF tpselt
AB_LicenseParametersToTPSQuery(AB_LicenseParameters* pThis)
{
    if (pThis)
    {
        // iterate requests only if QA logging enabled
        if (NB_ContextGetQaLog(pThis->context) != NULL)
        {
            LicenseParametersLogRequest(pThis);
        }

        return data_license_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
    }

    return 0;
}

/* See header file for description. */ 
AB_DEF NB_Error AB_LicenseParametersClone(AB_LicenseParameters* pThis, AB_LicenseParameters** clone)
{
    NB_Error err = NE_OK;
    AB_LicenseParameters* pClone = 0;

    if (!pThis || !clone)
    {
        return NE_INVAL;
    }
    *clone = 0;

    pClone = nsl_malloc(sizeof(*pClone));
    if (!pClone)
    {
        return NE_NOMEM;
    }
    nsl_memset(pClone, 0, sizeof(*pClone));

    err = data_license_query_copy(NB_ContextGetDataState(pThis->context), &pClone->query, &pThis->query);
    if (!err)
    {
        pClone->context = pThis->context;
        *clone = pClone;
    }
    else
    {
        nsl_free(pClone);
    }

    return err;
}

/* See header file for description. */
AB_DEF NB_Error AB_LicenseParametersSetLanguage(AB_LicenseParameters* pThis, const char* language)
{
    if (!pThis || !language)
    {
        return NE_INVAL;
    }

    return data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.language, language);
}

static const char*
ab_getCountry(const char* country)
{
    if (country)
    {
        return country;
    }

    return "USA";
}

static const char*
ab_getAction(AB_LicenseAction action)
{
    const char* actionStr = "list";

    if (action == AB_LicenseAction_Create)
    {
        actionStr = "create";
    }
    else if (action == AB_LicenseAction_Update)
    {
        actionStr = "update";
    }
    else if (action == AB_LicenseAction_Delete)
    {
        actionStr = "delete";
    }
    else if (action == AB_LicenseAction_Validate)
    {
        actionStr = "validate";
    }

    return actionStr;
}

/* See header file for description. */
AB_DEF NB_Error AB_LicenseParametersSetRequest(
    AB_LicenseParameters* pThis,
    const char* requestId,
    const char* transactionId,
    const char* vendorName,
    const char* vendorUserId,
    const char* country,
    const char* subscriberKey,
    AB_LicenseAction action)
{
    NB_Error err = NE_OK;
    data_util_state* pds = NB_ContextGetDataState(pThis->context);
    data_license_query* query = &pThis->query;

    err = err ? err : data_string_set(pds, &query->request.requestid, requestId);
    err = err ? err : data_string_set(pds,&query->request.license_request.transactionid, transactionId ? transactionId : "");
    err = err ? err : data_string_set(pds, &query->request.license_request.vendorname, vendorName);
    err = err ? err : data_string_set(pds, &query->request.license_request.vendoruserid, vendorUserId);
    err = err ? err : data_string_set(pds, &query->request.license_request.country, ab_getCountry(country));
    err = err ? err : data_string_set(pds, &query->request.license_request.subscriberkey, subscriberKey);
    err = err ? err : data_string_set(pds, &query->request.license_request.action, ab_getAction(action));
    return err;
}

AB_DEF NB_Error AB_LicenseParametersCreateLicense(
    AB_LicenseParameters* pThis,
    const char* key,
    const char* productId,
    const char* licRequestId,
    int *licIndex) 
{
    NB_Error err = NE_OK;
    data_util_state* pds = NB_ContextGetDataState(pThis->context);
    data_license_query* query = &pThis->query;
    data_license license;

    err = err ? err : data_license_init(pds, &license);
    err = err ? err : data_string_set(pds, &license.key, key);
    err = err ? err : data_string_set(pds, &license.productid, productId);
    err = err ? err : data_string_set(pds, &license.licrequestid, licRequestId);
    if (!err & !CSL_VectorAppend(query->request.license_request.vec_license, &license)) 
    {
        data_license_free(pds, &license);
        err = NE_NOMEM;
    }
    if (!err) {
        *licIndex = CSL_VectorGetLength(query->request.license_request.vec_license) - 1;
    }

    return err;
}

AB_DEF NB_Error AB_LicenseParametersAddBundle(
   AB_LicenseParameters* pThis,
   int licenseIndex,
   const char* bundleName,
   const char* bundleType)
{
    NB_Error err = NE_OK;
    const char* enddate = ""; //provide default empty enddate, client does not need to provide enddate for bundle
    data_bundle bundle;
    data_util_state* pds = NB_ContextGetDataState(pThis->context);
    data_license_query* query = &pThis->query;
    data_license* license = (data_license*)CSL_VectorGetPointer(query->request.license_request.vec_license, licenseIndex);

    if (!license)
    {
        err = NE_BADDATA;
    }

    err = err ? err : data_bundle_init(pds, &bundle);
    err = err ? err : data_string_set(pds, &bundle.name, bundleName);
    err = err ? err : data_string_set(pds, &bundle.enddate, enddate);
    err = err ? err : data_string_set(pds, &bundle.type, bundleType);
    if (!err && !CSL_VectorAppend(license->vec_bundle, &bundle))
    {
        data_bundle_free(pds, &bundle);
        err = NE_NOMEM;
    }

    return err;
}

AB_DEF NB_Error AB_LicenseParametersAddBundlePromoCode(
                                              AB_LicenseParameters* pThis,
                                              int licenseIndex,
                                              int bundleIndex,
                                              const char* bundlePromoCode)
{
    NB_Error err = NE_OK;

    data_util_state* pds = NB_ContextGetDataState(pThis->context);
    data_promo_code promo_code;
    data_bundle* bundle = NULL;
    data_license_query* query = &pThis->query;
    data_license* license = (data_license*)CSL_VectorGetPointer(query->request.license_request.vec_license, licenseIndex);

    if (!license)
    {
        err = NE_BADDATA;
    }

    bundle = (data_bundle*)CSL_VectorGetPointer(license->vec_bundle, bundleIndex);

    if (!bundle)
    {
        err = NE_BADDATA;
    }

    err = err ? err : data_promo_code_init(pds, &promo_code);
    err = err ? err : data_string_set(pds, &promo_code.value, bundlePromoCode);
    err = err ? err : data_promo_code_copy(pds, &bundle->promo_code, &promo_code);

    if (err)
    {
        data_promo_code_free(pds, &promo_code);
        err = NE_NOMEM;
    }

    return err;
}

void
LicenseParametersLogRequest(AB_LicenseParameters* pThis)
{
    data_util_state* dataState = NULL;

    int licenseCount = 0;
    int i = 0;

    if (!pThis)
    {
        return;
    }

    dataState = NB_ContextGetDataState(pThis->context);

    licenseCount = CSL_VectorGetLength(pThis->query.request.license_request.vec_license);

    for (i = 0; i < licenseCount; i++)
    {
        data_license* license = (data_license*)CSL_VectorGetPointer(pThis->query.request.license_request.vec_license, i);
        int bundleCount = CSL_VectorGetLength(license->vec_bundle);
        int j = 0;

        for (j = 0; j < bundleCount; j++)
        {
            data_bundle* bundle = (data_bundle*)CSL_VectorGetPointer(license->vec_bundle, j);

            AB_QaLogLicenseRequest(pThis->context,
                data_string_get(dataState, &pThis->query.request.requestid),
                data_string_get(dataState, &pThis->query.request.license_request.action),
                data_string_get(dataState, &pThis->query.request.license_request.subscriberkey),
                data_string_get(dataState, &pThis->query.request.license_request.vendorname),
                data_string_get(dataState, &pThis->query.request.license_request.country),
                data_string_get(dataState, &license->productid),
                data_string_get(dataState, &bundle->type),
                data_string_get(dataState, &bundle->name),
                data_string_get(dataState, &pThis->query.language));
        }
    }
}

/*! @} */
