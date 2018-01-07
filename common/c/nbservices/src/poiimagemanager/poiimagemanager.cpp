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

    @file       downloadstream.cpp

    See header file for description.
 */
/*
 (C) Copyright 2011 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of Networks In Motion is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/
/*! @{ */

#include "poiimagemanager.h"
#include "poiimagemanagerimplementation.h"
#include "downloadmanager.h"


using namespace nbsearch;

// Constants .....................................................................................

// Local types ...................................................................................


// Public functions
// ..............................................................................

/*! Check whether the configuration is valid.

  @return true if valid, or false otherwise.
*/
bool
IsConfigurationValid(PoiImageManagerConfigPtr configuration /*!< configuration to be checked. */
                    );

/* Refer to 'nbpoiimagemanager.h' for description */
PoiImageManager::~PoiImageManager()
{
}

/* Refer to 'nbpoiimagemanager.h' for description */
NB_Error
PoiImageManager::CreateInstance(NB_Context* context,
                                PoiImageListenerPtr listener,
                                PoiImageManagerConfigPtr configuration,
                                PoiImageManager** manager)
{
    if (!context || !configuration || !manager || !listener ||
        (!IsConfigurationValid(configuration)))
    {
        return NE_INVAL;
    }

    nbcommon::DownloadManager* downloader =
        (nbcommon::DownloadManager*)NB_ContextGetGenericHttpDownloadManager(context);
    if (!downloader)
    {
        return NE_INVAL;
    }

    PoiImageManagerImplementation* poiImageManager = new PoiImageManagerImplementation();
    if (!poiImageManager)
    {
        return NE_NOMEM;
    }

    NB_Error error = poiImageManager->Initialize(context, downloader, listener, configuration);
    if (error != NE_OK)
    {
        delete poiImageManager;
        poiImageManager = NULL;
        return error;
    }

    *manager = poiImageManager;

    return error;
}

NB_Error
PoiImageManager::DestroyInstance(PoiImageManager* manager)
{
    NB_Error error = NE_OK;
    if (manager)
    {
        delete (PoiImageManagerImplementation*)manager;
    }
    else
    {
        error = NE_INVAL;
    }
    return error;
}

bool IsConfigurationValid(PoiImageManagerConfigPtr configuration)

{
    if ((!configuration) || configuration->m_cachePath.length() == 0)
    {
        return false;
    }
    return true;
}



/*! @} */
