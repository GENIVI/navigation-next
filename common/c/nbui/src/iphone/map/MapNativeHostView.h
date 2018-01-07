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

 @file MapNativeHostView.h

 */
/*
 (C) Copyright 2010 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret
 as defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly prescribing
 the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#import <UIKit/UIKit.h>
#import "UIMapControllerProtocol.h"
#import "PinBubbleView.h"
#import "MapPoiBubble.h"
#include "MapViewUIInterface.h"
#import "UIMapViewProtocol.h"
#include "MapLegendInfo.h"

/*! NBGMView class.

 This class wraps the CAEAGLLayer(CoreAnimation) into a convenient UIView subclass.
 NBGMView contains EAGL surface to render NBGM scene into.
 This class should also contains methods to update NBGM data.
 NBGMView class should contain array of current tiles and methods to manage this array(add/remove tile). And
 we can't move this functionality to NBGMViewController(nbservices part) because UI updates are carried out in separate loop.

 */
@class MapNativeGestureRecognizer;
class NBGM_NativeRenderContext;

@interface BubbleViewParamers : NSObject
@property (nonatomic, retain) id<PinBubbleView> view;
@property (nonatomic, assign) float x;
@property (nonatomic, assign) float y;
@property (nonatomic, assign) bool orientation;

@end

@interface MapNativeHostView : UIView<UIMapControllerProtocol>
{
@private
    uint32                          lastFrameTimeStamp;
    MapViewUIInterface*             mapViewUI;

    // All kinds of useful widgets goes here.
    MapPoiBubble*  locationBubble;  /*!< Location bubble to show current location */
}

- (id)initWithFrame:(CGRect)frame
            mapView:(void*)mapViewUIInterface
           protocol:(void*)mapViewProtocal;

- (void) finalize;
- (void) setCurrentLocation:(double)lat lon:(double)lon;
- (void) update;
- (void) addBubbleView:(NSObject*)parameters;
- (void) hideBubbleView:(void *)view;
- (void) updateBubbleView:(NSObject*)parameters;
- (NBGM_NativeRenderContext*) createRenderContext;
- (void)onMapInitialized;

/*! Update timestamp label used for animation layer */
- (void) updateTimestampView:(unsigned int)timestamp;

/*! Add/remove/hide/update tip label used for animation layer */
- (void) createTipView;
- (void) removeTipView;
- (void) hideTipView;
- (void) updateTipView:(NSString*)tipString;

/*! Add/remove map legend */
- (void)showMapLegend:(shared_ptr<nbmap::MapLegendInfo>) legendInfo;
- (void)hideMapLegend;
- (void)updateButtonState:(BOOL)animationCanPlay;

/*! Add/remove/update location bubble of avatar */
- (void) createLocationBubble;
- (void) removeLocationBubble;
- (void) updateLocationBubble: (BubbleViewParamers*) params lat:(double)latitude lon:(double)longitude;


/*! Add/remove/update debug view */
- (void) createDebugView;
- (void) enableDebugView: (BOOL) enable;
- (void) setDebugInfo: (const char*) text;

/*! Implement UIMapControllerProtocol */
- (void)setMapOrientation:(BOOL)bPortraitMode;
- (void)setDebugViewPosition:(CGPoint) point;

@end

/*! @} */
