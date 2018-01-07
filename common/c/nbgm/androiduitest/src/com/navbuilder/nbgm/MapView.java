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


import android.content.Context;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class MapView extends SurfaceView implements SurfaceHolder.Callback{
	public interface MapViewHolder {
		public abstract void viewReady();
	}

	public MapView(Context context, Configuration configuration) {
		super(context);
		mConfiguration = configuration;
		new GestrueHandler(context, this);
		mNativeMapView = new NativeMapView();
		mTaskQueue = new TaskQueue();
		getHolder().addCallback(this);
	}

	public void invalidate(){
		mNativeMapView.Invalidate();
	}
	
	public void setBackground(boolean background){
		mNativeMapView.SetBackground(background);
	}
	
	public void setViewCenter(double mercatorX, double mercatorY){
		mNativeMapView.SetViewCenter(mercatorX, mercatorY);
	}
	public void setViewPointDistance(float distance){
		mNativeMapView.SetViewPointDistance(distance);
	}
	
	public void onTouchEvent(float screenX, float screenY){
		mNativeMapView.OnTouchEvent(screenX, screenY);
	}
	public void onPaning(float screenX, float screenY){
		mNativeMapView.OnPaning(screenX, screenY);
	}
	
	public int loadCommonMaterial(String materialName, String filePath){
		return mNativeMapView.LoadCommonMaterial(materialName, filePath);
	}
	public int setCurrentCommonMaterial(String materialName){
		return mNativeMapView.SetCurrentCommonMaterial(materialName);
	}
	public int loadNBMTile(String nbmName, int baseDrawOrder, int labelDrawOrder, String filePath){
		return mNativeMapView.LoadNBMTile(nbmName, baseDrawOrder, labelDrawOrder, filePath);
	}
	public void unLoadTile(String nbmName){
		mNativeMapView.UnLoadTile(nbmName);
	}
	
	public void setHolder(MapViewHolder holder){
		mHolder = holder;
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		mWidth = width;
		mHeight = height;
		mTaskQueue.AddTask(new SurfaceChangedTask(this));
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		mSurface = holder.getSurface();
		mTaskQueue.AddTask(new SurfaceCreatedTask(this));
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		if(mInitilized){
			mNativeMapView.SetBackground(false);
			mNativeMapView.SurfaceDestroyed();
		}
	}
	
	void onSurfaceCreated(){
		if(!mInitilized){
			mNativeMapView.Initialize(mSurface, mConfiguration);
			if(mHolder != null){
				mHolder.viewReady();
			}
			mInitilized = true;
		} else {
			mNativeMapView.SurfaceCreated(mSurface);
			mNativeMapView.SetBackground(false);
		}
	}
	
	void onSurfaceChanged(){
		if(mInitilized){
			mNativeMapView.SetViewSize(0, 0, mWidth, mHeight);
			mNativeMapView.SetPerspective(45, mWidth/(float)mHeight);
			mNativeMapView.SetSurfaceSize(mWidth, mHeight);
		}
		else{
			mTaskQueue.AddTask(new SurfaceChangedTask(this));
		}
	}

	private NativeMapView mNativeMapView;
	private Configuration mConfiguration;
	private MapViewHolder mHolder;
	
	private TaskQueue mTaskQueue;
	
	private Surface mSurface = null;
	private int mWidth;
	private int mHeight;
	private boolean mInitilized = false;
}
