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
    @file     palfavorites.cpp
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

#include "palfavorites.h"
#include "favorites.h"
#include "palstdlib.h"

struct PAL_Favorites
{
    Favorites* pFavorites;
};

PAL_DEF PAL_Error
PAL_FavoritesCreate(PAL_Instance* pal, PAL_Favorites** favorites )
{
    PAL_Error err = PAL_Ok;
    PAL_Favorites* pFav = NULL;

    if( (pal != NULL) && (favorites != NULL))
    {
       pFav = static_cast<PAL_Favorites*>(nsl_malloc(sizeof(PAL_Favorites)));
       if(pFav != NULL)
       {
          pFav->pFavorites = new Favorites(pal);
          if(pFav->pFavorites != NULL)
          {
              *favorites = pFav;
          }
          else
          {
              err = PAL_ErrNoMem;
              nsl_free(pFav);
          }
       }
       else
       {
           err = PAL_ErrNoMem;
       }
    }

    err = err ? err : pFav->pFavorites->Initialize();

    return err;
}

PAL_DEF PAL_Error
PAL_FavoritesDestroy(PAL_Favorites* favorites)
{
    if(favorites != NULL)
    {
        if(favorites->pFavorites != NULL)
        {
            favorites->pFavorites->FavoritesDestroy();
            delete(favorites->pFavorites);
            favorites->pFavorites = NULL;
        }
        nsl_free(favorites);
    }
    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_FavoritesGet(PAL_Favorites* favorites, char* filter, PAL_MyPlace** myPlacesArray, int* myPlaceCount, PAL_SortType sortType, double latitude, double longitude)
{
    if((favorites != NULL) && (favorites->pFavorites != NULL))
    {
        return favorites->pFavorites->FavoritesGet(filter, myPlacesArray, myPlaceCount, sortType, latitude, longitude);
    }
    return PAL_ErrNoData;
}

PAL_DEF PAL_Error
PAL_FavoritesAdd(PAL_Favorites* favorites, PAL_MyPlace* myPlace)
{
    if((favorites != NULL) && (favorites->pFavorites != NULL))
    {
        return favorites->pFavorites->FavoritesAdd(myPlace);
    }
    return PAL_ErrNoData;
}

PAL_DEF PAL_Error
PAL_FavoritesGetFavoriteByID(PAL_Favorites* favorites, uint32 id, PAL_MyPlace* myPlace)
{
    if((favorites != NULL) && (favorites->pFavorites != NULL))
    {
        return favorites->pFavorites->FavoritesGetFavoriteByID( id, myPlace);
    }
    return PAL_ErrNoData;
}

PAL_DEF nb_boolean
PAL_FavoritesIsFavorite(PAL_Favorites* favorites, uint32 id)
{
    if((favorites != NULL) && (favorites->pFavorites != NULL))
    {
        return favorites->pFavorites->FavoritesIsFavorite(id);
    }
    return FALSE;
}

PAL_DEF PAL_Error
PAL_FavoritesAddFavoriteTag(PAL_Favorites* favorites, uint32 id)
{
    if((favorites != NULL) && (favorites->pFavorites != NULL))
    {
        return favorites->pFavorites->FavoritesAddFavoriteTag(id);
    }
    return PAL_ErrNoData;
}

PAL_DEF PAL_Error
PAL_FavoritesRemoveFavoriteTag(PAL_Favorites* favorites, uint32 id)
{
    if((favorites != NULL) && (favorites->pFavorites != NULL))
    {
        return favorites->pFavorites->FavoritesRemoveFavoriteTag(id);
    }
    return PAL_ErrNoData;
}

PAL_DEF PAL_Error
PAL_FavoritesClear(PAL_Favorites* favorites)
{
    if((favorites != NULL) && (favorites->pFavorites != NULL))
    {
        return favorites->pFavorites->FavoritesClear();
    }
    return PAL_ErrNoData;
}

/*! @} */