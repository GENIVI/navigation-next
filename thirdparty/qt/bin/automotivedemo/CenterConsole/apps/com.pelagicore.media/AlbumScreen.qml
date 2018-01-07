/****************************************************************************
**
** Copyright (C) 2016 Pelagicore AG
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Neptune IVI UI.
**
** $QT_BEGIN_LICENSE:GPL-QTAS$
** Commercial License Usage
** Licensees holding valid commercial Qt Automotive Suite licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and The Qt Company.  For
** licensing terms and conditions see https://www.qt.io/terms-conditions.
** For further information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
** SPDX-License-Identifier: GPL-3.0
**
****************************************************************************/

import QtQuick 2.1
import QtQuick.Layouts 1.0

import service.music 1.0
import service.movie 1.0
import controls 1.0
import utils 1.0
import "."

UIScreen {
    id: root
    property string modelType: "Music"

    width: Style.screenWidth
    title: modelType === "Music" ? 'Music' : 'Movies'
    signal showCurrentTrack()

    GridView {
        id: grid
        width: Style.screenWidth - 60
        height: 1020
        cellWidth: 340
        cellHeight: root.modelType === "Music" ? 340 : 490
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: Style.statusBarHeight*2
        model: modelType === "Music" ? MusicProvider.nowPlaying.model : MovieProvider.model
        clip: true
        snapMode: GridView.SnapToRow
        delegate: Item {
            width: 330
            height: root.modelType === "Music" ? 330 : 500
            Image {
                source: Style.symbol("RectangleGradient256x512")
                anchors.fill: parent
            }
            Tool {
                vspan: 2
                z: 3
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 120
                name: {
                    if (root.modelType === "Music")
                        active ? "video-pause" : "video-play"
                    else
                        "video-play"
                }
                onClicked: {
                    if(root.modelType === "Music") {
                        active ? MusicService.pause() : MusicService.play()
                        if (MusicProvider.currentIndex !== index) {
                        active ? MusicService.pause() : MusicService.play()
                            MusicProvider.currentIndex = index
                        }
                    }
                    else {
                        MusicService.pause()
                        MovieProvider.currentIndex = index
                        root.showCurrentTrack()
                    }
                }
                active: MusicProvider.currentIndex === index && MusicService.playing
            }
            Image {
                z: 2
                source: Style.symbol('MusicOverlayGradient')
                anchors.fill: parent
            }

            Text {
                z: 2
                width: 300
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: coverImage.bottom
                anchors.bottomMargin: Style.padding
                maximumLineCount: 2
                text: model.title
                color: Style.colorWhite
                wrapMode: Text.WordWrap
                font.pixelSize: Style.fontSizeS
                horizontalAlignment: Text.AlignHCenter
                visible: modelType === "Music"
            }
            Image {
                id: coverImage
                source: modelType === "Music" ? MusicProvider.coverPath(model.cover) : MovieProvider.coverPath(model.cover)
                width: parent.width
                fillMode: Image.PreserveAspectFit
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }


    LaunchItemBackground {
        id: musicLibrary
        text: "Music"
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: Style.padding
        height: 226
        width: Style.launchBarHeight
        scale: musicArea.pressed?0.85:1.0

        Image {
            source: Style.symbol("music")
            anchors.centerIn: parent
        }

        MouseArea {
            id: musicArea
            anchors.fill: parent
            onClicked: {
                root.modelType = "Music"
            }
        }


        Component.onCompleted:  MusicProvider.selectRandomTracks()
    }

    LaunchItemBackground {
        text: "Movies"

        anchors.left: musicLibrary.right
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: Style.padding
        height: 226
        width: Style.launchBarHeight
        scale: movieArea.pressed?0.85:1.0

        Image {
            source: Style.symbol("video")
            anchors.centerIn: parent
        }

        MouseArea {
            id: movieArea
            anchors.fill: parent
            onClicked: {
                root.modelType = "Movies"
            }
        }
        Component.onCompleted:  MovieProvider.selectRandom()
    }
}
