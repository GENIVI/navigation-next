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

#ifndef __REGIONAL_LIST_ITEM_WIDGET_H__
#define __REGIONAL_LIST_ITEM_WIDGET_H__

#include <QWidget>
#include <QLabel>
#include <QToolButton>

namespace locationtoolkit {

class RegionalListController;

class RegionalListItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RegionalListItemWidget(QWidget *parent = 0);

public:
    enum ItemStatus
    {
        IS_UNINSTALL,
        IS_DOWNLOADING,
        IS_INSTALLED
    };

public:
    struct ItemData
    {
        int downloadSize;
        int uncompressedSize;
        QString version;
    };

public:
    void Initialize(const QString& id, RegionalListController* controller);
    const QString& Id() const;
    void SetData(const ItemData& itemData);
    const ItemData& Data() const;
    void SetStatus(const ItemStatus& status);
    const ItemStatus& Status() const;
    void SetName(const QString&  name);
    void SetStatus(const QString&  status);
    void SetIcon(const QIcon& icon);

signals:
    void ClickButton(const QString& id);

public slots:
    void OnClicked();

private:
    QString mId;
    ItemData mItemData;
    ItemStatus mStatus;
    QLabel* mContentName;
    QLabel* mContentStatus;
    QToolButton* mDownloadStatusButton;
};

}

#endif // CONTENTLISTITEM_H
