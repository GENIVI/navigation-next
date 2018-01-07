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

#include "toolbarcamera.h"

ToolBarCamera::ToolBarCamera(QWidget *parent):
    ToolBarCommon(parent),
    mIsAnimate( false )
{
    mDescriptionLabel->setText( "Show camera options" );

    QVBoxLayout* pColum1Layout = new QVBoxLayout;
    mBtnAni = new QPushButton( tr("Animate on") );
    mBtnAni->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );
    pColum1Layout->addWidget( mBtnAni );

    QHBoxLayout* pLine1Layout = new QHBoxLayout;
    QPushButton* pUpBtn = new QPushButton( tr("Up") );
    pLine1Layout->addStretch();
    pLine1Layout->addWidget( pUpBtn );
    pLine1Layout->addStretch();

    QHBoxLayout* pLine2Layout = new QHBoxLayout;
    QPushButton* pLeftBtn = new QPushButton( tr("Left") );
    QPushButton* pDownBtn = new QPushButton( tr("Down") );
    QPushButton* pRightBtn = new QPushButton( tr("Right") );
    pLine2Layout->addWidget( pLeftBtn );
    pLine2Layout->addWidget( pDownBtn );
    pLine2Layout->addWidget( pRightBtn );

    QVBoxLayout* pColum2Layout = new QVBoxLayout;
    pColum2Layout->addLayout( pLine1Layout );
    pColum2Layout->addLayout( pLine2Layout );

    QVBoxLayout* pColum3Layout = new QVBoxLayout;
    QPushButton* pZoomInBtn = new QPushButton( tr("+") );
    QPushButton* pZoomOutBtn = new QPushButton( tr("-") );
    pColum3Layout->addWidget( pZoomInBtn );
    pColum3Layout->addWidget( pZoomOutBtn );

    QHBoxLayout* pMainLine1Layout = new QHBoxLayout;
    pMainLine1Layout->addLayout( pColum1Layout );
    pMainLine1Layout->addStretch();
    pMainLine1Layout->addLayout( pColum2Layout );
    pMainLine1Layout->addStretch();
    pMainLine1Layout->addLayout( pColum3Layout );

    QHBoxLayout* pMainLine2Layout = new QHBoxLayout;
    QPushButton* pGoTo1Btn = new QPushButton( tr("Go to Long Beach") );
    QPushButton* pGoTo2Btn = new QPushButton( tr("Go to New York") );
    pGoTo1Btn->setMaximumHeight( 20 );
    pGoTo2Btn->setMaximumHeight( 20 );
    pMainLine2Layout->addWidget( pGoTo1Btn );
    pMainLine2Layout->addWidget( pGoTo2Btn );

    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addLayout( pMainLine1Layout );
    pLayout->addLayout( pMainLine2Layout );

    mMainLayOut->addLayout( pLayout );
    mMainLayOut->addWidget( mBackButton );

    connect( mBtnAni, SIGNAL(clicked()), this, SLOT(OnAnimateClicked()) );
    connect( pUpBtn, SIGNAL(clicked()), this, SLOT(OnUpClicked()) );
    connect( pDownBtn, SIGNAL(clicked()), this, SLOT(OnDownClicked()) );
    connect( pLeftBtn, SIGNAL(clicked()), this, SLOT(OnLeftClicked()) );
    connect( pRightBtn, SIGNAL(clicked()), this, SLOT(OnRightClicked()) );
    connect( pZoomInBtn, SIGNAL(clicked()), this, SLOT(OnZoomInClicked()) );
    connect( pZoomOutBtn, SIGNAL(clicked()), this, SLOT(OnZoomOutClicked()) );
    connect( pGoTo1Btn, SIGNAL(clicked()), this, SLOT(OnGotoSydniClicked()) );
    connect( pGoTo2Btn, SIGNAL(clicked()), this, SLOT(OnGotoBondiClicked()) );
}

void ToolBarCamera::OnAnimateClicked()
{
    mIsAnimate = QBool( !mIsAnimate );
    if( mIsAnimate == true )
    {
        mBtnAni->setText( tr("Animate off") );
    }
    else
    {
        mBtnAni->setText( tr("Animate on") );
    }
}

void ToolBarCamera::OnUpClicked()
{
    emit CameraActivity( ACTIVITY_UP, mIsAnimate );
}

void ToolBarCamera::OnDownClicked()
{
    emit CameraActivity( ACTIVITY_DOWN, mIsAnimate );
}

void ToolBarCamera::OnLeftClicked()
{
    emit CameraActivity( ACTIVITY_LEFT, mIsAnimate );
}

void ToolBarCamera::OnRightClicked()
{
    emit CameraActivity( ACTIVITY_RIGHT, mIsAnimate );
}

void ToolBarCamera::OnZoomInClicked()
{
    emit CameraActivity( ACTIVITY_ZOOM_IN, mIsAnimate );
}

void ToolBarCamera::OnZoomOutClicked()
{
    emit CameraActivity( ACTIVITY_ZOOM_OUT, mIsAnimate );
}

void ToolBarCamera::OnGotoSydniClicked()
{
    emit MoveCameraTo( "Sydni", mIsAnimate );
}

void ToolBarCamera::OnGotoBondiClicked()
{
    emit MoveCameraTo( "Bondi", mIsAnimate );
}
