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

 @file ltkhybridmanager.h
 @date 02/15/2015
 @addtogroup ltkcommon

 */
/*
 * (C) Copyright 2015 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */
/*! @{ */
#ifndef LTKHYBRIDMANAGER_H_
#define LTKHYBRIDMANAGER_H_

#include <QObject>
#include <QString>
#include "ltkcontext.h"

namespace locationtoolkit
{

/**
 * Manager that control hybrid functions.
 */
class LTKHybridManager
{
public:
    /**
     * Constructs a new LTKHybridManager instance.
     *
     * @param ltkContext LTKContext
     */
    static LTKHybridManager* CreateInstance(LTKContext& ltkContext);

    virtual ~LTKHybridManager() {}

public:

    /**
     * Enable/Disable Onboard Mode.
     *
     * @param enable Enable or disable flag.
     * @return Errorcode if failed.
     */
    virtual bool EnableOnboard(bool enable) = 0;

    /**
     * Check whether it is Onboard Mode.
     *
     * @return Return true if it is Onboard Mode.
     */
    virtual bool IsOnboardEnabled() = 0;

    /**
     * Sets the language for onboard mode.
     *
     * @param language Languege string for Onboard Mode.
     * @return Errorcode if failed.
     */
    virtual int SetLanguage(const QString& language) = 0;

    /**
     * Add a on-board map data path.
     *
     * @param path Map data path string.
     * @return Errorcode if failed.
     */
    virtual int AddMapDataPath(const QString& path) = 0;

    /**
     * Set the on-board world map data path.
     *
     * @param path Map data path string.
     * @return Errorcode if failed.
     */
    virtual int SetWorldMapDataPath(const QString& path) = 0;

    /**
     * Tell HybridManager client is asking him to load map data.
     * HybridManager will not initialize NCDB session before loading finished
     */
    virtual int BeginLoadMapData() = 0;

    /**
     * Tell HybridManager client loading map data is finished.
     * HybridManager will initialize NCDB session, and be available for using
     */
    virtual int EndLoadMapData() = 0;
    
    /**
      * Ask HybridManager for map data version 
      */
    virtual const char* MapDataVersion() = 0;
};
}

#endif // LTKHYBRIDMANAGER_H_
/*! @} */
