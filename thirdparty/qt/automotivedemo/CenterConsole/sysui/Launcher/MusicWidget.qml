/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.1

import QtApplicationManager 1.0
import controls 1.0
import utils 1.0
import controls 1.0

LaunchItemBackground {
    id: root
    applicationName: "com.pelagicore.media"
    Label {
        id: title
        z: 1
        text: "Music & TV"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: Style.paddingXXL
    }

    Image {
        id: image
        anchors.fill: root
        asynchronous: true
        source: musicControl.currentTrack ? musicControl.currentCover : ""
    }
    Image {
        source: Style.symbol("MusicOverlayGradient")
        anchors.bottom: parent.bottom
    }

    ApplicationIPCInterface {
        id: musicControl

        property var currentTrack
        property string currentCover: ""

        property string currentTime: "00:00"
        property string durationTime: "00:00"
        property bool playing: false

        signal previousTrack()
        signal nextTrack()
        signal play()
        signal pause()

        Component.onCompleted: {
            ApplicationIPCManager.registerInterface(musicControl, "com.pelagicore.music.control",
                                                    {})
        }
    }

    Behavior on scale {
        NumberAnimation {}
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Style.padding
        width: parent.width
        spacing: Style.padding

        Label {
            width: parent.width - 20
            anchors.horizontalCenter: parent.horizontalCenter
            maximumLineCount: 2
            text: musicControl.currentTrack ? musicControl.currentTrack.title : ""
            font.pixelSize: Style.fontSizeL
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: musicControl.currentTrack
                  ? qsTr('%1 / %2').arg(musicControl.currentTrack.artist).arg(musicControl.currentTrack.album)
                  : ""
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 120
        spacing: 90
        Tool {
            name: "video-previous"
            vspan: 2
            onClicked: musicControl.previousTrack()
        }
        Tool {
            vspan: 2
            name: active ? "video-pause" : "video-play"
            onClicked: active ? musicControl.pause() : musicControl.play()
            active: musicControl.playing
        }
        Tool {
            vspan: 2
            name: "video-next"
            onClicked: musicControl.nextTrack()
        }
    }
}
