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

package com.navbuilder.pal.android.ndk;

import java.util.Iterator;

import android.content.Context;
import android.location.GpsSatellite;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.widget.Toast;

public class GPSLocationManager {
	private static Context mAppContext;
	private int mGPSContext;
	private LocationManager mlocManager;
	private LocationListener mlocListener;
	private HandlerThread m_gpsThread;
	private int m_satellitesCount;
	private GpsStatus.Listener gpsStatusListener;

	public static void init(Context context) {
		mAppContext = context;
	}

	public GPSLocationManager() {
		mlocManager = (LocationManager) mAppContext.getSystemService(Context.LOCATION_SERVICE);
		mlocListener = new GPSLocationListener();
		m_gpsThread = new HandlerThread("GPSLocation Thread");
		gpsStatusListener = new GpsStatus.Listener() {
	        public void onGpsStatusChanged(int event) {
	            if( event == GpsStatus.GPS_EVENT_SATELLITE_STATUS){
	                GpsStatus status = mlocManager.getGpsStatus(null); 
	                Iterator<GpsSatellite> sats = status.getSatellites().iterator();
	                
	                m_satellitesCount = 0;
	                
	                while (sats.hasNext())
	                {
	                    GpsSatellite gpsSatellite=(GpsSatellite)sats.next();

	                    if(gpsSatellite.usedInFix())
	                    {
	                    	m_satellitesCount++;
	                    }
	                }
	            }
	        }
	    };
	}

	public void startLocationUpdates() {
		m_gpsThread.start();
		new Handler(m_gpsThread.getLooper()).post(new Runnable() {
			@Override
			public void run() {
				if (mlocManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
					mlocManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 0, 0, mlocListener);
					mlocManager.addGpsStatusListener(gpsStatusListener);
					Location lastKnown = mlocManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
					
					if(lastKnown != null){
						nativeGPSLocationChanged(lastKnown.getLatitude(),
												 lastKnown.getLongitude(),
												 lastKnown.getAltitude(),
												 lastKnown.getBearing(),
												 lastKnown.getTime(),
												 m_satellitesCount);
					}
				}else{
					Toast.makeText(mAppContext.getApplicationContext(), "The GPS Provider is not ready. Please turn on the GPS.", Toast.LENGTH_LONG).show();
				}
			}
		});
	}

	public void stopLocationUpdates() {
		mlocManager.removeUpdates(mlocListener);
		m_gpsThread.getLooper().quit();
		mlocManager.removeGpsStatusListener(gpsStatusListener);
	}

	public class GPSLocationListener implements LocationListener {
		@Override
		public void onLocationChanged(Location loc) {
			nativeGPSLocationChanged(loc.getLatitude(),
									 loc.getLongitude(),
									 loc.getAltitude(),
									 loc.getBearing(),
									 loc.getTime(),
									 m_satellitesCount);
		}

		@Override
		public void onProviderDisabled(String provider) {
		}

		@Override
		public void onProviderEnabled(String provider) {
		}

		@Override
		public void onStatusChanged(String provider, int status, Bundle extras) {
		}
	}

	private native void nativeGPSLocationChanged(double latitude, double longitude, double altitude, double heading, long time, int satNum);
}