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

#include "menuwidget.h"
#include "listdelegate.h"
#include "mainwindow.h"
#include <QVBoxLayout>

MenuWidget::MenuWidget(QWidget *parent) :
    QWidget(parent),
    mToolBar(NULL),
    mListWidget(NULL)
{
    mToolBar = new ToolBarMenu;
    mToolBar->setMaximumHeight( mToolBar->geometry().height() );

    mListWidget = new QListWidget;
    InitListWidget();

    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->setMargin( 0 );
    pLayout->setSpacing( 0 );
    pLayout->addWidget( mToolBar );
    pLayout->addWidget( mListWidget );
    this->setLayout( pLayout );

    connect( mListWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(OnListClicked(QModelIndex)) );    
}

void MenuWidget::OnListClicked(const QModelIndex& index)
{
    QString title = index.data(Qt::DisplayRole).toString();
    WIDGET_TYPE type = WIDGET_MENU;

    if( title == "Basic Map" )
    {
        type = WIDGET_BASIC_MAP;
    }
    else if( title == "POI Pin" )
    {
        type = WIDGET_POI_PIN;
    }
    else if( title == "Location" )
    {
        type = WIDGET_LOCATION;
    }
    else if( title == "Follow Me" )
    {
        type = WIDGET_FOLLOW_ME;
    }
    else if( title == "Optional layers" )
    {
        type = WIDGET_OPTIONAL_LAYERS;
    }
    else if( title == "Show Poi List" )
    {
        type = WIDGET_POI_LIST;
    }
    else if( title == "Settings" )
    {
        type = WIDGET_SETTINGS;
    }
    else if( title == "About the sample" )
    {
        type = WIDGET_ABOUT;
    }
    else if( title == "Camera" )
    {
        type = WIDGET_CAMERA;
    }
    else if( title == "Avatar" )
    {
        type = WIDGET_AVATAR;
    }

    emit SigShowWidget( type );
}

void MenuWidget::InitListWidget()
{
    AddListItem( "Basic Map", "Launches a map" );
    AddListItem( "POI Pin", "Show POI on map" );
    AddListItem( "Location", "Center map to location" );
    AddListItem( "Camera", "Show carmera options" );
    AddListItem( "Avatar", "Show avatar on the map" );
    AddListItem( "Follow Me", "Follow me mode" );
    AddListItem( "Optional layers", "Show the specified layers" );
    AddListItem( "Show Poi List", "Show all POI pins in one screen" );
    AddListItem( "Settings", "Set Server and GPS file" );
    AddListItem( "About the sample", "Show version information about the sample" );

    ListDelegate *delegate = new ListDelegate( mListWidget );
    mListWidget->setItemDelegate(delegate);
    mListWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(238, 238, 238);"));
}

void MenuWidget::AddListItem(QString theme, QString description)
{
    QListWidgetItem *item = new QListWidgetItem();
    item->setData(Qt::DisplayRole,  theme );
    item->setData(Qt::UserRole, description );
    mListWidget->addItem(item);
}
