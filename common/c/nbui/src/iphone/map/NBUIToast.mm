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

#import <UIKit/UIKit.h>
#import "NBUIToast.h"

const static CGFloat PADDING = 5.0;
const static CGFloat DEFAULT_SYSTEM_PADDING = 15.0;

@interface NBUIToastBackground : UIView

@property (nonatomic, strong) UIImageView* topleftImage;
@property (nonatomic, strong) UIImageView* topstripImage;
@property (nonatomic, strong) UIImageView* toprightImage;
@property (nonatomic, strong) UIImageView* rightstripImage;
@property (nonatomic, strong) UIImageView* rightbottomImage;
@property (nonatomic, strong) UIImageView* bottomstripImage;
@property (nonatomic, strong) UIImageView* bottomleftImage;
@property (nonatomic, strong) UIImageView* leftstripImage;
@property (nonatomic, strong) UIImageView* centerImage;

@end

@implementation NBUIToastBackground

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        self.topleftImage = [[UIImageView alloc]initWithImage:[UIImage imageNamed:@"Mapkit3D.bundle/Mapkit3DContent/ASSERTS/toast_top_left.png"]];
        self.topleftImage.frame = [self resizeFrameSize:self.topleftImage.frame];
        [self addSubview:self.topleftImage];

        self.topstripImage = [[UIImageView alloc]initWithImage:[UIImage imageNamed:@"Mapkit3D.bundle/Mapkit3DContent/ASSERTS/toast_top_strip.png"]];
        self.topstripImage.frame = [self resizeFrameSize:self.topstripImage.frame];
        [self addSubview:self.topstripImage];

        self.toprightImage = [[UIImageView alloc]initWithImage:[UIImage imageNamed:@"Mapkit3D.bundle/Mapkit3DContent/ASSERTS/toast_top_right.png"]];
        self.toprightImage.frame = [self resizeFrameSize:self.toprightImage.frame];
        [self addSubview:self.toprightImage];

        self.rightstripImage = [[UIImageView alloc]initWithImage:[UIImage imageNamed:@"Mapkit3D.bundle/Mapkit3DContent/ASSERTS/toast_right_strip.png"]];
        self.rightstripImage.frame = [self resizeFrameSize:self.rightstripImage.frame];
        [self addSubview:self.rightstripImage];

        self.rightbottomImage = [[UIImageView alloc]initWithImage:[UIImage imageNamed:@"Mapkit3D.bundle/Mapkit3DContent/ASSERTS/toast_bottom_right.png"]];
        self.rightbottomImage.frame = [self resizeFrameSize:self.rightbottomImage.frame];
        [self addSubview:self.rightbottomImage];

        self.bottomstripImage = [[UIImageView alloc]initWithImage:[UIImage imageNamed:@"Mapkit3D.bundle/Mapkit3DContent/ASSERTS/toast_bottom_Strip.png"]];
        self.bottomstripImage.frame = [self resizeFrameSize:self.bottomstripImage.frame];
        [self addSubview:self.bottomstripImage];

        self.bottomleftImage = [[UIImageView alloc]initWithImage:[UIImage imageNamed:@"Mapkit3D.bundle/Mapkit3DContent/ASSERTS/toast_bottom_left.png"]];
        self.bottomleftImage.frame = [self resizeFrameSize:self.bottomleftImage.frame];
        [self addSubview:self.bottomleftImage];

        self.leftstripImage = [[UIImageView alloc]initWithImage:[UIImage imageNamed:@"Mapkit3D.bundle/Mapkit3DContent/ASSERTS/toast_left_strip.png"]];
        self.leftstripImage.frame = [self resizeFrameSize:self.leftstripImage.frame];
        [self addSubview:self.leftstripImage];

        self.centerImage = [[UIImageView alloc]initWithImage:[UIImage imageNamed:@"Mapkit3D.bundle/Mapkit3DContent/ASSERTS/toast_center.png"]];
        self.centerImage.frame = [self resizeFrameSize:self.centerImage.frame];
        [self addSubview:self.centerImage];
    }
    return self;
}

- (CGRect)resizeFrameSize:(CGRect)rect
{
    CGFloat factor = [UIScreen mainScreen].scale;

    return CGRectMake(rect.origin.x, rect.origin.y, rect.size.width / factor, rect.size.height / factor);
}

- (void)layoutSubviews
{
    [super layoutSubviews];

    CGFloat x = 0;
    CGFloat y = 0;

    CGFloat width = self.topleftImage.frame.size.width;
    CGFloat height = self.topleftImage.frame.size.height;
    self.topleftImage.frame = CGRectMake(x, y, width, height);

    x += width;
    width = self.bounds.size.width - x - self.toprightImage.frame.size.width;
    self.topstripImage.frame = CGRectMake(x, y, width, height);

    x += width;
    width = self.toprightImage.frame.size.width;
    self.toprightImage.frame = CGRectMake(x, y, width, height);

    y += height;
    height = self.bounds.size.height - y - self.rightbottomImage.frame.size.height;
    self.rightstripImage.frame = CGRectMake(x, y, width, height);

    y += height;
    height = self.rightbottomImage.frame.size.height;
    self.rightbottomImage.frame = CGRectMake(x, y, width, height);

    x = 0;
    y = CGRectGetMaxY(self.topleftImage.frame);
    width = self.leftstripImage.frame.size.width;
    height = self.bounds.size.height - y - self.bottomleftImage.frame.size.height;
    self.leftstripImage.frame = CGRectMake(x, y, width, height);

    y += height;
    height = self.bottomleftImage.frame.size.height;
    self.bottomleftImage.frame = CGRectMake(x, y, width, height);

    x += width;
    width = self.bounds.size.width - x - self.rightbottomImage.frame.size.width;
    self.bottomstripImage.frame = CGRectMake(x, y, width, height);

    x = CGRectGetMaxX(self.topleftImage.frame);
    y = CGRectGetMaxY(self.topleftImage.frame);
    height = CGRectGetMinY(self.bottomstripImage.frame) - CGRectGetMaxY(self.topstripImage.frame);
    self.centerImage.frame = CGRectMake(x, y, width, height);
}

@end


@interface NBUIToastContent : UIView

- (id)initWithFrame:(CGRect)frame;
- (CGSize)getContentSize;

@property (nonatomic, copy) NSString* text;
@property (nonatomic, strong) UILabel* textLabel;
@property (nonatomic, strong) NBUIToastBackground* backgroundView;

@end

@implementation NBUIToastContent

- (id)initWithFrame:(CGRect)frame;
{
    self = [super initWithFrame:CGRectZero];
    if (self)
    {
        //self.text = showText;

        UIFont *font = [UIFont boldSystemFontOfSize:14];
        self.textLabel = [[UILabel alloc] initWithFrame:CGRectZero];
        self.textLabel.backgroundColor = [UIColor clearColor];
        self.textLabel.textColor = [UIColor whiteColor];
        self.textLabel.textAlignment = NSTextAlignmentCenter;
        self.textLabel.font = font;
        self.textLabel.numberOfLines = 0;

        self.backgroundView = [[NBUIToastBackground alloc]initWithFrame:CGRectZero];
        [self addSubview:self.backgroundView];
        [self addSubview:self.textLabel];

        self.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    }
    return self;
}

- (void)setText:(NSString *)text
{
    _text = text;

    NSMutableParagraphStyle* style = [[NSMutableParagraphStyle alloc] init];
    [style setAlignment:NSTextAlignmentCenter];
    [style setLineBreakMode:NSLineBreakByWordWrapping];
    NSDictionary* dic = @{NSForegroundColorAttributeName: [UIColor whiteColor],
                          NSFontAttributeName: self.textLabel.font,
                          NSParagraphStyleAttributeName:style};

    NSMutableAttributedString* attrStr = [[NSMutableAttributedString alloc] initWithString:self.text];
    NSRange range = NSMakeRange(0, attrStr.length);
    [attrStr addAttributes:dic range:range];
    self.textLabel.attributedText = attrStr;
}

- (CGSize)getContentSize
{
    CGSize size = [self getTextLabelSize];
    return CGSizeMake(size.width + PADDING * 4, size.height + PADDING * 2);
}

- (CGSize)getTextLabelSize
{
    NSRange range;
    NSDictionary* dic = [self.textLabel.attributedText attributesAtIndex:0
                                                          effectiveRange:&range];

    CGFloat maxWidth = [[UIScreen mainScreen] bounds].size.width - DEFAULT_SYSTEM_PADDING * 2;

    CGSize textSize = [self.text boundingRectWithSize:CGSizeMake(maxWidth, 0)
                                              options: NSStringDrawingTruncatesLastVisibleLine | NSStringDrawingUsesLineFragmentOrigin| NSStringDrawingUsesFontLeading
                                           attributes:dic
                                              context:nil].size;
    return textSize;
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    self.textLabel.frame = CGRectInset(self.bounds, PADDING * 2, PADDING);
    self.backgroundView.frame = self.bounds;
}

@end


@interface NBUIToast()

@property (nonatomic, strong) NBUIToastContent* contentView;

@end


@implementation NBUIToast

- (id)initWithHostView:(UIView*)hostView
{
    self = [super init];
    if (self)
    {
        self.contentView = [[NBUIToastContent alloc]initWithFrame:CGRectZero];

        [hostView addSubview:self.contentView];
    }

    return self;
}

- (void)show
{
    self.contentView.hidden = NO;
}

- (void)hide
{
    self.contentView.hidden = YES;
}

- (void)updateText:(NSString*)text
{
    self.contentView.text = text;
    [self contentViewLayout];
}

- (void)dismiss
{
    [self.contentView removeFromSuperview];
}

- (void)setCenter:(CGPoint)center
{
    _center = center;
    [self contentViewLayout];
}

- (void)contentViewLayout
{
    CGSize size = [self.contentView getContentSize];

    CGFloat contentwidth = size.width;
    CGFloat contentheight = size.height;

    self.contentView.frame = CGRectMake(self.center.x - contentwidth/2, self.center.y - contentheight/2, contentwidth, contentheight);
}

@end
