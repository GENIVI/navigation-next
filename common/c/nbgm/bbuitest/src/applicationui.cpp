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

/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

// Tabbed pane project template
#include "applicationui.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/ActionItem>
#include <QMetaType>
#include <bb/cascades/Container>
#include <bb/cascades/Page>
#include "nbgmuitest.h"
#include <bb/cascades/ForeignWindowControl>
#include <bb/cascades/NavigationPaneProperties>
#include <bb/cascades/PinchHandler>
#include "multimapview.h"

using namespace bb::cascades;

ApplicationUI::ApplicationUI(bb::cascades::Application *app)
: QObject(app)
{
    // create scene document from main.qml asset
    // set parent to created document to ensure it exists for the whole application lifetime

    Page* page = Page::create();

    m_navigationPane = NavigationPane::create();
    m_navigationPane->setPeekEnabled(false);
    m_navigationPane->push(page);
    // set created root object as a scene
    app->setScene(m_navigationPane);

    ActionItem* createMapViewAction = ActionItem::create().title("Create Map View");
    page->addAction(createMapViewAction);
    page->connect(createMapViewAction, SIGNAL(triggered()), this, SLOT(onCreateMapviewButtonClicked()));

    ActionItem* createMultiMapViewAction = ActionItem::create().title("Create Multiple Map View");
    page->addAction(createMultiMapViewAction);
    page->connect(createMultiMapViewAction, SIGNAL(triggered()), this, SLOT(onCreateMultiMapviewButtonClicked()));
}

void
ApplicationUI::onCreateMapviewButtonClicked()
{
    NBGMUITest* mainApp = new NBGMUITest;
    mainApp->Initialize(m_navigationPane);
}

void
ApplicationUI::onCreateMultiMapviewButtonClicked()
{
    MultiMapView* mapView = new MultiMapView;
    mapView->Initialize(m_navigationPane);
}

