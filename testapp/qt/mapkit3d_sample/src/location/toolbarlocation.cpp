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

#include "toolbarlocation.h"

ToolBarLocation::ToolBarLocation(QWidget *parent) :
    ToolBarCommon(parent),
    mNewYorkButton(NULL),
    mLongBeachButton(NULL)
{
    mDescriptionLabel->setText( "click the buttons to move" );

    mNewYorkButton = new QPushButton( "Go To New York" );
    mLongBeachButton = new QPushButton( "Go To Long Beach" );
    mNewYorkButton->setMinimumHeight( 31 );
    mLongBeachButton->setMinimumHeight( 31 );
    connect( mNewYorkButton, SIGNAL(clicked()), this, SLOT(OnNewYorkClicked()) );
    connect( mLongBeachButton, SIGNAL(clicked()), this, SLOT(OnLongBeachClicked()) );

    QVBoxLayout* pButtonLayout = new QVBoxLayout;
    pButtonLayout->addWidget( mNewYorkButton );
    pButtonLayout->addWidget( mLongBeachButton );

    mMainLayOut->addLayout( pButtonLayout );
    mMainLayOut->addWidget( mBackButton );
}

void ToolBarLocation::OnNewYorkClicked()
{
    emit SigMoveCameraTo( 40.7160, -74.0028 );
}

void ToolBarLocation::OnLongBeachClicked()
{
    emit SigMoveCameraTo( 33.76985, -118.19302 );
}