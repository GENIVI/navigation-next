/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.7

Item {
    property string position: "right"
    property alias text: statusNoteText.text
    property string icon
    property string highlight

    Column {
        spacing: 5

        Row {
            id: statusNoteRow
            spacing: 5

            Image {
                id: statusNotePointerImage
                height: 66
                fillMode: Image.Pad
                source: position === "left" ? icon : highlight
                anchors.verticalCenter: statusNoteImage.verticalCenter
            }

            Rectangle {
                id: statusNoteLineImage
                height: 2
                width: 100
                color: "gray"
                anchors.verticalCenter: statusNoteImage.verticalCenter
            }

            Image {
                id: statusNoteImage
                height: 66
                fillMode: Image.Pad
                source: position === "right" ? icon : highlight
            }
        }

        Text {
            id: statusNoteText
            anchors.right: position === "right" ? statusNoteRow.right : undefined
            anchors.left: position === "left" ? statusNoteRow.left : undefined
            style: Text.RichText
            font.pixelSize: 14
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
