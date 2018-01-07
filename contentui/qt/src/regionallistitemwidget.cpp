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

#include "regionallistitemwidget.h"
#include <QBoxLayout>
#include "regionallistcontroller.h"

using namespace locationtoolkit;

RegionalListItemWidget::RegionalListItemWidget(QWidget *parent) :
    QWidget(parent),
    mStatus(IS_UNINSTALL),
    mContentName(NULL),
    mContentStatus(NULL),
    mDownloadStatusButton(NULL)
{
    mContentName = new QLabel();
    mContentName->setFont(QFont(mContentName->font().family(), 14, QFont::Bold));
    mContentStatus = new QLabel();
    mContentStatus->setFont(QFont(mContentStatus->font().family(), 10, QFont::Light) );
    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->addWidget(mContentName);
    vLayout->addWidget(mContentStatus);

    mDownloadStatusButton = new QToolButton();
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addLayout(vLayout);
    hLayout->addWidget(mDownloadStatusButton);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addLayout(hLayout);
    layout->addWidget(line);

    setLayout(layout);
}

void
RegionalListItemWidget::Initialize(const QString& id, RegionalListController* controller)
{
    mId = id;
    connect(mDownloadStatusButton, SIGNAL(clicked()), this, SLOT(OnClicked()));
    connect(this, SIGNAL(ClickButton(QString)), controller, SLOT(OnButtonClicked(QString)));
}

const QString&
RegionalListItemWidget::Id() const
{
    return mId;
}

void
RegionalListItemWidget::SetData(const ItemData &itemData)
{
    mItemData = itemData;
}

const RegionalListItemWidget::ItemData&
RegionalListItemWidget::Data() const
{
    return mItemData;
}

void
RegionalListItemWidget::SetStatus(const ItemStatus& status)
{
    mStatus = status;
}

const RegionalListItemWidget::ItemStatus&
RegionalListItemWidget::Status() const
{
    return mStatus;
}

void
RegionalListItemWidget::SetName(const QString& name)
{
    mContentName->setText(name);
}

void
RegionalListItemWidget::SetStatus(const QString& status)
{
    mContentStatus->setText(status);
}

void
RegionalListItemWidget::SetIcon(const QIcon &icon)
{
    mDownloadStatusButton->setIcon(icon);
}

void
RegionalListItemWidget::OnClicked()
{
    emit ClickButton(mId);
}
