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

    @file nbgmimapdata.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBGM_IMAP_DATA_H_
#define _NBGM_IMAP_DATA_H_
#include "paltypes.h"
#include "nbgmexp.h"
#include "palerror.h"
#include <string>
#include <vector>
#include <set>

/*! \addtogroup NBGM_Instance
*  @{
*/

/*! Material used by NBGM_IMapData.
*/
class NBGM_IMapMaterial
{
public:
    virtual ~NBGM_IMapMaterial() {};
};

/*! Container of polylines created by NBGM_IMapData. 
    Different layers may have different far/near visibility values.
*/
class NBGM_IPolylineLayer
{
public:
    virtual ~NBGM_IPolylineLayer() {};

    /*! Create a polyline.
     @param id identifier of the polyline, which can be returned in GetElementsAt() function
     @param pts vertices of polyline, unit is Mercator
     @param width width of polyline, unit is Mercator
     @param exteralMateral, external material id. 
                            The id comes from the material array index passing to LoadExternalMaterial().
                            Using a non existing id will cause crash.
     @return polyline layer, which contains polylines
     */
    virtual void AddPolyline(const std::string& id, vector<NBGM_Point2d>& pts, float width, uint16 exteralMateral) = 0;
    
    /*! Create a polyline.
     @param id identifier of the polyline, which can be returned in GetElementsAt() function
     @param pts vertices of polyline, unit is Mercator
     @param width width of polyline, unit is Mercator
     @param material, internal material
     @return polyline layer, which contains polylines
     */
    virtual void AddPolyline(const std::string& id, vector<NBGM_Point2d>& pts, float width, NBGM_IMapMaterial* material) = 0;
};

/*! This interface provides a way to assemble nbm data in memory, without reading file.
*/
class NBGM_IMapData
{
public:
    virtual ~NBGM_IMapData() {};
    /*! Create a polyline layer. 
     @param nearVisibility min visible distance, in meters
     @param farVisibility max visible distance, in meters
     @return polyline layer, which contains polylines. It will be destroyed automatically so don't call delete on it.
     */
    virtual NBGM_IPolylineLayer* AddPolylineLayer(float nearVisibility, float farVisibility) = 0;
};

/*! @} */
#endif
