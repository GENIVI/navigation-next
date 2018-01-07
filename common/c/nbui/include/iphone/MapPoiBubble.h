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
 
 @file MapPoiBubble.h
 
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


#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "PinBubbleView.h"

@protocol MapPoiBubbleDelegate <NSObject>


/*! Handle the event of the bubble left button.
 
 @return None.
 */
- (void) handleBubbleLeftButton;

/*! Handle the event of the bubble right button.
 
 @return None.
 */
- (void) handleBubbleRightButton;

/*! Get the map size.
 
 The map size is the size of the map widget view.
 
 @return The map size.
 */
- (CGSize) getMapSize;
@end

@interface MapPoiBubble : UIView<PinBubbleView>
{
    id<MapPoiBubbleDelegate> m_delegate;               /*!< The map widget delegate. */
    UIView* m_bubbleView;                     /*!< The bubble view. */
    // Position of the placemark but zoom scale independent.
	// The offset of the placemark equals to m_placemarkPosition * zoomScale.
	// And calculate with the offset to generate m_bubbleView.frame.
    CGPoint m_placemarkPosition;                    /*!< The position of the placemark. */
    CGPoint m_pointOffset;                          /*!< The offset of the bubble point based on the placemark position. */
    
    // Cache some layout parameters to avoid layout again for the same bubble.
    CGSize  m_lastMapSize;
    CGPoint m_lastPlacemarkPosition;
    CGPoint m_lastPointOffset;
    NSString* m_lastTitleText;
    NSString* m_lastSubTitleText;
    UIImageView* m_bubbleLeft;                      /*!< The bubble left image view. */
    UIImageView* m_bubbleLeftStrip;                 /*!< The bubble left strip image view. */
    UIImageView* m_bubblePoint;                     /*!< The bubble point image view. */
    UIImageView* m_bubbleRightStrip;                /*!< The bubble right strip image view. */
    UIImageView* m_bubbleRight;                     /*!< The bubble right image view. */
    UIButton* m_leftButton;                         /*!< The left button. */
    UIButton* m_rightButton;                        /*!< The right button. */
    UIButton* m_defaultActionButton;                /*!< The default action button. */
    UILabel* m_title;                               /*!< The title label. */
    UILabel* m_subtitle;                            /*!< The subtitle label. */
    CGFloat m_minWidth;                             /*!< The minimum width of the UI bubble view. */
    // m_pointOffset.x >= m_pointLeftMargin
    CGFloat m_pointLeftMargin;                      /*!< The minimum left margin of the point. */
    // self.bounds.size.width - m_pointOffset.x >= m_pointRightMargin
    CGFloat m_pointRightMargin;                     /*!< The minimum right margin of the point. */
    BOOL m_leftBubbleEnabled;
    BOOL m_rightBubbleEnabled;
}

@property (nonatomic, assign) id<MapPoiBubbleDelegate> m_delegate;
@property (nonatomic, retain) UIView* m_bubbleView;
//@property (nonatomic, assign) CGPoint m_placemarkPosition;
@property (nonatomic, assign) CGPoint m_pointOffset;
@property (nonatomic, retain) NSString* m_lastTitleText;
@property (nonatomic, retain) NSString* m_lastSubTitleText;
@property (nonatomic, retain) UIImageView* m_bubbleLeft;
@property (nonatomic, retain) UIImageView* m_bubbleLeftStrip;
@property (nonatomic, retain) UIImageView* m_bubblePoint;
@property (nonatomic, retain) UIImageView* m_bubbleRightStrip;
@property (nonatomic, retain) UIImageView* m_bubbleRight;
@property (nonatomic, retain) UIButton* m_leftButton;
@property (nonatomic, retain) UIButton* m_rightButton;
@property (nonatomic, retain) UIButton* m_defaultActionButton;
@property (nonatomic, retain) UILabel* m_title;
@property (nonatomic, retain) UILabel* m_subtitle;
@property (nonatomic, assign) CGFloat m_minWidth;
@property (nonatomic, assign) CGFloat m_pointLeftMargin;
@property (nonatomic, assign) CGFloat m_pointRightMargin;
@property (nonatomic, assign) BOOL m_leftBubbleEnabled;
@property (nonatomic, assign) BOOL m_rightBubbleEnabled;
@property (nonatomic, retain) NSString *m_pinId;

/*! Initialize the UI bubble with delegate.
 
 @return The UI bubble instance.
 */
- (id) initWithDelegate:(id<MapPoiBubbleDelegate>)delegate;

- (CGFloat)setTitle:(NSString*)titleText
           subtitle:(NSString*)subtitleText
  leftButtonEnabled:(BOOL)leftButtonEnabled
 rightButtonEnabled:(BOOL)rightButtonEnabled
       mapViewFrame: (CGRect) frame;

- (void)setBubblePosition:(CGPoint)position
                    title:(NSString*)titleText
                 subtitle:(NSString*)subtitleText
        leftButtonEnabled:(BOOL)leftButtonEnabled
       rightButtonEnabled:(BOOL)rightButtonEnabled
             mapViewFrame:(CGRect) frame;

- (void) updateBubble: (CGRect) frame;
@end

/*! @} */