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

@file data_box.c

*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#include "data_box.h"

NB_Error
data_box_init(data_util_state* dataState, data_box* box)
{
    NB_Error err = NE_OK;

    err = err ? err : data_point_init(dataState, &box->topLeft);
    err = err ? err : data_point_init(dataState, &box->bottomRight);

    return err;
}

void
data_box_free(data_util_state* dataState, data_box* box)
{
    data_point_free(dataState, &box->topLeft);
    data_point_free(dataState, &box->bottomRight);
}

tpselt
data_box_to_tps(data_util_state* dataState, data_box* box)
{
    tpselt child = 0;
    tpselt newElement = te_new("box");
    if (!newElement)
    {
        goto errexit;
    }

    child = data_point_to_tps(dataState, &box->topLeft);
    if (!child || !te_attach(newElement, child))
    {
        goto errexit;
    }

    child = data_point_to_tps(dataState, &box->bottomRight);
    if (!child || !te_attach(newElement, child))
    {
        goto errexit;
    }

    return newElement;

errexit:
    te_dealloc(newElement);
    return NULL;
}

NB_Error
data_box_from_tps(data_util_state* dataState, data_box* box, tpselt element)
{
    NB_Error err = NE_OK;
    tpselt child = 0;
    int iter = 0;
    double temp = 0.0;

    if (!element)
    {
        return NE_INVAL;
    }

    data_box_free(dataState, box);

    err = data_box_init(dataState, box);
    if (err)
    {
        return err;
    }

    child = te_nextchild(element, &iter);
    if (child)
    {
        err = data_point_from_tps(dataState, &box->topLeft, child);
    }

    if (!err)
    {
        child = te_nextchild(element, &iter);
        if (child)
        {
            err = data_point_from_tps(dataState, &box->bottomRight, child);
        }
    }

    // make sure the two points are topleft and bottomright.
    if ( box->topLeft.lat < box->bottomRight.lat )
    {
        temp = box->topLeft.lat;
        box->topLeft.lat = box->bottomRight.lat;
        box->bottomRight.lat = temp;
    }

    if ( box->topLeft.lon > box->bottomRight.lon )
    {
        temp = box->topLeft.lon;
        box->topLeft.lon = box->bottomRight.lon;
        box->bottomRight.lon = temp;
    }

    if (err)
    {
        data_box_free(dataState, box);
    }

    return err;
}

boolean
data_box_equal(data_util_state* dataState, data_box* box1, data_box* box2)
{
    if (!box1 && !box2)
        return TRUE;
    if ((!box1 && box2) || (box1 && !box2))
        return FALSE;
    return (data_point_equal(dataState, &box1->topLeft, &box2->topLeft) &&
            data_point_equal(dataState, &box1->bottomRight, &box2->bottomRight));
}

NB_Error
data_box_copy(data_util_state* dataState, data_box* dest, data_box* src)
{
    NB_Error err = NE_OK;

    data_box_free(dataState, dest);
    data_box_init(dataState, dest);

    err = err ? err : data_point_copy(dataState, &dest->topLeft, &src->topLeft);
    err = err ? err : data_point_copy(dataState, &dest->bottomRight, &src->bottomRight);

    return err;
}

uint32
data_box_get_tps_size(data_util_state* dataState, data_box* box)
{
    if ( box == NULL )
        return 0;
    else
        return sizeof(data_box);
}

void
data_box_to_buf(data_util_state* dataState, data_box* box, struct dynbuf *dbp)
{
    if ( (box == NULL) || (dbp == NULL) )
        return;

    data_point_to_buf(dataState, &box->topLeft, dbp);
    data_point_to_buf(dataState, &box->bottomRight, dbp);
}

NB_Error
data_box_from_binary(data_util_state* dataState, data_box* box, byte** ppdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_point_from_binary(dataState, &box->topLeft, ppdata, pdatalen);
    err = err ? err : data_point_from_binary(dataState, &box->bottomRight, ppdata, pdatalen);

    return err;
}
