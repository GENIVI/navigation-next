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

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

import android.content.res.AssetManager;
import android.os.Environment;
import android.util.Log;

public class NBGMUtility {
    static String getSdCardPath(){
        File sdDir = null; 
        boolean sdCardExist = Environment.getExternalStorageState().equals(android.os.Environment.MEDIA_MOUNTED); 
        if (sdCardExist)
        { 
            sdDir = Environment.getExternalStorageDirectory();
        }
        return sdDir.toString(); 
    }

    static void CopyNBGMResourceFiles(AssetManager am, String resourceFolder){
        final String[] NBGM_RESOURCE_FILE = {
                "ARROW_AVATAR_MODEL",
                "CAR_AVATAR_MODEL",
                "MAP_AVATAR_MODEL",
                "NBM_PIN_MATERIALS",
                "SQUARE_FLAG_MODEL_2D",
                "SQUARE_FLAG_MODEL_3D",
                "TEXTURE/ARROW_AVATAR_TEX",
                "TEXTURE/CAR_AVATAR_TEX",
                "TEXTURE/FOLLOW_ME_ICON",
                "TEXTURE/LOCATE_ME_ICON",
                "TEXTURE/SKY",
                "TEXTURE/NIGHT_SKY",
                "TEXTURE/SQUARE_FLAG_TEX_2D_END",
                "TEXTURE/SQUARE_FLAG_TEX_2D_START",
                "TEXTURE/SQUARE_FLAG_TEX_3D_END",
                "TEXTURE/SQUARE_FLAG_TEX_3D_START"}; 

        //create NBGM resource folder if not exist
        File folder = new File(resourceFolder);
        if(!folder.exists()){
            folder.mkdirs();
        }
        File texFolder = new File(resourceFolder + "TEXTURE/");
        if(!texFolder.exists()){
            texFolder.mkdirs();
        }
        
        for(String srcFileName :NBGM_RESOURCE_FILE){
            File file = null;
            InputStream fis = null;
            FileOutputStream fos = null;
            try{
                file = new File(resourceFolder, srcFileName);
                if(!file.exists()){
                    file.createNewFile();
                    fis = am.open("nbgmresource/" + srcFileName);
                    fos = new FileOutputStream(file);
                    byte[] data = new byte[4*1024];
                    int readLen = -1; 
                    while((readLen = fis.read(data)) != -1){
                        fos.write(data, 0, readLen);
                    }
                    if(fis != null){
                        fis.close();
                    }
                    if(fos != null){
                        fos.close();
                    }
                    Log.i("NBGM", "Copied NBGM resource " + srcFileName);

                }
            }catch(Exception e){
                if(file != null){
                    file.delete();
                }
                e.printStackTrace();
            }
        }
    }
    
    static void CopySampleNBMFiles(AssetManager am, String destFolder){
        String[] SAMPLE_NBM_FILES = {"DVA/T_DVA_1_NBM_23_5666_13137",
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
                "DVR/T_DVR_1_NBM_23_5668_13132",
                "MAT/T_DMAT_1_NBM_23"};

        //create NBGM resource folder if not exist
        File folder = new File(destFolder);
        if(!folder.exists()){
            folder.mkdirs();
        }

        File dvaFolder = new File(destFolder + "DVA/");
        if(!dvaFolder.exists()){
        	dvaFolder.mkdirs();
        }

        File dvrFolder = new File(destFolder + "DVR/");
        if(!dvrFolder.exists()){
        	dvrFolder.mkdirs();
        }
        
        File matFolder = new File(destFolder + "MAT/");
        if(!matFolder.exists()){
        	matFolder.mkdirs();
        }
        
        for(String srcFileName :SAMPLE_NBM_FILES){
            File file = null;
            InputStream fis = null;
            FileOutputStream fos = null;
            try{
                file = new File(destFolder, srcFileName);
                if(!file.exists()){
                    file.createNewFile();
                    fis = am.open("NBM/" + srcFileName);
                    fos = new FileOutputStream(file);
                    byte[] data = new byte[4*1024];
                    int readLen = -1; 
                    while((readLen = fis.read(data)) != -1){
                        fos.write(data, 0, readLen);
                    }
                    if(fis != null){
                        fis.close();
                    }
                    if(fos != null){
                        fos.close();
                    }
                    Log.i("NBGM", "Copied NBM files: " + srcFileName);

                }
            }catch(Exception e){
                if(file != null){
                    file.delete();
                }
                e.printStackTrace();
            }
        }
    }
}
