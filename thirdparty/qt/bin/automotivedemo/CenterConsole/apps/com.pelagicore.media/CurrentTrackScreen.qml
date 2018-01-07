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

import controls 1.0
import utils 1.0
import service.music 1.0
import "."

UIScreen {
    id: root
    hspan: 24
    vspan: 24

    title: 'Music'

    property var track: MusicProvider.currentEntry
    property bool libraryVisible: false

    signal showAlbums()

    Image {
        id: screenCast
        source: Style.symbol("screencast")
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        MouseArea {
            anchors.fill: parent
            onClicked: {

            }
        }
    }

    ColumnLayout {
        id: musicControl
        width: Style.screenWidth
        height: root.height
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 0
        Spacer {
            vspan: 2
        }

        SwipeView {
            id: pathView
            itemWidth: Style.screenWidth

            width: Style.screenWidth
            height: Style.cellHeight * 12

            items: MusicProvider.nowPlaying.model

            currentViewIndex: MusicProvider.currentIndex

            onCurrentViewIndexChanged: MusicProvider.currentIndex = pathView.currentViewIndex

            delegate: CoverItem {
                z: PathView.z
                scale: PathView.scale
                source: MusicProvider.coverPath(model.cover)
                title: model.title
                subTitle: model.artist
                onClicked: {
                  //  MusicService.togglePlay()
                    root.showAlbums()
                }
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            spacing: 10
            Label {
                id: textElement
                text: MusicService.currentTime
            }

            Slider {
                id: slider
                value: MusicService.position
                minimum: 0.00
                maximum: MusicService.duration
                vspan: 1
                function valueToString() {
                    return Math.floor(value/60000) + ':' + Math.floor((value/1000)%60)
                }
                onActiveValueChanged: {
                    MusicService.seek(activeValue)
                }
            }

            Label {
                text: MusicService.durationTime
            }
        }

        RowLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 0
            Tool {
                hspan: 2
                name: 'video-previous'
                onClicked: MusicService.previousTrack()
                size: Style.symbolSizeXS
            }
            Spacer { hspan: 2 }
            Tool {
                hspan: 2
                name: MusicService.playing?'video-pause':'video-play'
                onClicked: MusicService.togglePlay()
            }
            Spacer { hspan: 2 }
            Tool {
                hspan: 2
                name: 'video-next'
                onClicked: MusicService.nextTrack()
                size: Style.symbolSizeXS
            }
        }
        Spacer {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    Library {
        id: library
        x: parent.width
        opacity: 0
        visible: opacity > 0
        onClose: {
            root.libraryVisible = false
        }
    }

    UIElement {
        id: sourceOption
        hspan: 4
        vspan: 12
        anchors.right: musicControl.left
        anchors.rightMargin: 60
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -60

        Column {
            spacing: 1
            Button_neptune {
                hspan: 4
                vspan: 4
                text: "BLUETOOTH"
                label.font.pixelSize: Style.fontSizeL
            }
            Button_neptune {
                hspan: 4
                vspan: 4
                text: "USB"
                enabled: false
                label.font.pixelSize: Style.fontSizeL
            }
            Button_neptune {
                hspan: 4
                vspan: 4
                text: "SPOTIFY"
                enabled: false
                label.font.pixelSize: Style.fontSizeL
            }
        }
    }

    Component.onCompleted: MusicProvider.selectRandomTracks()

    states: State {
        name: "libaryMode"; when: root.libraryVisible

        PropertyChanges {
            target: library
            opacity: 1
            x: root.width - library.width
        }

        PropertyChanges {
            target: libraryButton
            opacity: 0
        }

        PropertyChanges {
            target: sourceOption
            opacity: 0
        }

        AnchorChanges {
            target: musicControl
            anchors.horizontalCenter: undefined
        }

        PropertyChanges {
            target: musicControl
            x: 0
        }
    }

    transitions: Transition {
        from: ""; to: "libaryMode"; reversible: true

        ParallelAnimation {
            NumberAnimation { target: library; properties: "opacity"; duration: 400 }
            NumberAnimation { target: library; properties: "x"; duration: 300 }
            NumberAnimation { target: libraryButton; properties: "opacity"; duration: 300 }
            NumberAnimation { target: sourceOption; properties: "opacity"; duration: 300 }
            NumberAnimation { target: musicControl; properties: "x"; duration: 300 }
        }
    }

}
