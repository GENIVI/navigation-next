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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * data_position.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_position.h"

NB_Error
data_position_init(data_util_state* dataState, data_position* position)
{
    NB_Error err = NE_OK;

    err = err ? err : data_point_init(dataState, &position->point);
    err = err ? err : data_gps_init(dataState, &position->gps);
    err = err ? err : data_geographic_position_init(dataState, &position->geographic_position);
    err = err ? err : data_string_init(dataState, &position->variant);

    position->boxValid = FALSE;
    err = err ? err : data_box_init(dataState, &position->boundingBox);

    return err;
}

void
data_position_free(data_util_state* dataState, data_position* position)
{
    data_point_free(dataState, &position->point);
    data_gps_free(dataState, &position->gps);
    data_geographic_position_free(dataState, &position->geographic_position);
    data_string_free(dataState, &position->variant);
    data_box_free(dataState, &position->boundingBox);
}

tpselt
data_position_to_tps(data_util_state* dataState, data_position* position)
{
    tpselt childElement = NULL;
    tpselt newElement = NULL;

    // If the bounding box is valid then we don't add a position element
    if (position->boxValid)
    {
        newElement = data_box_to_tps(dataState, &position->boundingBox);
        if (newElement == NULL)
        {
            goto errexit;
        }
    }
    else
    {
        newElement = te_new("position");
        if (newElement == NULL)
        {
            goto errexit;
        }

        if (data_string_compare_cstr(&position->variant, "point")) 
        {
            if ((childElement = data_point_to_tps(dataState, &position->point)) != NULL && te_attach(newElement, childElement))
            {
                childElement = NULL;
            }
            else
            {
                goto errexit;
            }
        }

        if (data_string_compare_cstr(&position->variant, "gps")) 
        {
            if ((childElement = data_gps_to_tps(dataState, &position->gps)) != NULL && te_attach(newElement, childElement))
            {
                childElement = NULL;
            }
            else
            {
                goto errexit;
            }
        }

        if (data_string_compare_cstr(&position->variant, "geographic-position"))
        {
            if ((childElement = data_geographic_position_to_tps(dataState, &position->geographic_position)) != NULL && te_attach(newElement, childElement))
            {
                childElement = NULL;
            }
            else
            {
                goto errexit;
            }
        }

        // @todo: Other variants

        if (!te_setattrc(newElement, "variant", data_string_get(dataState, &position->variant)))
        {
            goto errexit;
        }
    }

    return newElement;

errexit:

    te_dealloc(newElement);
    return NULL;
}

NB_Error
data_position_from_tps(data_util_state* dataState, data_position* position, tpselt element)
{
    NB_Error err = NE_OK;
    tpselt	childElement = NULL;

    if (element == NULL) 
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_position_free(dataState, position);

    err = data_position_init(dataState, position);

    if (err != NE_OK)
    {
        return err;
    }

    err = data_string_from_tps_attr(dataState, &position->variant, element, "variant");

    if (err != NE_OK)
    {
        return err;
    }

    if (data_string_compare_cstr(&position->variant, "point")) 
    {
        childElement = te_getchild(element, "point");

        if (childElement == NULL) 
        {
            err = NE_INVAL;
            goto errexit;
        }

        err = err ? err : data_point_from_tps(dataState, &position->point, childElement);
    }

    if (data_string_compare_cstr(&position->variant, "gps")) 
    {
        childElement = te_getchild(element, "gps");

        if (childElement == NULL) 
        {
            err = NE_INVAL;
            goto errexit;
        }

        err = err ? err : data_gps_from_tps(dataState, &position->gps, childElement);
    }

    if (data_string_compare_cstr(&position->variant, "geographic-position"))
    {
        childElement = te_getchild(element, "geographic-position");

        if (childElement == NULL)
        {
            err = NE_INVAL;
            goto errexit;
        }

        err = err ? err : data_geographic_position_from_tps(dataState, &position->geographic_position, childElement);
    }

errexit:
    if (err != NE_OK)
    {
        data_position_free(dataState, position);
    }
    return err;
}

boolean
data_position_equal(data_util_state* dataState, data_position* pp1, data_position* pp2)
{
    // @todo: Should compare the values for "boxValid" and "variant" separately?
    return (data_string_equal(dataState, &pp1->variant, &pp2->variant) &&
            data_point_equal(dataState, &pp1->point, &pp2->point) &&
            data_gps_equal(dataState, &pp1->gps, &pp2->gps) &&
            data_geographic_position_equal(dataState, &pp1->geographic_position, &pp2->geographic_position) &&
            (pp1->boxValid == pp2->boxValid) &&
            data_box_equal(dataState, &pp1->boundingBox, &pp2->boundingBox));
}

NB_Error
data_position_copy(data_util_state* dataState, data_position* pp_dest, data_position* pp_src)
{
    // @todo: Should copy the values for "boxValid" and "variant" separately?
    NB_Error err = NE_OK;

    data_position_free(dataState, pp_dest);
    data_position_init(dataState, pp_dest);

    err = err ? err : data_string_copy(dataState, &pp_dest->variant, &pp_src->variant);
    err = err ? err : data_point_copy(dataState, &pp_dest->point, &pp_src->point);
    err = err ? err : data_gps_copy(dataState, &pp_dest->gps, &pp_src->gps);
    err = err ? err : data_geographic_position_copy(dataState, &pp_dest->geographic_position, &pp_src->geographic_position);

    pp_dest->boxValid = pp_src->boxValid;

    err = err ? err : data_box_copy(dataState, &pp_dest->boundingBox, &pp_src->boundingBox);

    return err;
}
