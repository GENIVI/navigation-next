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

    @file     gpscriteriaset.c
    @defgroup gpsciteria GPS
*/
/*
    (C) Copyright 2008 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#include "gpscriteriaset.h"
#include "palstdlib.h"
#include "csdict.h"

/*! @{ */

#define INITIAL_SIZE 16

struct gps_criteria_set {

    struct CSL_Dictionary* crit;
};

NB_DEF gps_criteria_set_t*
gps_criteria_set_create()
{
    gps_criteria_set_t* gcs = NULL;
    
    gcs = nsl_malloc(sizeof(gps_criteria_set_t));
    
    if (gcs != NULL) {
        
        gcs->crit = CSL_DictionaryAlloc(INITIAL_SIZE);
        
        if (gcs->crit == NULL) {
            
            gps_criteria_set_destroy(gcs);
            gcs = NULL;
        }
    }
    
    return gcs;
}

NB_DEF void
gps_criteria_set_destroy(gps_criteria_set_t* gcs)
{
    if (gcs != NULL) {
        
        if (gcs->crit != NULL)
            CSL_DictionaryDealloc(gcs->crit);
        gcs->crit = NULL;
    
        nsl_free(gcs);
    }
}

NB_DEF NB_Error
gps_criteria_set_add_criteria(gps_criteria_set_t* gcs, const char* name, NB_GpsCriteriaSetting* criteria)
{
    NB_Error err = NE_OK;
    
    if (!CSL_DictionarySet(gcs->crit, name, (const char*) criteria, sizeof(*criteria))) {
    
        err = NE_NOMEM;
    }
    
    return err;
}

NB_DEF NB_GpsCriteriaSetting*
gps_criteria_set_get_criteria(gps_criteria_set_t* gcs, const char* name)
{
    size_t len = 0; 
    
    return (NB_GpsCriteriaSetting*) CSL_DictionaryGet(gcs->crit, name, &len);
}

NB_DEF NB_Error
gps_criteria_set_clear_criteria(gps_criteria_set_t* gcs)
{
    return NE_NOSUPPORT;
}

/*! @} */
