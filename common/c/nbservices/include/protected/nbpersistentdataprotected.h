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

    @file     nbpersistentdataprotected.h

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

#ifndef NBPERSISTENTDATAPROTECTED_H
#define NBPERSISTENTDATAPROTECTED_H

#include "pal.h"
#include "paltypes.h"
#include "nberror.h"
#include "nbexp.h"
#include "nbpersistentdata.h"
#include "tpselt.h"

/*!
@addtogroup nbpersistentdata
@{
*/

/*! Get the next persistent element

    This function is used to iterate the persistent data and return the key and data. The key
    is returned as a const string and should not be destroyed. But the data should be destroyed
    with the function 'nsl_free' when no longer needed.

    @return NE_OK if success, NE_NOENT if there are no additional items.
*/
NB_DEC NB_Error
NB_PersistentDataNext(NB_PersistentData* persistentData,    /*!< A persistent data instance */
                      int* iterator,                        /*!< Iteration state, set to 0 to begin */
                      const char** key,                     /*!< Key of the next element */
                      uint8** data,                         /*!< Data of the next element. It should be freed
                                                                 with 'nsl_free' when no longer needed. */
                      nb_size* dataSize                     /*!< Data size of the next element */
                      );


// @todo: Should below two functions rename and move to other files?

/*! Create a persistent data instance by a TPS element

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_PersistentDataCreateByTpsElement(tpselt tpsElement,                  /*!< A TPS element */
                                    NB_PersistentData** persistentData  /*!< On success, it returns a created
                                                                             persistent data instance */
                                    );

/*! Get the data of the persistent data instance to a TPS element

    This function gets the persistent data to a new created TPS element and returns this
    TPS element. This new TPS element should be destroyed with the function 'te_dealloc'
    when no longer needed.

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_PersistentDataGetToTpsElement(NB_PersistentData* persistentData,     /*!< A persistent data instance */
                                 tpselt* tpsElement                     /*!< A created TPS element. It should be
                                                                             destroyed with the function 'te_dealloc'
                                                                             when no longer needed. */
                                         );

/*! @} */

#endif // NBPERSISTENTDATAPROTECTED_H
