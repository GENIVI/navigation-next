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

#ifndef _MAPLEGENDWIDGET_H_
#define _MAPLEGENDWIDGET_H_

#include "MapLegendInfo.h"
#include "maplegendcolorwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QToolButton>
#include <QTimer>
#include <QGraphicsView>

class MapViewUIInterface;
class MapLegendWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapLegendWidget(QWidget *parent = 0);
    ~MapLegendWidget();

public:
    void SetMapUIInterface(MapViewUIInterface* mapViewUI);
    void UpdateButtonState(bool animationCanPlay);
    void UpdateTimestampLabel(uint32 gpsTime);
    void ShowMapLegend(shared_ptr<nbmap::MapLegendInfo> mapLegend);
    void HideMapLegend();

private:
    void SetLegendInfo( const nbmap::MapLegendInfo& info );

private slots:
    void on_startPauseButton_clicked();
    void on_horizontalSlider_valueChanged(int value);

private:
    QToolButton* mStartPauseButton;
    QSlider* mHorizontalSlider;
    QLabel *mLabelTime;
    QLabel *mLabelOpacity;
    QGraphicsView *mGraphicsView;
    QHBoxLayout* mColorBarLayout;

    bool mStartAnimation;
    QList<MapLegendColorWidget*> mColorBars;
    QList<QSpacerItem*> mSpacers;

    const static int mWidgetWidth;
    const static int mWidgetHeight;
    const static int mColorBlockSize;
    static const int DEFAULTDOPPLEROPACITY = 50;

    MapViewUIInterface* m_mapViewUI;
    int m_dopplerOpacity;
};

#endif // _MAPLEGENDWIDGET_H_
