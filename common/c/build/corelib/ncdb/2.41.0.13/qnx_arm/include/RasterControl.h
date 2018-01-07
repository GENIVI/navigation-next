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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
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

/*!--------------------------------------------------------------------------

@file     RasterControl.h
@date     04/28/2009
@defgroup MOBIUS_MAP  Mobius Map Draw API
@author   John Efseaff

@details The Mobius map draw API consists of these main interfaces.



	#Ncdb::RasterControl

	#Ncdb::RasterShow

	#Ncdb::RasterTile

	#Ncdb::VectorTile


*/
/*
(C) Copyright 2009 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */
#pragma once
#ifndef RASTERCONTROL_H_ONCE
#define RASTERCONTROL_H_ONCE

#include "NcdbTypes.h"
#include "AutoArray.h"
#include "AutoPtr.h"
#include "Overlay.h"
#include "GeoTransform.h"
#include "SketchSurface.h"
#include "Shaper.h"
#include <utility>

namespace Ncdb {

class SketchSurface;
class Session;
class OverlayList;
class Labeler;

//! The map display settings class.
class NCDB_API RasterControl
{
public:
	//! Raster control constructor.
	RasterControl(Session& session);

	//! Raster control destructor.
	~RasterControl(void);

	// Initialize the SketchSurface and the AntigrainShaper
	void Init(int width, int height, ColorMode colormode);

	//! Get the attached session pointer.
	Session& getSession(void);

	//! Get the attached surface pointer.
	SketchSurface& getSurface(void);

	Shaper& getShaper(void);

	//! Get the overlay list.
	void copyOverlayList(OverlayList& overlays) const;

	//! Get the overlay list.
	void pasteOverlayList(const OverlayList& overlays);

    //! Add an overlay to the map display.
	void addOverlay(AutoSharedPtr< Overlay > overlay);

	//! Remove an overlay to the map display.
    void removeOverlay(AutoSharedPtr< Overlay > overlay);

	//! Remove all overlays added to the map display.
    void clearOverlays(void);

	//! Set the display configuration.
	bool setStyleByIndex(int index);

	//! Set the display configuration.
	bool setStyleByName(const char * name);

	//! Set the color display mode.
	void setColorMode(ColorMode Mode);

	//! Get the color display mode.
	ColorMode getColorMode(void);

	//! Set the 2D or 3D display modes.
	void setDisplayMode(DisplayMode Mode);

	//! Returns the 2D or 3D display mode setting.
	DisplayMode getDisplayMode(void);

	//! Set to draw all roads thin.
	void setAllRoadsThin(bool mode);

	//! Returns the thin roads state.
	bool getAllRoadsThin(void) const;

	//! Set drawing mode with anti-aliased or aliased.
	void setAntiAliased(const bool AntiAliased);

	//! @brief Convert a coordinate to a screen pixel.
	void ll2px(float lon, float lat, int& x, int& y) const
	{
		m_DrawnTransform.ll2px(lon, lat, x, y);
	}

	//! @brief Convert a screen pixel to a coordinate.
	void px2ll(float& lon, float& lat, int x, int y) const
	{
		m_DrawnTransform.px2ll(lon, lat, x, y);
	}


	//! Get a new scale from the configuration file.
	bool getNewScale(float& newScale, float currentScale, int delta) const;

	//! Get the default center pos from the configuration file.
	bool getDefaultCenter(WorldPoint& center);

	//! Get the default scale from the configuration file.
	bool getDefaultScale(float& scale);

	//! @brief Set the scale in meters per pixel at the screen center.
	void setCenterScale(float MetersPerPixel)
	{
		m_UpdateTransform.setCenterScale(MetersPerPixel);
	}

	//! @brief Get the center scale in meters per pixel previously set.
	float getCenterScale(void) const
	{
		return m_DrawnTransform.getCenterScale();
	}

	//! @brief Set the scale in meters per pixel at the equator.
	void setEquitorialScale(float MetersPerPixel)
	{
		m_UpdateTransform.setEquitorialScale(MetersPerPixel);
	}

	//! @brief Get the equitorial scale in meters per pixel previously set.
	float getEquitorialScale(void) const
	{
		return m_DrawnTransform.getEquitorialScale();
	}

	//! @brief Update the map projection mode.
	void setProjection(ProjectionMode mode)
	{
		m_UpdateTransform.setProjection(mode);
	}

	//! @brief Return the drawn map projection mode.
	ProjectionMode getProjection(void) const
	{
		return m_DrawnTransform.getProjection();
	}

	//! @brief Set the rotation of the displayed map in degrees.
	//! @param[in] angle Rotation angle in degrees zero north.
	//! @note The rotation is typically set by the vehicle heading.
	void setRotation(float angle)
	{
		m_UpdateTransform.setRotation(angle);
	}

	//! @brief Get the rotation in degrees previously set.
	float getRotation(void) const
	{
		return m_DrawnTransform.getRotation();
	}



	//! @brief Explicitly set the center coordinates of the displayed map.
	//! @note The center may be offset by the Center Pixel and the Center Offset.
	void setCoordCenter(double Longitude, double Latitude)
	{
		m_UpdateTransform.setCoordCenter(Longitude, Latitude);
	}

	//! @brief Return the coordinate center previously set.
	const WorldPoint& getCoordCenter(void) const
	{
		return m_DrawnTransform.getCoordCenter();
	}

	//! @brief Set the center of the map for the next update.
	//! @note On 3D maps you may want to set to 1/3 the screen height and
	//! on 2D maps you may want to set to 1/2 the screen height.  The center
	//! is typically the location of the vehicle icon.
	void setCenterPixel(int x, int y)
	{
		m_UpdateTransform.setCenterPixel(x, y);
	}

    const Ncdb::PixelPoint& getCenterPixel() const
    {
        return m_UpdateTransform.getCenterPixel();
    }

	//! @brief Set the center offset for the next update.
	//! @note The center offset is typically used for panning.  When using
	//! Equidistant map projection its critical to use this offset to move
	//! the map to prevent projection errors.
	void setCenterOffset(int x, int y)
	{
		m_UpdateTransform.setCenterOffset(x, y);
	}

	//! @brief Get the drawn center pixel offset.
	void getCenterOffset(WorldPoint& p) const
	{
		m_DrawnTransform.getCenterOffset(p);
	}

	//! @brief Update the center offset for panning.
	//! @note The center offset is typically used for panning.  When using
	//! Equidistant map projection its critical to use this offset to move
	//! the map to prevent projection errors.
	void addCenterOffset(int x, int y)
	{
		m_UpdateTransform.addCenterOffset(x, y);
	}

	void setLabelMode(LabelMode mode);

	LabelMode getLabelMode(void) const;

    //! Update all overlays
    void updateOverlays();

	//! Get the currently draw transform.
	const GeoTransform* getDrawnTransform(void) const;

	//! Over write the currently draw transform.
	void copyDrawnTransform(GeoTransform& transform) const;

	//! Over write the currently draw transform.
	void pasteDrawnTransform(const GeoTransform& transform);

	//! Get the next drawn transform.
	const GeoTransform* getUpdateTransform(void) const;

	//! Get the next drawn transform.
	void copyUpdateTransform(GeoTransform& transform) const;

	//! Overwrite the next drawn transform before the next update.
	void pasteUpdateTransform(const GeoTransform& transform);

	//! Get the label cache used for drawing maps.
	Labeler* getLabeler(void){ return m_Labeler; }

    //! Signal map data content change (multi-map).
    void mapUpdate ();

	//! Operator new overload for release.
	void* operator new (size_t Size);

	//! Operator new overload for debug.
	void* operator new(size_t Size, const char *file, int line);

	//! Operator new overload.
	void operator delete (void *Void);

	//! Operator new overload for debug.
	void operator delete(void *Void, const char *file, int line);

    //! @brief Get the resolution set in the configuration file.  
    bool getResolution(int& resolution) const;

    //! Set avoid render labels low and high scale range,
    //! Engine need not draw polygon and point labels in this range.
    //! This API should only call by Static label editor tool!
    void setAvoidLabelScales(float lowScale, float highScale)
    {
        m_avoidLabelLowScale  = lowScale;
        m_avoidLabelHighScale = highScale;
    }

    //! Get avoid render labels low and high scale range,
    //! Engine need not draw polygon and point labels in this range.
    void getAvoidLabelScales(float& lowScale, float& highScale)
    {
        lowScale  = m_avoidLabelLowScale;
        highScale = m_avoidLabelHighScale;
    }

private:

	Session& m_Session;

	SharedSurface m_Surface;

	Shaper*  m_Shaper;

	OverlayList m_Overlays;

	DisplayMode m_DisplayMode;

	ColorMode m_ColorMode;

	Labeler* m_Labeler;

	GeoTransform m_UpdateTransform;

	GeoTransform m_DrawnTransform;

	LabelMode m_LabelMode;

    RasterControl& operator=(const RasterControl& other);
	bool m_AllRoadsThin;

    // Avoid draw labels(polygon and pointfeat) low and high scales
    float m_avoidLabelLowScale;
    float m_avoidLabelHighScale;
};


class NCDB_API RasterControlHandle : public AutoSharedPtr<RasterControl>
{
public:
	RasterControlHandle(void)
	: AutoSharedPtr<RasterControl>()
	{
	}
	RasterControlHandle(RasterControl* config)
	: AutoSharedPtr<RasterControl>(config)
	{
	}
};

};
#endif // RASTERCONTROL_H_ONCE
/*! @} */

