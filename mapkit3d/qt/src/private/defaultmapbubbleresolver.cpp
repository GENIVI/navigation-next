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

#include "defaultmapbubbleresolver.h"
#include "bubbleinternal.h"
#include "defaultbubble.h"
//#include <QDir>
//#include <QCoreApplication>

using namespace locationtoolkit;

DefaultMapBubbleResolver::DefaultMapBubbleResolver(const QString& resourcePath, QWidget *parent):
    mResourcePath(resourcePath),
    mParent( parent )
{
}

shared_ptr<nbmap::BubbleInterface> DefaultMapBubbleResolver::GetPinBubble(double latitude, double longitude)
{
    QString title = "Location:";
    QString content;
    content.sprintf( "lat: %.3f, lon: %.3f", latitude, longitude );

    QString bubbleImagePath = mResourcePath + "/images/map_bubble.9.png";

    DefaultBubble* bubbleImpl = new DefaultBubble( title, content, bubbleImagePath, mParent );
    return shared_ptr<nbmap::BubbleInterface>( new BubbleInternal( bubbleImpl ) );
}

shared_ptr<nbmap::BubbleInterface> DefaultMapBubbleResolver::GetStaticPoiBubble(const string &id, const string &name, double /*latitude*/, double /*longitude*/)
{
    QString title = QString::fromStdString( id );
    //in CCC's nbgm,"name" has been appended two extra "\0" which will display extra charaters in linux QT's UI,
    //in order to reduce affects on ther products,so abandon them in ltk.
    const char* cName = name.c_str();
    QString content( cName );

    QString bubbleImagePath = mResourcePath + "/images/map_bubble.9.png";
    DefaultBubble* bubbleImpl = new DefaultBubble( title, content, bubbleImagePath, mParent );
    return shared_ptr<nbmap::BubbleInterface>( new BubbleInternal( bubbleImpl ) );
}

shared_ptr<nbmap::BubbleInterface> DefaultMapBubbleResolver::GetTrafficIncidentBubble(int /*type*/,
                                                                                      int /*severity*/,
                                                                                      long /*entryTime*/,
                                                                                      long /*startTime*/,
                                                                                      long /*endTime*/,
                                                                                      const string &description,
                                                                                      const string &road,
                                                                                      double /*lat*/,
                                                                                      double /*lon*/)
{
    QString title = QString::fromStdString( road );
    QString content = QString::fromStdString( description );

    QString bubbleImagePath = mResourcePath + "/images/map_bubble.9.png";

    DefaultBubble* bubbleImpl = new DefaultBubble( title, content, bubbleImagePath, mParent );
    return shared_ptr<nbmap::BubbleInterface>(new BubbleInternal(bubbleImpl));
}
