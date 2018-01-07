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

#include "regionallistcontroller.h"
#include "regionallistitemwidget.h"
#include "ltkcontextimpl.h"
#include "OnboardContentManager.h"
#include <QtDebug>
#include <QDir>
#include <QCoreApplication>

static const QSize itemSize = QSize( 280, 70 );

using namespace locationtoolkit;
using namespace nbcommon;

static void UpdateListItemStatus(RegionalListItemWidget* widget, const RegionDownloadStatus& status, int percentage)
{
    bool needChangeIcon = widget->Status() == status;

    switch(status)
    {
    case RDS_NotStart:
    case RDS_Paused:
    case RDS_Failed:
    case RDS_Removed:
        widget->SetStatus(RegionalListItemWidget::IS_UNINSTALL);
        widget->SetStatus(QString::number(widget->Data().uncompressedSize / 1000000)+" MB");
        break;
    case RDS_Downloading:
        widget->SetStatus(RegionalListItemWidget::IS_DOWNLOADING);
        widget->SetStatus(QString::number(percentage)+"% of "+QString::number(widget->Data().uncompressedSize / 1000000)+" MB");
        break;
    case RDS_Compelete:
        widget->SetStatus(RegionalListItemWidget::IS_INSTALLED);
        widget->SetStatus("Installed - "+QString::number(widget->Data().uncompressedSize / 1000000)+" MB");
        break;
    default:
        break;
    }

    if(needChangeIcon)
    {
        QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
        QString strRoot = dir.absolutePath() + "/resource/images/";
        if(widget->Status() == RegionalListItemWidget::IS_UNINSTALL)
        {
            QIcon icon(strRoot + "download.png");
            widget->SetIcon(icon);
        }
        else
        {
            QIcon icon(strRoot + "cancel.png");
            widget->SetIcon(icon);
        }
    }
}

class OnboardContentManagerRef
{
public:
    OnboardContentManagerRef() {}
    ~OnboardContentManagerRef() {}

public:
    shared_ptr<OnboardContentManager> onBoardContentManager;
};

class RegionalListController::RegionalMapDataListenerImpl : public RegionalMapDataListener
{
public:
    RegionalMapDataListenerImpl(RegionalListController* controller) : mController(controller) {}
    ~RegionalMapDataListenerImpl() {}

public:
    virtual void OnError(NB_Error error);

    virtual void OnAvailableRegions(const std::vector<RegionalInformationPtr>& regions);

private:
    RegionalListController* mController;
};

class RegionalListController::DownloadStatusListenerImpl : public DownloadStatusListener
{
public:
    DownloadStatusListenerImpl(RegionalListController* controller) : mController(controller) {}
    ~DownloadStatusListenerImpl() {}
public:
    virtual void OnStatusChanged(const std::string& regionalId, NB_EnhancedContentDataType dataType,
                                 RegionDownloadStatus newStatus);
    virtual void OnError(const std::string& regionalId, NB_Error error);
    virtual void OnProgress(const std::string& regionalId, int percentage);

private:
    RegionalListController* mController;
};

void
RegionalListController::RegionalMapDataListenerImpl::OnError(NB_Error error)
{
    qDebug()<<"RegionalMapDataListener::OnError errorCode = "<< error <<endl;
}

void
RegionalListController::RegionalMapDataListenerImpl::OnAvailableRegions(const std::vector<RegionalInformationPtr> &regions)
{
    mController->mListWidget->clear();
    for(std::vector<RegionalInformationPtr>::const_iterator iter = regions.begin(); iter != regions.end(); iter++)
    {
        const RegionalInformationPtr& infoPtr = *iter;
        RegionalListItemWidget* widget = mController->AddListItem(QString(infoPtr->m_regionalID.c_str()), QString(infoPtr->m_regionalID.c_str()));
        RegionalListItemWidget::ItemData data;
        data.downloadSize = infoPtr->m_downloadedSize;
        data.uncompressedSize = infoPtr->m_uncompressedSize;
        data.version = QString(infoPtr->m_version.c_str());
        widget->SetData(data);
        UpdateListItemStatus(widget, infoPtr->m_downloadStatus, infoPtr->m_downloadedProgress);
    }
}

void
RegionalListController::DownloadStatusListenerImpl::OnError(const string &regionalId, NB_Error error)
{
    qDebug()<<"DownloadStatusListenerImpl::OnError regionalId = "<<regionalId.c_str()<<", errorCode = " << error<<endl;
}

void
RegionalListController::DownloadStatusListenerImpl::OnStatusChanged(const string &regionalId, NB_EnhancedContentDataType dataType, RegionDownloadStatus newStatus)
{
    QString id(regionalId.c_str());
    RegionalListItemWidget* widget = mController->GetListItem(id);
    if(widget)
    {
        UpdateListItemStatus(widget, newStatus, 0);
    }
    else
    {
        qDebug()<<"DownloadStatusListenerImpl OnStatusChanged incorrect id = "<<id<<endl;
    }
}

void
RegionalListController::DownloadStatusListenerImpl::OnProgress(const string &regionalId, int percentage)
{
    QString id(regionalId.c_str());
    RegionalListItemWidget* widget = mController->GetListItem(id);
    if(widget)
    {
        UpdateListItemStatus(widget, RDS_Downloading, percentage);
    }
    else
    {
        qDebug()<<"DownloadStatusListenerImpl OnProgress incorrect id = "<<id<<endl;
    }
}

RegionalListController::RegionalListController(QObject *parent) :
    QObject(parent),
    mContentManagerRef(new OnboardContentManagerRef()),
    mMapDataListenerImpl(NULL),
    mDownloadStatusListenerImpl(NULL)
{
    mMapDataListenerImpl = new RegionalMapDataListenerImpl(this);
    mDownloadStatusListenerImpl = new DownloadStatusListenerImpl(this);
}

RegionalListController::~RegionalListController()
{
    if(mContentManagerRef->onBoardContentManager)
    {
        mContentManagerRef->onBoardContentManager->UnregisterDownloadStatusListener(mDownloadStatusListenerImpl);
        mContentManagerRef->onBoardContentManager->UnregisterRegionalMapDataListener(mMapDataListenerImpl);
    }

    if(mContentManagerRef)
    {
        delete mContentManagerRef;
    }

    if(mMapDataListenerImpl)
    {
        delete mMapDataListenerImpl;
    }

    if(mDownloadStatusListenerImpl)
    {
        delete mDownloadStatusListenerImpl;
    }
}

void
RegionalListController::Initialize(LTKContext &ltkContext, const QString& workPath)
{
    LTKContextImpl& contextImpl = static_cast<LTKContextImpl &>( ltkContext );
    //Copy from android, need to refactor
    OnboardContentManagerConfigPtr config(new OnboardContentManagerConfig(
                                                780,
                                                585,
                                                780 * 585,
                                                "USA",
                                                "EN-US",
                                                "1",
                                                workPath.toStdString(),
                                                workPath.toStdString(),
                                                "1234567890",
                                                NB_ECDT_MAP_ALL));
    mContentManagerRef->onBoardContentManager = OnboardContentManager::CreateOnboardContentManager(contextImpl.GetNBContext(), config);
    mContentManagerRef->onBoardContentManager->RegisterRegionalMapDataListener(mMapDataListenerImpl);
    mContentManagerRef->onBoardContentManager->RegisterDownloadStatusListener(mDownloadStatusListenerImpl);
    mContentManagerRef->onBoardContentManager->CheckAvaliableRegions();
}

void
RegionalListController::SetListWidget(QListWidget *listWidget)
{
    mListWidget = listWidget;
}

RegionalListItemWidget*
RegionalListController::AddListItem(const QString &id, const QString &name)
{
    RegionalListItemWidget* content = new RegionalListItemWidget(mListWidget);
    content->Initialize(id, this);
    content->SetName(name);
    QListWidgetItem *item = new QListWidgetItem(mListWidget);
    mListWidget->setItemWidget(item, content);
    item->setSizeHint(itemSize);
    return content;
}

RegionalListItemWidget*
RegionalListController::GetListItem(const QString &id)
{
    for(int i = 0; i < mListWidget->count(); i++)
    {
        RegionalListItemWidget* widget = static_cast<RegionalListItemWidget*> (mListWidget->itemWidget(mListWidget->item(i)));
        if(widget->Id() == id)
        {
            return widget;
        }
    }
    return NULL;
}

void
RegionalListController::OnButtonClicked(const QString& id)
{
    RegionalListItemWidget* widget = GetListItem(id);
    if(widget == NULL)
    {
        qDebug()<<"RegionalListController::OnButtonClicked incorrect id = "<<id<<endl;
        return;
    }
    switch(widget->Status())
    {
    case RegionalListItemWidget::IS_UNINSTALL:
        mContentManagerRef->onBoardContentManager->RequestRegionalData(id.toStdString());
        break;
    case RegionalListItemWidget::IS_DOWNLOADING:
        mContentManagerRef->onBoardContentManager->CancelDownload(id.toStdString());
        break;
    case RegionalListItemWidget::IS_INSTALLED:
        mContentManagerRef->onBoardContentManager->RemoveRegionalData(id.toStdString());
        break;
    default:
        break;
    }
}
