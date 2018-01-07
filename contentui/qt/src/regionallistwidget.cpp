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

#include "regionallistwidget.h"
#include "regionallistitemwidget.h"
#include <QBoxLayout>
#include <QLabel>
#include <QListWidget>
#include "OnboardContentManager.h"
#include "regionallistcontroller.h"

using namespace locationtoolkit;

static const QString titleString = "Download Maps";
static const QString descriptionString = "Download regional maps. After you download the maps, you should restart application to enable onboard feature.";

RegionalListWidget::RegionalListWidget(QWidget *parent) :
    QWidget(parent),
    mController(new RegionalListController)
{
    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->setMargin(0);

    QLabel* titleLabel = new QLabel(titleString);
    titleLabel->setAlignment(Qt::AlignHCenter);
    titleLabel->setFont(QFont(titleLabel->font().family(), 18, QFont::Bold));
    titleLabel->setStyleSheet("QLabel{background:rgb(153,153,153);color:white}");
    vLayout->addWidget(titleLabel);

    QLabel* descriptionLabel = new QLabel(descriptionString);
    descriptionLabel->setAlignment(Qt::AlignHCenter);
    descriptionLabel->setWordWrap(true);
    vLayout->addWidget(descriptionLabel);

    QListWidget* listWidget = new QListWidget();
    listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    listWidget->setFocusPolicy(Qt::NoFocus);
    vLayout->addWidget(listWidget);

    this->setLayout(vLayout);

    mController->SetListWidget(listWidget);
}

RegionalListWidget::~RegionalListWidget()
{
    delete mController;
}

void
RegionalListWidget::Initialize(LTKContext &ltkContext, const QString& workPath)
{
    mController->Initialize(ltkContext, workPath);
}
