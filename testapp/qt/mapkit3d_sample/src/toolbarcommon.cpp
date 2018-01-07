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

#include "toolbarcommon.h"
#include "mainwindow.h"
#include <QHBoxLayout>
#include <QLabel>

const int ToolBarCommon::mToolbarHeight( 91 );

ToolBarCommon::ToolBarCommon(QWidget *parent) :
    QWidget(parent),
    mBackgroundView(NULL),
    mTitleLabel(NULL),
    mDescriptionLabel(NULL),
    mBackButton(NULL),
    mMainLayOut(NULL)
{
    resize( 400, mToolbarHeight );
    mBackgroundView = new QGraphicsView(this);
    mBackgroundView->setGeometry( this->geometry() );
    mBackgroundView->setStyleSheet(QString::fromUtf8("background-color: rgb(77, 77, 77);"));

    mMainLayOut = new QHBoxLayout;

    mTitleLabel = new QLabel( "Mapkit3d sample" );
    mDescriptionLabel = new QLabel;

    QPalette pa;
    pa.setColor(QPalette::WindowText,Qt::white);
    mTitleLabel->setPalette(pa);
    mDescriptionLabel->setPalette(pa);

    QFont font1;
    font1.setPointSize(14);
    font1.setBold(true);
    font1.setItalic(true);
    font1.setWeight(75);
    mTitleLabel->setFont( font1 );

    QFont font;
    font.setPointSize(11);
    mDescriptionLabel->setFont( font );

    QVBoxLayout* pLabelLayout = new QVBoxLayout;
    pLabelLayout->addWidget( mTitleLabel );
    pLabelLayout->addWidget( mDescriptionLabel );
    mMainLayOut->addLayout( pLabelLayout );

    setLayout( mMainLayOut );

    mBackButton = new QPushButton( "BackToMenu" );
    mBackButton->setMinimumHeight( 31 );
    if( parent != NULL )
    {
        MainWindow* pMainWindow = static_cast<MainWindow *>( parent );
        connect( mBackButton, SIGNAL(clicked()), pMainWindow, SLOT(OnBackButtonClicked()) );
    }
}

void ToolBarCommon::resizeEvent(QResizeEvent *event)
{
    QRect rect;
    rect.setWidth( event->size().width() );
    rect.setHeight( this->height() );
    mBackgroundView->setGeometry( rect );
}
