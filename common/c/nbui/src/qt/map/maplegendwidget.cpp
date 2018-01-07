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

#include "maplegendwidget.h"
#include <QPixmap>
#include <QTime>
#include <QSpacerItem>
#include <QDir>
#include <QCoreApplication>
#include "MapViewUIInterface.h"
#include <QMessageBox>

const int MapLegendWidget::mWidgetWidth(426);
const int MapLegendWidget::mWidgetHeight(55);
const int MapLegendWidget::mColorBlockSize(10);

MapLegendWidget::MapLegendWidget(QWidget *parent) :
    QWidget(parent),
    mStartAnimation(true),
    mStartPauseButton(NULL),
    mHorizontalSlider(NULL),
    mLabelTime(NULL),
    mLabelOpacity(NULL),
    mColorBarLayout(NULL),
    mGraphicsView(NULL),
    m_mapViewUI(NULL),
    m_dopplerOpacity(DEFAULTDOPPLEROPACITY)
{
    resize(mWidgetWidth, mWidgetHeight);
    this->setWindowFlags( Qt::FramelessWindowHint );

    mGraphicsView = new QGraphicsView(this);
    mGraphicsView->setGeometry( this->geometry() );
    mGraphicsView->setStyleSheet(QString::fromUtf8("background-color: rgb(248, 248, 248);"));

    mStartPauseButton = new QToolButton;
    mHorizontalSlider = new QSlider;
    mHorizontalSlider->setOrientation(Qt::Horizontal);

    mLabelTime = new QLabel;
    mLabelOpacity = new QLabel( "OPACITY" );

    QHBoxLayout* opacityLayout = new QHBoxLayout;
    opacityLayout->addWidget( mLabelOpacity );
    opacityLayout->addWidget( mHorizontalSlider );

    mColorBarLayout = new QHBoxLayout;

    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->addLayout( opacityLayout );
    vLayout->addLayout( mColorBarLayout );

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget( mStartPauseButton );
    mainLayout->addWidget( mLabelTime );
    mainLayout->addLayout( vLayout );

    setLayout( mainLayout );

    QFont font;
    font.setPointSize(12);
    mLabelTime->setFont( font );
    mLabelOpacity->setFont( font );

    QPixmap pix;
    QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
    QString strRoot = dir.absolutePath();

    pix.load( strRoot + "/resource/images/pause.png" );
    mStartPauseButton->setIcon( pix );

    mHorizontalSlider->setMinimum( 0 );
    mHorizontalSlider->setMaximum( 100 );

    connect( mStartPauseButton, SIGNAL(clicked()), this, SLOT(on_startPauseButton_clicked()) );
    connect( mHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(on_horizontalSlider_valueChanged(int)) );
}

MapLegendWidget::~MapLegendWidget()
{
}

void MapLegendWidget::SetLegendInfo(const nbmap::MapLegendInfo& info)
{
    // clear layout
    for( int i = 0; i < mColorBars.size(); i++ )
    {
        mColorBarLayout->removeWidget( mColorBars[i] );
        mColorBars[i]->deleteLater();
    }
    mColorBars.clear();

    for( int i = 0; i < mSpacers.size(); i++ )
    {
        mColorBarLayout->removeItem( mSpacers[i] );
        delete mSpacers[i];
    }
    mSpacers.clear();

    // then add items to it
    for( uint32 i = 0; i < info.GetTotalColorBars(); i++ )
    {
        std::vector<nb_color>& colors = info.GetColorBar(i)->second;
        QList<QColor> qcolors;
        for( size_t index = 0; index < colors.size(); index++ )
        {
            QColor color( NB_COLOR_GET_R( colors[index] ), NB_COLOR_GET_G( colors[index] ), NB_COLOR_GET_B( colors[index] ) );
            qcolors.push_back( color );
        }

        QString strTitle = QString::fromStdString( info.GetColorBar(i)->first );
        MapLegendColorWidget* colorWidget = new MapLegendColorWidget( strTitle, qcolors, mColorBlockSize );
        mColorBarLayout->addWidget( colorWidget );
        mColorBars.push_back( colorWidget );

        if ( i < info.GetTotalColorBars() - 1 )
        {
            QSpacerItem* spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
            mColorBarLayout->addSpacerItem( spacer );
            mSpacers.push_back( spacer );
        }
    }
}

void MapLegendWidget::SetMapUIInterface(MapViewUIInterface *mapViewUI)
{
    m_mapViewUI = mapViewUI;
}

void MapLegendWidget::UpdateButtonState(bool animationCanPlay)
{
    QPixmap pix;
    QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
    QString strRoot = dir.absolutePath();

    if( animationCanPlay)
    {
        mStartAnimation = true;
        pix.load( strRoot + "/resource/images/pause.png" );
    }
    else
    {
        mStartAnimation = false;
        pix.load( strRoot + "/resource/images/start.png" );
    }

    mStartPauseButton->setIcon( pix );
}

void MapLegendWidget::UpdateTimestampLabel(uint32 gpsTime)
{
    QDateTime t;
    t.setTime_t(gpsTime);
    QString strTime = t.toString( "hh:mm AP" );
    mLabelTime->setText( strTime );
}

void MapLegendWidget::ShowMapLegend(shared_ptr<nbmap::MapLegendInfo> mapLegend)
{
    SetLegendInfo( *mapLegend );
    mHorizontalSlider->setValue( m_dopplerOpacity );
    show();
}

void MapLegendWidget::HideMapLegend()
{
    hide();
}

void MapLegendWidget::on_startPauseButton_clicked()
{
    QPixmap pix;
    mStartAnimation = !mStartAnimation;
    QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
    QString strRoot = dir.absolutePath();

    if( mStartAnimation )
    {
        pix.load( strRoot + "/resource/images/pause.png" );
        if(m_mapViewUI)
        {
            m_mapViewUI->UI_SetDopplerPlayState(true);
        }
    }
    else
    {
        pix.load( strRoot + "/resource/images/start.png" );
        if(m_mapViewUI)
        {
            m_mapViewUI->UI_SetDopplerPlayState(false);
        }
    }
    mStartPauseButton->setIcon( pix );
}

void MapLegendWidget::on_horizontalSlider_valueChanged(int value)
{
    m_dopplerOpacity = value;
    if(m_mapViewUI)
    {
        m_mapViewUI->UI_SetDopplerOpacity(m_dopplerOpacity);
    }
}
