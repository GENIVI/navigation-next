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

#include "toolbaroptionallayers.h"

ToolBarOptionalLayers::ToolBarOptionalLayers(QWidget *parent) :
    ToolBarCommon(parent),
    mListModel(NULL),
    mListView(NULL)
{
    mDescriptionLabel->setText( "select the layers" );

    QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);

    mListView = new QListView(this);
    mListView->setGeometry(QRect(290, 0, 200, 91));
    sizePolicy1.setHeightForWidth(mListView->sizePolicy().hasHeightForWidth());
    mListView->setSizePolicy(sizePolicy1);
    QFont font2;
    font2.setPointSize(12);
    mListView->setFont(font2);

    mListModel = new QStandardItemModel();
    mListView->setModel( mListModel );
    mListView->update();

    mMainLayOut->addWidget( mListView );
    mMainLayOut->addWidget( mBackButton );

    connect( mListModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(OnListItemChanged(QStandardItem*)) );
}

void ToolBarOptionalLayers::AddLayerList(QString layerName, QBool checked)
{
    QStandardItem *Item = new QStandardItem( layerName );
    Item->setCheckable( true );
    Qt::CheckState chState = checked ? Qt::Checked : Qt::Unchecked;
    Item->setCheckState( chState );
    mListModel->appendRow( Item );
}

void ToolBarOptionalLayers::SetLayerCheckState(QString layerName, QBool checked)
{
    QList<QStandardItem*> items = mListModel->findItems( layerName );
    for( int i = 0; i < items.size(); i++ )
    {
        Qt::CheckState chState = checked ? Qt::Checked : Qt::Unchecked;
        items[i]->setCheckState( chState );
    }
    mListView->update();
}

void ToolBarOptionalLayers::OnListItemChanged(QStandardItem *pItem)
{
    QBool show = pItem->checkState() == Qt::Checked ? QBool(true) : QBool(false);
    emit SigShowLayer( pItem->text(), show );
}
