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
   @file        NKUIMapDataType.h
   @defgroup    nkui
*/
/*
   (C) Copyright 2014 by TeleCommunication Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */

#ifndef _NKUIMAPDATATYPE_H_
#define _NKUIMAPDATATYPE_H_

namespace nkui
{
/*!
 * enum of trafficColor;
 */
enum NKUITrafficColor
{
    TC_Transparent = 0,
    TC_Red,
    TC_Yellow,
    TC_Green
};

/*!
 * Type of Pin;
 */
enum NKUIPinType
{
    PT_None = 0,       /*! none */
    PT_Origin,         /*! Origin */
    PT_Destination     /*! Destination */
};

/*!
 * Type of Navigation;
 */
enum NKUINavigationMode
{
    NNM_Remain = 0,         /*! remain current status */
    NNM_TurnByTurn,         /*! trun by turn */
    NNM_TripOverView,       /*! trip over view */
    NNM_RTS                 /*! RTS */
};

/*! tpye of animation acceleration */
enum NKUIAnimationAccelerationType
{
    NAAT_None = 0,
    NAAT_Linear,
    NAAT_Decleration
};

/*! type of avatar mode */

enum NKUIAvatarMode
{
    NAM_Remain = 0,     /** remain current mode */
    NAM_MapMode,        /** map mode */
    NAM_NavMode         /** nav arrow mode */
};

/*!
 * like CGPoint
 */
typedef struct
{
    float x;
    float y;
}ScreenPoint;

}

#endif /* _NKUIMAPDATATYPE_H_ */

/*! @} */
