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

    @file     data_error_msg.c
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

#include "data_error_msg.h"

NB_Error
data_error_msg_init(data_util_state* pds, data_error_msg* pem)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_init(pds, &pem->code);
    err = err ? err : data_string_init(pds, &pem->description);
    pem->codeValue =0;

    return err;
}

void
data_error_msg_free(data_util_state* pds, data_error_msg* pem)
{
    data_string_free(pds, &pem->code);
    data_string_free(pds, &pem->description);
}

NB_Error
data_error_msg_from_tps(data_util_state* pds, data_error_msg* pem, tpselt ce)
{
    NB_Error err = NE_OK;

    data_error_msg_init(pds, pem);

    //we are getting the code as string, per protocol
    //we also store it as uint32 for other existing APis that use code as uint32
    err = err ? err : data_string_from_tps_attr(pds, &pem->code, ce, "code");
    err = err ? err : data_string_from_tps_attr(pds, &pem->description, ce, "description");

    // for now, try to convert the code to an uint32, and if that fails and
    // the attribute has a length of one byte, return the ASCII value of the byte
    {
        #define DIGIT_LIMIT 8
        char *value = 0;
        size_t vlen = 0;
        int i = 0;

        i = te_getattr(ce, "code", &value, &vlen);
        if (!i || vlen > DIGIT_LIMIT || vlen < 1)
        {
            pem->codeValue = 0;
        }
        else
        {
            pem->codeValue = nsl_atoi(value);
            if (!pem->codeValue && vlen == 1 && value[0] != '0')
            {
                pem->codeValue = (uint32)value[0];
            }
        }
    }

    return err;
}

boolean
data_error_msg_equal(data_util_state* pds, data_error_msg* pem1, data_error_msg* pem2)
{
    return (data_string_equal(pds, &pem1->code, &pem2->code) && data_string_equal(pds, &pem1->description, &pem2->description)
                && (boolean)(pem1->codeValue == pem2->codeValue));
}

NB_Error
data_error_msg_copy(data_util_state* pds, data_error_msg* pem_dest, data_error_msg* pem_src)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_copy(pds, &pem_dest->code, &pem_src->code);
    err = err ? err : data_string_copy(pds, &pem_dest->description, &pem_src->description);
    pem_dest->codeValue = pem_src->codeValue;

    return err;
}

/*! @} */
