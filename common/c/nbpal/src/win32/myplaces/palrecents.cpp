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
    @file     palrecents.cpp
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "palrecents.h"
#include "recents.h"
#include "palstdlib.h"

struct PAL_Recents
{
    Recents* pRecents;
};

PAL_DEF PAL_Error
PAL_RecentsCreate(PAL_Instance* pal, PAL_Recents** recents)
{
    PAL_Error err = PAL_Ok;
    PAL_Recents* pRec = NULL;

    if ((pal != NULL) && (recents != NULL))
    {
       pRec = static_cast<PAL_Recents*>(nsl_malloc(sizeof(PAL_Recents)));
       if (pRec != NULL)
       {
          pRec->pRecents = new Recents(pal);
          if (pRec->pRecents != NULL)
          {
              *recents = pRec;
          }
          else
          {
              err = PAL_ErrNoMem;
              nsl_free(pRec);
          }
       }
       else
       {
           err = PAL_ErrNoMem;
       }
    }

    err = err ? err : pRec->pRecents->Initialize();

    return err;
}

PAL_DEF PAL_Error
PAL_RecentsDestroy(PAL_Recents* recents)
{
    if (recents != NULL)
    {
        if (recents->pRecents != NULL)
        {
            recents->pRecents->RecentsDestroy();
            delete(recents->pRecents);
            recents->pRecents = NULL;
        }
        nsl_free(recents);
        recents = NULL;
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_RecentsGet(PAL_Recents* recents, char* filter, PAL_MyPlace** myPlacesArray, int* myPlaceCount, PAL_SortType sortType, double latitude, double longitude)
{
    if ((recents != NULL) && (recents->pRecents != NULL))
    {
        return recents->pRecents->RecentsGet(filter, myPlacesArray, myPlaceCount, sortType, latitude, longitude);
    }

    return PAL_ErrNoData;
}

PAL_DEF PAL_Error
PAL_RecentsAdd(PAL_Recents* recents, PAL_MyPlace* myPlace)
{
    if ((recents != NULL) && (recents->pRecents != NULL))
    {
        return recents->pRecents->RecentsAdd(myPlace);
    }

    return PAL_ErrNoData;
}

PAL_DEF PAL_Error
PAL_RecentsGetPlaceById(PAL_Recents* recents, uint32 id, PAL_MyPlace* myPlace)
{
    if ((recents != NULL) && (recents->pRecents != NULL))
    {
        return recents->pRecents->RecentsGetPlaceById(id, myPlace);
    }

    return PAL_ErrNoData;
}

PAL_DEF PAL_Error
PAL_RecentsClear(PAL_Recents* recents)
{
    if ((recents != NULL) && (recents->pRecents != NULL))
    {
        return recents->pRecents->RecentsClear();
    }

    return PAL_ErrNoData;
}

/*! @} */