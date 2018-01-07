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

    @file     data_feature.c
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

/*! @{ */

#include "data_feature.h"
#include "vec.h"

NB_Error
data_feature_init(data_util_state* pds, data_feature* pdf)
{
    NB_Error err = NE_OK;

    pdf->endDate = 0;
    err = err ? err : data_string_init(pds, &pdf->name);
    err = err ? err : data_string_init(pds, &pdf->type);
    err = err ? err : data_string_init(pds, &pdf->region);
    pdf->exp_app_feature = FALSE;

    return err;
}

void
data_feature_free(data_util_state* pds, data_feature* pdf)
{
    data_string_free(pds, &pdf->type);
    data_string_free(pds, &pdf->name);
    data_string_free(pds, &pdf->region);
    pdf->exp_app_feature = FALSE;
}


NB_Error
data_feature_from_tps(data_util_state* pds, data_feature* pdf, tpselt ce)
{
    NB_Error err = NE_OK;

	data_feature_free(pds, pdf);

    err = data_feature_init(pds, pdf);

    pdf->endDate = te_getattru(ce, "enddate");
    err = err ? err : data_string_from_tps_attr(pds, &pdf->name, ce, "name");
    err = err ? err : data_string_from_tps_attr(pds, &pdf->type, ce, "type");

    //optional V6 attribute, if not available set to empty string
    err = err ? err : data_string_set(pds, &pdf->region, te_getattrc(ce, "region"));

    // Check if feature belongs to 3rd party external app
    if (err == NE_OK)
    {
        tpselt te = NULL;
        te = te_getchild(ce, "external-app");
        pdf->exp_app_feature = (nb_boolean)(te != NULL);
    }

    return err;
}

NB_Error
data_feature_copy(data_util_state* pds, data_feature* pdf_dest, data_feature* pdf_src)
{
    NB_Error err = NE_OK;

    pdf_dest->endDate = pdf_src->endDate;
    err = err ? err : data_string_copy(pds, &pdf_dest->name, &pdf_src->name);
    err = err ? err : data_string_copy(pds, &pdf_dest->type, &pdf_src->type);

    //optional v6 attribute
    err = err ? err : data_string_copy(pds, &pdf_dest->region, &pdf_src->region);

    if (err == NE_OK)
    {
        pdf_dest->exp_app_feature = pdf_src->exp_app_feature;
    }

    return err;
}

/*! @} */
