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

 @file MapNativeHostView.mm

 See file description in header file.

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

#import "MapNativeHostView.h"
#import <QuartzCore/QuartzCore.h>
#import "NBUI_GLESDrawContext.h"
#import "MapNativeGestureConnector.h"
#import "PinBubbleView.h"
#import "nbgmnativerendercontext.h"
#import "MapNativeDebugView.h"
#import "NBUIToast.h"

#include "NBUIConfig.h"
#include "MapLegendInfo.h"
#include "palclock.h"

static const CGFloat TIP_TO_BOTTOM = 150.0;

@implementation BubbleViewParamers
@synthesize view, x, y, orientation;
@end

@interface MapNativeHostView()

@property (nonatomic, strong) id<UIMapViewProtocol> protocol;
@property (nonatomic, strong) MapNativeGestureConnector* gestureConnector;
@property (nonatomic, assign) BOOL mapLegendCreated;
@property (nonatomic, strong) MapNativeDebugView* debugView;
@property (nonatomic, assign) BOOL toastCreated;
@property (nonatomic, strong) NBUIToast* toast;

@end

@implementation MapNativeHostView

// You must implement this method
+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

//The EAGL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:.
- (id)initWithFrame:(CGRect)frame
            mapView:(void*)mapViewUIInterface
           protocol:(void*)mapViewProtocal
{
    self = [super initWithFrame:frame];
    if (self != nil)
    {
        // Initialization code.
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                        nil];

        self.protocol = (__bridge id<UIMapViewProtocol>)mapViewProtocal;

        mapViewUI = (MapViewUIInterface*)mapViewUIInterface;
    }
    return self;
}

/*! This gets called when the MapView in nbservices gets fully destroyed.
 
    Don't call into MapView after this function gets called. Don't wait until dealloc() gets called.
*/
- (void) finalize
{
    // Remove the gesture recognizer from the host view
    [self.gestureConnector finalize];

    // Don't access MapView after this call anymore
    mapViewUI = NULL;
}

- (void)dealloc
{
    [super dealloc];
}

- (void) update
{
    // This gets called in the render thread (and not the UI thread)

    if (self.hidden || (mapViewUI == NULL))
    {
        return;
    }

    uint32 cTime = PAL_ClockGetTimeMs();
    uint32 interval = cTime - lastFrameTimeStamp;

    //NSLog(@"interral = %u", interval);
    mapViewUI->UI_Render(interval);

    lastFrameTimeStamp = cTime;
}

- (void) setCurrentLocation:(double)lat lon:(double)lon
{
    if (mapViewUI)
    {
        mapViewUI->UI_SetCurrentPosition(lat, lon);
    }
}

- (void) addBubbleView:(NSObject*)parameters
{
    if (parameters)
    {
        BubbleViewParamers *viewParameters = (BubbleViewParamers *)parameters;
        id<PinBubbleView> view = viewParameters.view;
        view.placemarkPosition = CGPointMake(viewParameters.x, viewParameters.y);
        view.orientation = viewParameters.orientation;
        [self addSubview:(UIView*) view];
        [view updateBubble: self.frame];
        [viewParameters release];
    }
}

- (void) hideBubbleView:(void *)view
{
    if (view)
    {
        id<PinBubbleView> bubble = (id<PinBubbleView>)view;
        [(UIView*)bubble removeFromSuperview];
    }
}

- (void) updateBubbleView:(NSObject*)parameters
{
    if (parameters)
    {
        BubbleViewParamers *viewParameters = (BubbleViewParamers *)parameters;
        id<PinBubbleView> view = viewParameters.view;
        view.placemarkPosition = CGPointMake(viewParameters.x, viewParameters.y);
        view.orientation = viewParameters.orientation;
        [view updateBubble: self.frame];
        [viewParameters release];
    }
}

- (void)onMapInitialized
{
    self.gestureConnector = [[MapNativeGestureConnector alloc] init:(__bridge id)mapViewUI];
    [self.protocol setMapHostView:self];
    [self.protocol setNativeGestureConnector:self.gestureConnector];
    [self.protocol onMapInitialized];
}

- (NBGM_NativeRenderContext*) createRenderContext
{
    return new NBGM_NativeRenderContext(self);
}

#pragma mark -
#pragma mark UIMapControllerProtocol

- (void)setMapOrientation:(BOOL)bPortraitMode
{
    if (mapViewUI == NULL)
    {
        return;
    }

    self.toast.center = CGPointMake(self.bounds.size.width * 0.5, self.bounds.size.height - TIP_TO_BOTTOM);

    int width = (int)self.frame.size.width * (int)[UIScreen mainScreen].scale;
    int height = (int)self.frame.size.height * (int)[UIScreen mainScreen].scale;
    mapViewUI->UI_SetViewSize(width, height);
    mapViewUI->UI_SetScreenOrientation(bPortraitMode);
}

- (void)setDebugViewPosition:(CGPoint) point
{
    if (self.debugView)
    {
        CGRect frame = CGRectZero;
        frame.origin = point;
        frame.size = self.debugView.frame.size;
        self.debugView.frame = frame;
    }
}

#pragma mark -
#pragma mark Tip Label

- (void)createTipView
{
    if (self.toastCreated == NO)
    {
        self.toast = [[NBUIToast alloc] initWithHostView:self];

        self.toast.center = CGPointMake(self.bounds.size.width * 0.5, self.bounds.size.height - TIP_TO_BOTTOM);

        self.toastCreated = YES;
    }
}

- (void)removeTipView
{
    if (self.toastCreated)
    {
        [self.toast dismiss];
    }
}

- (void)hideTipView
{
    if (self.toastCreated)
    {
        [self.toast hide];
    }
}

- (void)updateTipView:(NSString*)tipString
{
    if (self.toastCreated == NO)
    {
        [self createTipView];
    }

    [self.toast show];
    [self.toast updateText:tipString];

    [self performSelector:@selector(hideTipView) withObject:nil afterDelay:3.0];
}

#pragma mark -
#pragma mark Doppler Legend Bar

- (void)showMapLegend:(shared_ptr<nbmap::MapLegendInfo>) legendInfo
{
    if(self.mapLegendCreated == NO)
    {
        if(!legendInfo)
        {
            return;
        }

        for(uint32 i = 0; i < legendInfo->GetTotalColorBars(); ++i)
        {
            const shared_ptr<nbmap::ColorBar> colorBar = legendInfo->GetColorBar(i);
            string name = colorBar->first;
            NSString* barName = [NSString stringWithCString: name.c_str() encoding:NSUTF8StringEncoding];
            NSInteger index = [self.protocol addMapLegendBar:barName];

            vector<nb_color>& colors = colorBar->second;
            for(uint32 j = 0; j < colors.size(); ++j)
            {
                nb_color nbColor = colors.at(j);
                float r = NB_COLOR_GET_R(nbColor) / 255.0f;
                float g = NB_COLOR_GET_G(nbColor) / 255.0f;
                float b = NB_COLOR_GET_B(nbColor) / 255.0f;
                float a = 1.0f;

                UIColor* uiColor = [UIColor colorWithRed:r green:g blue:b alpha:a];
                [self.protocol addMapLegendColor:index
                                       withColor:uiColor];
            }
        }
        self.mapLegendCreated = YES;
    }

    [self.protocol showMapLegendView];
}

- (void)hideMapLegend
{
    if(self.mapLegendCreated == YES)
    {
        [self.protocol hideMapLegendView];
    }
}

- (void)updateTimestampView:(unsigned int)timestamp
{
    [self.protocol updateTimestamp:timestamp];
}

- (void)updateButtonState:(BOOL)animationCanPlay
{
    [self.protocol updateButtonState:animationCanPlay];
}

#pragma mark -
#pragma mark Bubble

- (void) createLocationBubble
{
    if (!locationBubble)
    {
        locationBubble = [[MapPoiBubble alloc] initWithDelegate:nil];
        locationBubble.m_title.text = [NSString stringWithUTF8String:"Current Location"];
        locationBubble.m_leftBubbleEnabled = false;
        locationBubble.m_rightBubbleEnabled = false;
        [locationBubble setHidden:NO];
        [self addSubview:locationBubble];
    }
}

- (void) removeLocationBubble
{
    if (locationBubble)
    {
        [locationBubble setHidden:YES];
        [locationBubble removeFromSuperview];
        locationBubble = nil;
    }
}

- (void) updateLocationBubble: (BubbleViewParamers*) params lat:(double)latitude lon:(double)longitude
{
    if (!locationBubble)
    {
        [self createLocationBubble];
        if (!locationBubble)
        {
            return;
        }
    }

    locationBubble.m_subtitle.text = [NSString stringWithFormat:@"Lat: %.3f, Lon:%.3f", latitude, longitude];
    locationBubble.placemarkPosition = CGPointMake(params.x, params.y);
    locationBubble.orientation = params.orientation;
    [locationBubble updateBubble: self.frame];
    [params release];
}

#pragma mark -
#pragma mark Debug View

- (void) createDebugView
{
    self.debugView = [[MapNativeDebugView alloc] initWithFrame:CGRectMake(5.0, 20.0, 160.0, 205.0)];
    [self addSubview:self.debugView];
}

- (void) enableDebugView: (BOOL) enable
{
    if (enable && self.debugView == nil)
    {
        [self createDebugView];
    }
    else if (enable && self.debugView)
    {
        [self.debugView setHidden:NO];
    }
    else if(self.debugView)
    {
        [self.debugView setHidden:YES];
    }
}

- (void) setDebugInfo: (const char*) text
{
    if (self.debugView)
    {
        [self.debugView setDebugInformation:text];
    }
}

@end

/*! @} */
