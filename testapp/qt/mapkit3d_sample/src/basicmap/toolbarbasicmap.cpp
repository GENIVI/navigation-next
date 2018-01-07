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

#include "toolbarbasicmap.h"

ToolBarBasicMap::ToolBarBasicMap(QWidget *parent):
    ToolBarCommon(parent),
    mNightModeButton(NULL),
    mDebugViewButton(NULL),
    mIsNightMode(false),
    mIsDebugViewShown(false)
{
    mDescriptionLabel->setText( "operate the map with gestures" );

    mNightModeButton = new QPushButton( "night mode" );
    mDebugViewButton = new QPushButton( "show debug view" );
    mNightModeButton->setMinimumHeight( 31 );
    mDebugViewButton->setMinimumHeight( 31 );
    connect( mNightModeButton, SIGNAL(clicked()), this, SLOT(OnNightModeClicked()) );
    connect( mDebugViewButton, SIGNAL(clicked()), this, SLOT(OnShowDebugViewClicked()) );

    QVBoxLayout* pButtonLayout = new QVBoxLayout;
    pButtonLayout->addWidget( mNightModeButton );
    pButtonLayout->addWidget( mDebugViewButton );

    mMainLayOut->addLayout( pButtonLayout );
    mMainLayOut->addWidget( mBackButton );
}

void ToolBarBasicMap::OnNightModeClicked()
{
    mIsNightMode = QBool( !mIsNightMode );
    if( mIsNightMode )
    {
        mNightModeButton->setText( "day mode" );
    }
    else
    {
        mNightModeButton->setText( "night mode" );
    }
    emit SigNightMode( mIsNightMode );
}

void ToolBarBasicMap::OnShowDebugViewClicked()
{
    mIsDebugViewShown = QBool(!mIsDebugViewShown);
    if( mIsDebugViewShown )
    {
        mDebugViewButton->setText( "hide debug view" );
    }
    else
    {
        mDebugViewButton->setText( "show debug view" );
    }
    emit SigShowDebugView( mIsDebugViewShown );
}
