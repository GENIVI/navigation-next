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

@file     Main.h
@date     01/20/2009

Global constants for unit tests

*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.                

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#define UNIT_TEST_VERBOSE_HIGH


#ifdef __BREW__
#define PRINTF DBGPRINTF
#else
#define PRINTF printf
#define SPRINTF sprintf
#endif

#if WINCE || WIN32
#define PATH_DELIMITER "\\"
#else
#define PATH_DELIMITER "/"
#endif


// Constants shared by all/some unit tests .......................................................

#define TEST_MAX_URL_LENGTH                         256
#define TEST_MAX_PATH_LENGTH                        256

#define TEST_SCREEN_WIDTH                           780
#define TEST_SCREEN_HEIGHT                          585

#define INVALID_LATITUDE                            -999.0
#define INVALID_LONGITUDE                           -999.0

#define TEST_COUNTRY                                "USA"
#define TEST_LANGUAGE                               "en-US"
#define TEST_NBGM_VERSION                           "1"

#define TEST_ECM_FILE_FORMAT                        "BIN"
#define TEST_MJO_FILE_FORMAT                        "BIN"
#define TEST_SAR_FILE_FORMAT                        "PNG"
#define TEST_TEXTURE_FILE_FORMAT                    "PNG"
#define TEST_SPEED_LIMITS_SIGN_FILE_FORMAT          "PNG"
#define TEST_COMPRESSED_TEXTURE_FILE_FORMAT         "zip"

// directories for test data
#define CITYMODEL_PATH                              "Citymodels"
#define MJO_PATH                                    "MJO"
#define SAR_PATH                                    "SAR"
#define TEXTURE_PATH                                "Textures"
#define SPEED_LIMITS_IMAGES_PATH                    "SpeedLimitsImages"


/*! Database filename */
#define METADATA_DATABASE_FILENAME                  "metadata.dat"

#ifdef __cplusplus
extern "C"
    {
#endif
int test_main(void);
#ifdef __cplusplus
    }
#endif
/*! @} */
