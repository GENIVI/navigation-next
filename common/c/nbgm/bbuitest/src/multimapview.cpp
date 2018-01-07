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

#include "multimapview.h"
#include "nbgmbb10rendercontext.h"
#include "nbtaskqueue.h"
#include "bbutil.h"
#include "nbgmgesturehandler.h"
#include "nbgmdevicelocation.h"
#include "tilenameparser.h"
#include "paltimer.h"
#include <bb/cascades/PinchEvent>
#include <bb/cascades/pickers/FilePicker>
#include "nbrelog.h"
#include "nbremath.h"
#include <bb/cascades/Page>
#include <bb/cascades/ActionItem>
#include <bb/cascades/ForeignWindowControl>
#include <bb/cascades/NavigationPaneProperties>
#include <bb/cascades/StackLayout>
#include <bb/cascades/AbsoluteLayout>
#include <bb/cascades/AbsoluteLayoutProperties>
#include <bb/cascades/ImageView>

using namespace bb::cascades::pickers;
using namespace bb::cascades;

static const char* NBGM_WORK_SPACE = "app/native/assets/nbgmresource/";


MultiMapView::MultiMapView(): mNavigationPane(NULL)
{
    mMapview = new LightWeightMapView*[2];
    mMapview[0] = new LightWeightMapView();
    mMapview[1] = new LightWeightMapView();
}

MultiMapView::~MultiMapView()
{
}

void MultiMapView::onOpenNBMFile()
{
    FilePicker* filePicker = new FilePicker();
    filePicker->setType(FileType::Other);
    filePicker->setTitle("SelectNBM File");
    filePicker->setMode(FilePickerMode::PickerMultiple);
    filePicker->open();

    // Connect the fileSelected() signal with the slot.
    QObject::connect(filePicker,
        SIGNAL(fileSelected(const QStringList&)),
        this,
        SLOT(onFileSelected(const QStringList&)));
}

void MultiMapView::onUnloadAllFile()
{
    mMapview[0]->onUnloadAllFile();
    mMapview[1]->onUnloadAllFile();
}

void MultiMapView::onFileSelected(const QStringList& files)
{
    mMapview[0]->onFileSelected(files);
    mMapview[1]->onFileSelected(files);
}


bool MultiMapView::Initialize(NavigationPane* navPane)
{
    mNavigationPane = navPane;

    Page* page = Page::create();
    ActionItem* openNBMFileAction = ActionItem::create().title("Open NBM file");
    page->addAction(openNBMFileAction);
    page->connect(openNBMFileAction, SIGNAL(triggered()), this, SLOT(onOpenNBMFile()));

    ActionItem* unloadAction = ActionItem::create().title("Unload All File");
    page->addAction(unloadAction);
    page->connect(unloadAction, SIGNAL(triggered()), this, SLOT(onUnloadAllFile()));

    mNavigationPane->push(page);

    Container* container = Container::create();
    container->setLayout(new AbsoluteLayout());
    page->setContent(container);

    mMapview[0]->Initialize(container, 0, 0, 768, 640);
    mMapview[1]->Initialize(container, 0, 640, 768, 640);

    bb::cascades::ActionItem* backAction = ActionItem::create().title("Back");
    page->setPaneProperties(NavigationPaneProperties::create().backButton(backAction));

    QObject::connect(backAction, SIGNAL(triggered()), this, SLOT(onBackClickedAction()));

    return true;
}




void MultiMapView::onBackClickedAction()
{
    mNavigationPane->pop();
    delete this;
}
