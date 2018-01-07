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
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
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

import java.util.ArrayList;

class NativeMapView {
    NativeMapView(){
        mHolder = new NativeMapViewHolder(this);
    }
    native void Initialize(Object surface, Object config);
    native void Finalize();
    
    native void Invalidate();
    
    native void SetBackground(boolean background);
    
    native void SetViewCenter(double mercatorX, double mercatorY);
    native void SetViewPointDistance(float distance);
    
    native void SetViewSize(int x, int y, int width, int height);
    native void SetPerspective(float fov, float aspect);
    native void SetSurfaceSize(int width, int height);

    native void SurfaceCreated(Object surface);
    native void SurfaceDestroyed();

    native void OnTouchEvent(float screenX, float screenY);
    native void OnPaning(float screenX, float screenY);
    
    native int LoadCommonMaterial(String materialName, String filePath);
    native int SetCurrentCommonMaterial(String materialName);
    native int LoadNBMTile(String nbmName, int baseDrawOrder, int labelDrawOrder, String filePath);
    native void UnLoadTile(String nbmName);
    
    NativeMapViewHolder getHolder(){
        return mHolder;
    }
    
    static {
        System.loadLibrary("stlport_shared");
        System.loadLibrary("nbpal");
        System.loadLibrary("nbgm");
        System.loadLibrary("nbgmuitest");
    }
    
    private NativeMapViewHolder mHolder;
    private long mNativePtr;
}

class NativeMapViewHolder{
    interface CallBack {
        public abstract void mapViewReady();
    }
    
    NativeMapViewHolder(NativeMapView view){
        mNativeMapView = view;
    }
    
    void addCallBack(CallBack cb){
        mCallBackList.add(cb);
    }
    
    private void OnReady(){
        for(CallBack cb: mCallBackList){
            cb.mapViewReady();
        }
    }
    private ArrayList<CallBack> mCallBackList;
    NativeMapView mNativeMapView;
}
