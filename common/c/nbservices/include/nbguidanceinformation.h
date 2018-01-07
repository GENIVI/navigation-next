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

    @file     nbguidanceinformation.h

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

#ifndef GUIDANCEINFORMATION_H
#define GUIDANCEINFORMATION_H

#include "nbexp.h"
#include "nbcontext.h"
#include "navpublictypes.h"

// @todo (BUG 55815) - should this be exposed
#include "csltypes.h" 

/*!
    @addtogroup nbguidanceinformation
    @{
*/

/*! @struct NB_GuidanceInformation
Information about Guidance
*/
typedef struct NB_GuidanceInformation NB_GuidanceInformation;

typedef struct NB_GuidanceInformationConfigureImage
{
    char countryCode[16];
    char* imagesBuffer;
    nb_size imagesBufferSize;
} NB_GuidanceInformationConfigureImage;

/*! @struct NB_GuidanceInformationConfiguration
Configuration for Guidance Information creation
*/
typedef struct NB_GuidanceInformationConfiguration
{
    char* imageStyle;   /*!< Image style name */
    char* stepsStyle;   /*!< Steps style name */
    char* pronunStyle;  /*!< Voice style name */

    char* voiceBuffer;
    nb_size voiceBufferSize;

    char* stepsBuffer;
    nb_size stepsBufferSize;

    char* imagesBuffer;
    nb_size imagesBufferSize;

    char* basicAudioBuffer;
    nb_size basicAudioBufferSize;

    char* datalibBuffer;
    nb_size datalibBufferSize;

    char* voicelibBuffer;
    nb_size voicelibBufferSize;

    char* stepslibBuffer;
    nb_size stepslibBufferSize;
    
    char* imagelibBuffer;
    nb_size imagelibBufferSize;

    char* basicAudiolibBuffer;
    nb_size basicAudiolibBufferSize;

    NB_GuidanceInformationConfigureImage* localeImages;  /*!< Array of locale image configures. */
    nb_size localeImagesSize;
} NB_GuidanceInformationConfiguration;

typedef nb_boolean (*NB_GuidanceDataSourceFunction) (uint32 elem, uint32 maneuverIndex, void* pUser, const char** pdata, nb_size* psize, nb_boolean* pfreedata);

/*! Callback function to format maneuver text

@param font Font to display the text in
@param color Color to display the text in
@param text Text to be displayed
@param newline Non-zero if a newline should follow the text; zero otherwise
@param userData Application data passed along with this function.
*/
typedef void (*NB_GuidanceInformationTextCallback)(NB_Font font, nb_color color, const char* text, nb_boolean newline, void* userData);


/*! Create and initialize a new GuidanceInformation object

@param context NB_Context
@param params Parameters used to create NB_GuidanceInformation object
@param information On success, the newly created information object; NULL otherwise.  A valid object must be destroyed using NB_GuidanceInformationDestroy.
@returns NB_Error
*/
NB_DEC NB_Error
NB_GuidanceInformationCreate(NB_Context* context, NB_GuidanceInformationConfiguration* configuration, NB_GuidanceInformation** information );

/*! Get Guidance information command version

@param information A NB_GuidanceInformation objects
@returns uint32 
*/
NB_DEC uint32 
NB_GuidanceInformationGetCommandVersion(NB_GuidanceInformation* information);


/*! Destroy a previously created GuidanceInformation object

@param information A NB_GuidanceInformation object created with NB_GuidanceInformationCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_GuidanceInformationDestroy(NB_GuidanceInformation* information);

/*! Get a pronun information from basicAudio sexp

@param route NB_GuidanceInformation previously created guidance object
@param key name of base voice or street voice
@param textBuffer plain text buffer
@param textBufferSize size of plain text buffer (return 0 if empty)
@param duration time of playback duration in ms
@returns NB_Error
*/
NB_DEC NB_Error NB_GuidanceInformationGetPronunInformation(NB_GuidanceInformation* guidanceInformation, const char* key,
                                                           char* textBuffer, uint32* textBufferSize,
                                                           double* duration);

/*! @} */

#endif // GUIDANCEINFORMATION_H
