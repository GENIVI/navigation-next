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

 @file     data_exit_number.c
 */
/*
 (C) Copyright 2012 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, Inc is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/

#include "data_exit_number.h"

NB_Error
data_exit_number_init(data_util_state* state,
                      data_exit_number* item
                      )
{
    NB_Error err = NE_OK;
    err = data_string_init(state, &item->number);
    err = ((err != NE_OK) ?  err : data_blob_init(state, &item->pronun));
    if (err!= NE_OK)
    {
        data_exit_number_free(state, item);
    }
    return err;
}

void
data_exit_number_free(data_util_state* state,
                      data_exit_number* items
                      )
{
    data_string_free(state, &items->number);
    data_blob_free(state, &items->pronun);
    return ;
}

NB_Error
data_exit_number_from_tps(data_util_state* state,
                          data_exit_number* exitNumber,
                          tpselt tpsElement
                          )
{
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, exitNumber, data_exit_number);

    if (err != NE_OK)
    {
        return err;
    }

    err = data_string_from_tps_attr(state, &exitNumber->number, tpsElement, "number");
    err = ((err != NE_OK) ?  err : data_blob_from_tps_attr(state, &exitNumber->pronun, tpsElement, "pronun"));

    if (err != NE_OK)
    {
        data_exit_number_free(state, exitNumber);
    }

    return err;
}

boolean
data_exit_number_equal(data_util_state* state,
                       data_exit_number* exitNumber1,
                       data_exit_number* exitNumber2
                       )
{
    boolean result = TRUE;

    DATA_EQUAL(state, result,&exitNumber1->number, &exitNumber2->number, data_string);
    DATA_EQUAL(state, result,&exitNumber1->pronun, &exitNumber2->pronun, data_blob);

    return result;
}

NB_Error
data_exit_number_copy(data_util_state* state,
                      data_exit_number* destinationExitNumber,
                      data_exit_number* sourceExitNumber
                      )
{
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, destinationExitNumber, data_exit_number);

    DATA_COPY(state, err, &destinationExitNumber->number, &sourceExitNumber->number, data_string);
    DATA_COPY(state, err, &destinationExitNumber->pronun, &sourceExitNumber->pronun, data_blob);
    return err;

}

