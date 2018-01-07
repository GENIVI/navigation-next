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
 
 @file MapPoiBubble.m
 
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

#import "MapPoiBubble.h"

#define BUBBLE_ALPHA_VALUE 0.00                                 /*!< The alpha value of the bubble. */
#define BUBBLE_CENTER_HEIGHT 24.0                               /*!< The height of the bubble center. */
#define BUBBLE_TITLE_FONT_SIZE 18                               /*!< The default title font size of the bubble. */
#define BUBBLE_SUBTITLE_FONT_SIZE 14                            /*!< The default subtitle font size of the bubble. */
// Margin is the blank width outside the bubble.
#define BUBBLE_LEFT_MARGIN 32.0                                 /*!< The width of the left margin of the bubble. */
#define BUBBLE_RIGHT_MARGIN 10.0                                /*!< The width of the right margin of the bubble. */
#define BUBBLE_TOP_MARGIN 10.0                                  /*!< The height of the top margin of the bubble. */
#define BUBBLE_BOTTOM_MARGIN 10.0                               /*!< The height of the bottom margin of the bubble. */
// Padding is the blank width inside the bubble.
#define BUBBLE_LEFT_PADDING 12.0                                /*!< The width of the left padding of the bubble. */
#define BUBBLE_RIGHT_PADDING 12.0                               /*!< The width of the right padding of the bubble. */
// Text padding is blank width before button and text inside the bubble.
#define BUBBLE_TEXT_PADDING 2.0                                 /*!< The width of the text padding of the bubble. */
#define BUBBLE_BUTTON_WIDTH 30.0
#define BUBBLE_BUTTON_HEIGHT 30.0

@interface MapPoiBubble (private)

- (void) createBubbleView;
- (void)relayoutWithFrame;//:(CGRect)newFrame;

@end

@implementation MapPoiBubble

@synthesize m_delegate;
@synthesize m_bubbleView;
@synthesize placemarkPosition = m_placemarkPosition;
@synthesize orientation;
@synthesize m_pointOffset;
@synthesize m_lastTitleText;
@synthesize m_lastSubTitleText;
@synthesize m_bubbleLeft;
@synthesize m_bubbleLeftStrip;
@synthesize m_bubblePoint;
@synthesize m_bubbleRightStrip;
@synthesize m_bubbleRight;
@synthesize m_leftButton;
@synthesize m_rightButton;
@synthesize m_defaultActionButton;
@synthesize m_title;
@synthesize m_subtitle;
@synthesize m_minWidth;
@synthesize m_pointLeftMargin;
@synthesize m_pointRightMargin;
@synthesize m_leftBubbleEnabled;
@synthesize m_rightBubbleEnabled;
@synthesize m_pinId;

- (id)initWithDelegate:(id<MapPoiBubbleDelegate>)delegate
{
	if(self = [super initWithFrame:CGRectZero])
	{
		self.m_delegate = delegate;
		//self.m_bubbleView = self;
		m_placemarkPosition = CGPointZero;
		self.m_pointOffset = CGPointZero;
        
        // Create and hide the UI bubble view when initialization.
        // The UI bubble view should be added as a subview of the scroll view.
        // Set the properties and set the hidden NO when displaying the bubble.
        [self createBubbleView];
        self.hidden = NO;
        
        m_lastMapSize = CGSizeZero;
        m_lastPlacemarkPosition = CGPointZero;
        m_lastPointOffset = CGPointZero;
        self.m_lastTitleText = nil;
        self.m_lastSubTitleText = nil;
	}
	return self;
}

/*! This function is used to make the bubble to only respond buttons. If user
    taps on the other other areas of the bubble, it is the same behavior as
    tapping on the map or other bubbles.
*/
- (UIView*)hitTest:(CGPoint)point withEvent:(UIEvent*)event
{
    if(self.hidden)
    {
        return nil;
    }

    if(!m_leftButton.hidden)
    {
        CGRect rect = m_leftButton.frame;
        if(event && event.type == UIEventTypeTouches && event.type == UIEventSubtypeNone &&
           CGRectContainsPoint(rect, point) == true)
        {
            return m_leftButton;
        }
    }
    if(!m_rightButton.hidden)
    {
        CGRect rect = m_rightButton.frame;
        if(event && event.type == UIEventTypeTouches && event.type == UIEventSubtypeNone &&
           CGRectContainsPoint(rect, point) == true)
        {
            return m_rightButton;
        }
    }

    // @todo: There is no default action button for map3d.
//    if(!m_defaultActionButton.hidden)
//    {
//        CGRect rect = m_defaultActionButton.frame;
//        if(event && event.type == UIEventTypeTouches && event.type == UIEventSubtypeNone &&
//           CGRectContainsPoint(rect, point) == true)
//        {
//            return m_defaultActionButton;
//        }
//    }
    return nil;
}

- (void)createBubbleView
{
    //m_bubbleView = [[UIView alloc] initWithFrame:CGRectZero];
    if(self)
	{
        // Initialize the bubble images.
        UIImage* leftImage = [UIImage imageNamed:@"bubbleLeft.png"];
        UIImage* rightImage = [UIImage imageNamed:@"bubbleRight.png"];
        UIImage* pointImage = [UIImage imageNamed:@"bubblePoint.png"];
        UIImage* stripImage = [UIImage imageNamed:@"bubbleStrip.png"];
        
        // Initialize the bubble image views.
        UIImageView* imageView = NULL;
        
        imageView = [[UIImageView alloc] initWithImage:leftImage];
        imageView.frame = CGRectMake(imageView.frame.origin.x, imageView.frame.origin.y, 
                                     imageView.frame.size.width / 2, imageView.frame.size.height / 2);
        CGFloat leftWidth = imageView.bounds.size.width;
        self.m_bubbleLeft = imageView;
        [imageView release];
        
        imageView = [[UIImageView alloc] initWithImage:stripImage];
        imageView.frame = CGRectMake(imageView.frame.origin.x, imageView.frame.origin.y, 
                                     imageView.frame.size.width / 2, imageView.frame.size.height / 2);
        CGFloat stripWidth = imageView.bounds.size.width;
        self.m_bubbleLeftStrip = imageView;
        [imageView release];
        
        imageView = [[UIImageView alloc] initWithImage:pointImage];
        imageView.frame = CGRectMake(imageView.frame.origin.x, imageView.frame.origin.y, 
                                     imageView.frame.size.width / 2, imageView.frame.size.height / 2);
        CGSize pointSize = imageView.bounds.size;
        self.m_bubblePoint = imageView;
        [imageView release];
        
        imageView = [[UIImageView alloc] initWithImage:stripImage];
        imageView.frame = CGRectMake(imageView.frame.origin.x, imageView.frame.origin.y, 
                                     imageView.frame.size.width / 2, imageView.frame.size.height / 2);
        self.m_bubbleRightStrip = imageView;
        [imageView release];
        
        imageView = [[UIImageView alloc] initWithImage:rightImage];
        imageView.frame = CGRectMake(imageView.frame.origin.x, imageView.frame.origin.y, 
                                     imageView.frame.size.width / 2, imageView.frame.size.height / 2);
        CGFloat rightWidth = imageView.bounds.size.width;
        self.m_bubbleRight = imageView;
        [imageView release];
        
        // Set the point offset.
        self.m_pointOffset = CGPointMake(0.0, pointSize.height);
        
        // Set the minimum width of the UI bubble view.
        self.m_minWidth = leftWidth + rightWidth + pointSize.width + stripWidth * 2.0;
        
        // Set the left margin of the point.
        self.m_pointLeftMargin = leftWidth + stripWidth + pointSize.width / 2.0;
        
        // Set the left margin of the point.
        self.m_pointRightMargin = rightWidth + stripWidth + pointSize.width / 2.0;
        
        [self addSubview:m_bubbleLeft];
        [self addSubview:m_bubbleLeftStrip];
        [self addSubview:m_bubblePoint];
        [self addSubview:m_bubbleRightStrip];
        [self addSubview:m_bubbleRight];
        // @todo: I think don't set the bubble height could let the bubble
        // don't response the touch and tranform to the super view.
        // @todo: Check using the height of point if right.
        //self.bounds = CGRectMake(0.0, 0.0, m_minWidth, pointSize.height);
        self.frame = CGRectMake(0, 0, m_minWidth, pointSize.height);
        // Initialize the left and the right button.
        UIImage* image = nil;
        CGFloat imageWidth = BUBBLE_BUTTON_WIDTH;
        CGFloat imageHeight = BUBBLE_BUTTON_HEIGHT;
        
        image = [UIImage imageNamed:@"go_btn.png"];
        self.m_leftButton = [UIButton buttonWithType:UIButtonTypeCustom];
        m_leftButton.frame = CGRectMake(BUBBLE_LEFT_PADDING, BUBBLE_CENTER_HEIGHT - imageHeight / 2.0, imageWidth, imageHeight);
        m_leftButton.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
        m_leftButton.contentHorizontalAlignment = UIControlContentHorizontalAlignmentCenter;
        [m_leftButton setBackgroundImage:image forState:UIControlStateNormal];
        [m_leftButton addTarget:self action:@selector(leftButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
        m_leftButton.adjustsImageWhenDisabled = YES;
        m_leftButton.adjustsImageWhenHighlighted = YES;
        m_leftButton.backgroundColor = [UIColor clearColor];
        
        image = [UIImage imageNamed:@"arrow_btn.png"];
        self.m_rightButton = [UIButton buttonWithType:UIButtonTypeCustom];
        m_rightButton.frame = CGRectMake(0.0, BUBBLE_CENTER_HEIGHT - imageHeight / 2.0, imageWidth, imageHeight);
        m_rightButton.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
        m_rightButton.contentHorizontalAlignment = UIControlContentHorizontalAlignmentCenter;
        [m_rightButton setBackgroundImage:image forState:UIControlStateNormal];
        [m_rightButton addTarget:self action:@selector(rightButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
        m_rightButton.adjustsImageWhenDisabled = YES;
        m_rightButton.adjustsImageWhenHighlighted = YES;
        m_rightButton.backgroundColor = [UIColor clearColor];
        
        self.m_defaultActionButton = [UIButton buttonWithType:UIButtonTypeCustom];
        m_defaultActionButton.frame = CGRectZero;
        [m_defaultActionButton addTarget:self action:@selector(defaultActionButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
        m_defaultActionButton.backgroundColor = [UIColor clearColor];
        
        // Initialize the title and the subtitle.
        UILabel* title = [[UILabel alloc] initWithFrame:CGRectZero];
        self.m_title = title;
        [title release];
        m_title.textColor = [UIColor whiteColor];
        m_title.backgroundColor = [UIColor clearColor];
        UIFont* font = [UIFont boldSystemFontOfSize:BUBBLE_TITLE_FONT_SIZE];
        m_title.font = font;
        
        UILabel* subtitle = [[UILabel alloc] initWithFrame:CGRectZero];
        self.m_subtitle = subtitle;
        [subtitle release];
        m_subtitle.textColor = [UIColor whiteColor];
        m_subtitle.backgroundColor = [UIColor clearColor];
        font = [UIFont systemFontOfSize:BUBBLE_SUBTITLE_FONT_SIZE];
        m_subtitle.font = font;
        
        [self addSubview:m_defaultActionButton];
        [self addSubview:m_title];
        [self addSubview:m_subtitle];
        [self addSubview:m_leftButton];
        [self addSubview:m_rightButton];
    }
}

- (CGFloat)setTitle:(NSString*)titleText
           subtitle:(NSString*)subtitleText
  leftButtonEnabled:(BOOL)leftButtonEnabled
 rightButtonEnabled:(BOOL)rightButtonEnabled
       mapViewFrame:(CGRect)frame
{
    m_leftButton.hidden = !leftButtonEnabled;
    m_rightButton.hidden = !rightButtonEnabled;
    m_rightBubbleEnabled = rightButtonEnabled;
    m_leftBubbleEnabled = leftButtonEnabled;

    // @todo: Wait to think about how to handle the behaviors of m_delegate in nbui.
    CGSize mapSize = frame.size;

    CGFloat buttonsWidth = BUBBLE_LEFT_PADDING + BUBBLE_RIGHT_PADDING;
    if(leftButtonEnabled)
    {
        buttonsWidth += m_leftButton.bounds.size.width;
    }
    if(rightButtonEnabled)
    {
        buttonsWidth += m_rightButton.bounds.size.width;
    }    
    CGFloat maxWidth = mapSize.width - BUBBLE_LEFT_MARGIN - BUBBLE_RIGHT_MARGIN;
    CGFloat constrainedWidth = maxWidth - buttonsWidth - (BUBBLE_TEXT_PADDING * 2);
    UIFont* font = nil;
    CGSize titleSize = CGSizeZero;
    CGSize subtitleSize = CGSizeZero;
    if(titleText)
    {
        font = [UIFont boldSystemFontOfSize:BUBBLE_TITLE_FONT_SIZE];
        // @todo: Check if should set the right height of the constrained size.
        titleSize = [titleText sizeWithFont:font
                          constrainedToSize:CGSizeMake(constrainedWidth, self.m_title.bounds.size.height)
                              lineBreakMode:UILineBreakModeTailTruncation];
        titleSize.width += (BUBBLE_TEXT_PADDING * 2);
    }
    if(subtitleText)
    {
        font = [UIFont systemFontOfSize:BUBBLE_SUBTITLE_FONT_SIZE];
        // @todo: Check if should set the right height of the constrained size.
        subtitleSize = [subtitleText sizeWithFont:font
                                constrainedToSize:CGSizeMake(constrainedWidth, self.m_subtitle.bounds.size.height)
                                    lineBreakMode:UILineBreakModeTailTruncation];
        subtitleSize.width += (BUBBLE_TEXT_PADDING * 2);
    }
    
    if(titleSize.width > 0.0 && subtitleSize.width > 0.0)
    {
        m_title.frame = CGRectMake(0.0, BUBBLE_CENTER_HEIGHT - titleSize.height, 0.0, titleSize.height);
        m_title.hidden = NO;
        m_subtitle.frame = CGRectMake(0.0, BUBBLE_CENTER_HEIGHT, 0.0, subtitleSize.height);
        m_subtitle.hidden = NO;
    }
    else if(titleSize.width > 0.0)
    {
        m_title.frame = CGRectMake(0.0, BUBBLE_CENTER_HEIGHT - titleSize.height / 2.0, 0.0, titleSize.height);
        m_title.hidden = NO;
        m_subtitle.hidden = YES;
    }
    else if(subtitleSize.width > 0.0)
    {
        m_title.hidden = YES;
        m_subtitle.frame = CGRectMake(0.0, BUBBLE_CENTER_HEIGHT - subtitleSize.height / 2.0, 0.0, subtitleSize.height);
        m_subtitle.hidden = NO;
    }
    m_title.text = titleText;
    m_subtitle.text = subtitleText;
    
    CGFloat result = MAX(titleSize.width, subtitleSize.width) + buttonsWidth;
    result = MAX(result, m_minWidth);
    result = MIN(result, maxWidth);
    return result;
}

- (void) updateBubble: (CGRect) frame
{
    [self setBubblePosition:m_placemarkPosition
                      title:m_title.text
                   subtitle:m_subtitle.text
          leftButtonEnabled:m_leftBubbleEnabled
         rightButtonEnabled:m_rightBubbleEnabled
               mapViewFrame:frame];
}

- (void)setBubblePosition:(CGPoint)position
                    title:(NSString*)titleText
                 subtitle:(NSString*)subtitleText
        leftButtonEnabled:(BOOL)leftButtonEnabled
       rightButtonEnabled:(BOOL)rightButtonEnabled
             mapViewFrame:(CGRect)frame

{
    // If the new layout parameters are same to the previous one, need do nothing

    // @todo: Wait to think about how to handle the behaviors of m_delegate in nbui.
    CGSize mapSize = frame.size;

    if (!self.hidden &&     // If bubble view is hidden, need re layout again.
        m_lastMapSize.width == mapSize.width &&
        m_lastMapSize.height == mapSize.height &&
        m_lastPlacemarkPosition.x == position.x &&
        m_lastPlacemarkPosition.y == position.y &&
        [self.m_lastTitleText compare:titleText] == NSOrderedSame &&
        [self.m_lastSubTitleText compare:subtitleText] == NSOrderedSame)
    {
        return;
    }
    else
    {
        m_lastMapSize = mapSize;
        m_lastPlacemarkPosition = position;
        self.m_lastTitleText = titleText;
        self.m_lastSubTitleText = subtitleText;
    }
    
    self.hidden = NO;
    position.x = mapSize.width * position.x;
    position.y = mapSize.height * position.y;
    m_placemarkPosition = position;
    
    // Set the frame of the UI bubble view.
    CGRect rect = self.frame;
    CGFloat width = [self setTitle:titleText
                          subtitle:subtitleText
                 leftButtonEnabled:leftButtonEnabled
                rightButtonEnabled:rightButtonEnabled
                      mapViewFrame:frame];
    rect.size.width = width;
    
    self.m_pointOffset = CGPointMake(width / 2, self.m_pointOffset.y);
    
    rect.origin.x = position.x - m_pointOffset.x;
    rect.origin.y = position.y - self.frame.size.height;
    self.frame = rect;
    if (rect.origin.y < -rect.size.height || rect.origin.x <  -width / 2 || rect.origin.y > mapSize.height || rect.origin.x > mapSize.width) 
    {
        self.hidden = YES;
    }
    // Couldn't use self.frame or self.bounds because animations are running.
    [self relayoutWithFrame];
}

- (void)relayoutWithFrame
{
    CGRect rect = CGRectZero;
    
    rect = m_bubbleRight.frame;
    // Couldn't use self.frame or self.bounds because animations are running.
    //    CGFloat width = self.bounds.size.width;
    CGFloat width = self.frame.size.width;
    CGFloat rightWidth = rect.size.width;
    rect.origin.x = width - rightWidth;
    m_bubbleRight.frame = rect;
    
    rect = m_bubblePoint.frame;
    CGFloat pointWidth = rect.size.width;
    CGFloat pointX = self.m_pointOffset.x - pointWidth / 2.0;
    rect.origin.x = pointX;
    m_bubblePoint.frame = rect;
    
    rect = m_bubbleLeftStrip.frame;
    CGFloat leftWidth = m_bubbleLeft.bounds.size.width;
    rect.origin.x = leftWidth;
    rect.size.width = pointX - leftWidth;
    m_bubbleLeftStrip.frame = rect;
    
    rect = m_bubbleRightStrip.frame;
    rect.origin.x = pointX + pointWidth;
    rect.size.width = width  - rightWidth - rect.origin.x;
    m_bubbleRightStrip.frame = rect;
    
    CGFloat leftButtonWidth = BUBBLE_LEFT_PADDING;
    if(!m_leftButton.hidden)
    {
        leftButtonWidth += m_leftButton.bounds.size.width;
    }
    CGFloat rightButtonWidth = BUBBLE_RIGHT_PADDING;
    if(!m_rightButton.hidden)
    {
        rightButtonWidth += m_rightButton.bounds.size.width;
    }
    if(!m_title.hidden)
    {
        rect = m_title.frame;
        rect.origin.x = leftButtonWidth + BUBBLE_TEXT_PADDING;
        rect.size.width = width - leftButtonWidth - rightButtonWidth - BUBBLE_TEXT_PADDING * 2;
        m_title.frame = rect;        
    }
    
    if(!m_subtitle.hidden)
    {
        rect = m_subtitle.frame;
        rect.origin.x = leftButtonWidth + BUBBLE_TEXT_PADDING;
        rect.size.width = width - leftButtonWidth - rightButtonWidth - BUBBLE_TEXT_PADDING * 2;
        m_subtitle.frame = rect;
    }
    
    rect = m_rightButton.frame;
    rect.origin.x = width - rightButtonWidth;
    m_rightButton.frame = rect;
    
    //Default action button only available when left and right button showing, other map view not trigger
    if (!m_leftButton.hidden && !m_rightButton.hidden)
    {
        rect = m_defaultActionButton.frame;
        rect.origin.x = m_leftButton.frame.origin.x + m_leftButton.frame.size.width;
        rect.origin.y = m_leftButton.frame.origin.y;
        rect.size.width = m_rightButton.frame.origin.x - m_leftButton.frame.origin.x - m_leftButton.frame.size.width;
        rect.size.height = m_leftButton.frame.size.height;
        m_defaultActionButton.frame = rect;
    }
    [self setNeedsLayout];
    [self setNeedsDisplay];
}

- (void)leftButtonPressed:(id)sender
{
    [m_delegate handleBubbleLeftButton];
}

- (void)rightButtonPressed:(id)sender
{
    [m_delegate handleBubbleRightButton];
}

- (void)defaultActionButtonPressed:(id)sender
{
    //[m_delegate handleBubbleDefaultActionButton];
}

@end

/*! @} */
