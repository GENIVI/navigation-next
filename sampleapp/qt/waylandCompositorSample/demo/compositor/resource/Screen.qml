// Copyright 2017  Andreas Cord-Landwehr <cordlandwehr@kde.org>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import QtQuick 2.0
import QtQuick.Window 2.2
import QtWayland.Compositor 1.0

WaylandOutput {
    id: output
    property alias surfaceList: listModel
    function showNotification(message) {
        notificationText.text = message
        notificationArea.visible = true
        notificationTimer.start()
    }

    sizeFollowsWindow: true

    window: Window {
        id: screen
        width: 768
        height: 1024
        visible: true
        onWidthChanged: {
            screenSizeChanged(screen.width,screen.height)
        }
        onHeightChanged: {
            screenSizeChanged(screen.width,screen.height)
        }

        WaylandMouseTracker {
            id: mouseTracker
            anchors.fill: parent
            windowSystemCursorEnabled: true
            Rectangle {
                anchors.fill: parent
                color: "darkgrey"
                ListView {
                    id: listView
                    anchors.fill: parent
                    model: ListModel {
                        id: listModel
                    }
                    orientation: ListView.Vertical
                    highlightFollowsCurrentItem: true
                    highlightRangeMode: ListView.StrictlyEnforceRange
                    snapMode: ListView.SnapToItem
                    delegate: ShellSurfaceItem {
                        shellSurface: model.shellSurface
                        onSurfaceDestroyed: {
                            listModel.remove(index)
                        }
                    }
                }
                ListView {
                    id: navigationInfo
                    height: 20
                    width: count * 20 + (count - 1) * 5
                    anchors {
                        bottom: parent.bottom
                        bottomMargin: 10
                        right: parent.right
                        rightMargin: 10
                    }
                    model: listView.count
                    spacing: 5
                    orientation: ListView.Horizontal
                    interactive: false
                    delegate: Rectangle {
                        width: 20
                        height: 20
                        radius: 10
                        opacity: 0.7
                        anchors.verticalCenter: parent.verticalCenter
                        color: (index === listView.currentIndex || index === listView.currentIndex + 1) ? "steelblue" : "white"
                    }
                }

                Rectangle {
                    id: notificationArea
                    width: parent.width
                    height: 0.33 * parent.height
                    color: "#3daee9"
                    opacity: 0.7
                    visible: false
                    Text {
                        id: notificationText
                        anchors.centerIn: parent
                        font.pointSize: 24
                    }
                    Timer {
                        id: notificationTimer
                        interval: 3000 // 3 seconds
                        onTriggered: notificationArea.visible = false
                    }
                }
            }

            WaylandCursorItem {
                inputEventsEnabled: false
                x: mouseTracker.mouseX - hotspotX
                y: mouseTracker.mouseY - hotspotY
                seat: output.compositor.defaultSeat
            }
        }
    }
}
