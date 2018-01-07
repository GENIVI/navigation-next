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

package com.nimone.nbgmandoriduitest;


import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.content.res.Configuration;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Window;
import android.view.WindowManager;

import com.navbuilder.nbgm.MapView;
import com.navbuilder.nbgm.MapView.MapViewHolder;
import com.nimone.nbgmuitest.R;

public class MainActivity extends Activity implements MapViewHolder {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String resoureFolder = NBGMUtility.getSdCardPath() + DATA_ROOT_FOLDER + "nbgmresource/";
        NBGMUtility.CopyNBGMResourceFiles(getAssets(), resoureFolder);
        
        String nbmFolder = NBGMUtility.getSdCardPath() + DATA_ROOT_FOLDER + "NBM/";
        NBGMUtility.CopySampleNBMFiles(getAssets(), nbmFolder);

        com.navbuilder.nbgm.Configuration config = new com.navbuilder.nbgm.Configuration();
        config.preferredLanguageCode = 0;
        config.enableLOD = false;
        config.deviceDPI = 373;
        config.renderSystemType = com.navbuilder.nbgm.Configuration.NBGM_RS_GLES20;
        config.drawAvatar = true;
        config.drawFlag = false;
        config.drawSky = false;
        config.renderInterval = 100;
        config.workSpace = resoureFolder;
        
        mMapView = new MapView(getApplicationContext(), config);
        mMapView.setHolder(this);
        setContentView(mMapView);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.activity_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item){
        if(item.getItemId() == R.id.open_nbm){
            mMapView.setBackground(true);
            Intent intent =new Intent(this, FileBrowser.class); 
            startActivityForResult(intent, 0);
        }
        return false;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 0 && resultCode == RESULT_OK) {
            String folder = data.getStringExtra("folder");
            Log.e("NBGM", "File Browser Get Result: " + folder);
            loadVector();
            mMapView.setBackground(false);
        }
    }

    @Override
    public void viewReady() {
        mMapView.loadCommonMaterial("day_material",
                NBGMUtility.getSdCardPath() + DATA_ROOT_FOLDER + "NBM/MAT/T_DMAT_1_NBM_23");
        mMapView.setCurrentCommonMaterial("day_material");
        loadVector();
    }

    private void loadVector(){
        String[] tileNames = {"DVA/T_DVA_1_NBM_23_5666_13137",
                "DVA/T_DVA_1_NBM_23_5667_13132",
                "DVA/T_DVA_1_NBM_23_5667_13133",
                "DVA/T_DVA_1_NBM_23_5667_13136",
                "DVA/T_DVA_1_NBM_23_5667_13137",
                "DVA/T_DVA_1_NBM_23_5667_13138",
                "DVA/T_DVA_1_NBM_23_5667_13139",
                "DVA/T_DVA_1_NBM_23_5668_13132",
                "DVR/T_DVR_1_NBM_23_5666_13137",
                "DVR/T_DVR_1_NBM_23_5667_13132",
                "DVR/T_DVR_1_NBM_23_5667_13133",
                "DVR/T_DVR_1_NBM_23_5667_13136",
                "DVR/T_DVR_1_NBM_23_5667_13137",
                "DVR/T_DVR_1_NBM_23_5667_13138",
                "DVR/T_DVR_1_NBM_23_5667_13139",
                "DVR/T_DVR_1_NBM_23_5668_13132"};
        loadTiles(tileNames);
        mMapView.setViewCenter(-2.05486319635384, 0.622508595806721);
        mMapView.setViewPointDistance(13600/4);
    }

    private void loadTiles(String[] tileNames){
        for(int i=0; i<tileNames.length; ++i){
            String tileName = tileNames[i];
            mMapView.loadNBMTile(tileName, 2, 50, NBGMUtility.getSdCardPath() + DATA_ROOT_FOLDER + "NBM/" + tileName);
        }
    }

    static final String DATA_ROOT_FOLDER = "/nbgmuitest/";
    private MapView mMapView;
}
