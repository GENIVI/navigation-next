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

import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;

class GestrueHandler implements OnTouchListener, OnGestureListener {

    GestrueHandler(android.content.Context context, MapView mapView){
        mMapView = mapView;
        mMapView.setOnTouchListener(this);
        mMapView.setFocusable(true);
        mMapView.setClickable(true);
        mMapView.setLongClickable(true);

        mGestureDetector = new GestureDetector(context, this);
        mGestureDetector.setIsLongpressEnabled(true);
    }

    @Override
    public boolean onTouch(View view, MotionEvent event) {
//        int i = MotionEvent.
//        return mGestureDetector.onTouchEvent(event);
        int eventaction = event.getAction();

        switch (eventaction) {
        case MotionEvent.ACTION_DOWN:
            mMapView.onTouchEvent(event.getX(), event.getY());
            break;
        case MotionEvent.ACTION_MOVE:
            mMapView.onPaning(event.getX(), event.getY());
            break;
        case MotionEvent.ACTION_UP:
        	break;
        default:
        	break;
        }

        return false;
    }
    
    @Override
    public boolean onDown(MotionEvent event) {
        return false;
    }
    @Override
    public boolean onFling(MotionEvent event, MotionEvent arg1, float arg2, float arg3) {
        return false;
    }
    @Override
    public void onLongPress(MotionEvent event) {
        
    }
    @Override
    public boolean onScroll(MotionEvent event, MotionEvent arg1, float arg2, float arg3) {
        return false;
    }
    @Override
    public void onShowPress(MotionEvent event) {
        
    }
    @Override
    public boolean onSingleTapUp(MotionEvent event) {
        return false;
    }
    
    private MapView mMapView;
    private GestureDetector mGestureDetector;
}
