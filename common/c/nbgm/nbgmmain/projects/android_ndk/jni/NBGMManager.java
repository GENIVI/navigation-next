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

package com.navbuilder.nbgm;

import com.navbuilder.app.atlasbook.AppBuildConfig;
public class NBGMManager{

    public static NBGMManager getInstance(){
        if(mSelf == null){
            mSelf = new NBGMManager();
        }
        return mSelf;
    }
	
    static final byte NBRE_OrthoGraphic = 0;
    static final byte NBRE_Perspective = 1;
    
    static final int NBRE_TLT_Enhanced = 1<<0;
    static final int NBRE_TLT_VectorRoad = 1<<1;
    static final int NBRE_TLT_VectorArea = 1<<2;
    static final int NBRE_TLT_3DLandMark = 1<<3;
    static final int NBRE_TLT_UntexturedBuilding = 1<<4;
    static final int NBRE_TLT_All = 0xFFFFFFFF;
 
    // same as NBRE_DOM_LayerType, see nbgmdom.h
    static final int NBRE_DLT_Associated = 0;
    static final int NBRE_DLT_RoadNetwork = 1;
    static final int NBRE_DLT_AreaPolygon = 2;
    static final int NBRE_DLT_UnTextureBuilding = 3;
    static final int NBRE_DLT_UnTextureLandmarkBuilding = 4;
    static final int NBRE_DLT_LandmarkBuilding = 5;
    
    public native void setNightMode(boolean isNightMode);
    
    public native void vectormapCreate();
    public native void ecmCreate(String textureDir);
    public native void vectormapInit();
    public native void ecmInit();
    public native void vectormapSetFrustum(float near, float far, float yFov, float w, float h);
    public native void vectormapSetOrtho(float near, float far, float top, float bottom, float left, float right, float w, float h);
    public native void ecmSetFrustum(float near, float far, float yFov, float w, float h);
    public native void ecmSetOrtho(float near, float far, float top, float bottom, float left, float right, float w, float h);
    public native void vectormapRender(String[] tiles);
    public native void vectormapUpdateLayout(String[] tiles, float secondsElapsed, boolean needRefresh);
    public native void ecmRender(String[] tiles);
    public native void vectormapSetCamera(double[] eye, double[] look, float[] up);
    public native void ecmSetCamera(float[] eye, float[] look, float[] up);
    public native void vectormapAppendTile(String tileId, String tilePath);
    public native void ecmAppendTile(String fileName);
    public native void vectormapRefreshTile();
    public native void ecmRefreshTile();
    public native void vectormapSetAvatarLocation(double[] location);
    public native void ecmSetAvatarLocation(float[] location);
    public native float[] ecmGetAvatarLocation();
    public native float ecmGetAvatarDistanceFromStart();
    public native void vectormapAddRoutePolyline(float[][] polylineData);
    public native void vectormapRemoveRoutePolyline();
    public native void vectormapRefreshPois(String[] imageName, float[] location, float[] size, int count);
    public native void vectormapSetEndflagLocation(float[] location);
    public native void vectormapSetStartFlagLocation(float[] location);
    public native void ecmSetEndflagLocation(float[] location);
    public native void vectormapSetCurrentManeuver(int maneuverId);
    public native void ecmSetCurrentManeuver(int maneuverId);
    public native void vectormapSetCommonMaterial(String materialName);
    public native void ecmAddManeuverPosition(int maneuverId, float[] position);
    public native void ecmLoadRouteSpline(byte[][] splineData);
    public native void ecmRemoveRouteSpline();
    public native boolean ecmIsAvatarBlocked(String[] tilesID, int count, float distance, float height);
    public native void vectormapSetAvatarScale(float factor);
    public native void ecmSetAvatarScale(float factor);
    public native void ecmSetBackgroundColor(int red, int green, int blue, int alpha);
    public native void vectormapSetSkySize(float skyDistance, float skyHeight, float horizonToScreenTop);
    public native void ecmSetSkySize(float skyDistance, float skyHeight);
    public native void vectormapSetBaseDrawOrder(int[] drawOrder);
    public native void vectormapSetLabelDrawOrder(int[] drawOrder);
    public native void vectormapUpdateExcludeRect(float[] rectArray);
    public native void create(String ecmWorkDir, boolean isNightMode, boolean isCarNav, boolean enableLog, int nbmOffset);
    public native void destroy();
    public native void setIsCarNav(boolean isCarNav);
    public native void setDpi(float dpi);
    public native void ecmSetEnvLight(int red, int green, int blue, int alpha);
    public native void vectormapSetEnvLight(int red, int green, int blue, int alpha);
    public native void vectormapSetShow3DBuilding(boolean bShow);
    private NBGMManager(){
    }
    private static NBGMManager mSelf;
    static {
        System.loadLibrary("NBRE_"+AppBuildConfig.VERSION);
    }   
}
