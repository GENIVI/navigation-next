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

pragma Singleton
import QtQuick 2.1
import QtApplicationManager 1.0
import QtQuick.Window 2.2

QtObject {
    id: root

    property bool withCluster
    property bool withPassengerView
    property int paddingXS
    property int paddingS
    property int padding
    property int paddingL
    property int paddingXL
    property int paddingXXL

    property int screenWidth: 1080//Screen.width
    property int screenHeight: 1920//Screen.height
    property int bottomBarWidth: screenWidth
    property int bottomBarHeight: 270
    property int launchBarHeight: 512
    property int statusBarHeight: 100
    property int searchInputHeight: 50

    property int cellWidth
    property int cellHeight
    property int tumblerWidth
    property int buttonRadius

    property string fontFamily
    property real fontWeight
    property int fontSizeXXS
    property int fontSizeXS
    property int fontSizeS
    property int fontSizeM
    property int fontSizeL
    property int fontSizeXL
    property int fontSizeXXL

    property color colorWhite
    property color colorOrange
    property color colorGrey
    property color colorBlack
    property color colorPink

    property color textcolor: '#ffffff'

    property color darkGrey: '#101A1A'
    property bool debugMode
    property bool fakeBackground
    property string displayBackground
    property real disabledIconOpacity

    property int defaultSymbolSize
    property int defaultGfxSize
    property int symbolSizeXS
    property int symbolSizeS
    property int symbolSizeM
    property int symbolSizeL

    property bool clusterWidgetReady: false

    property url drawableUrl: Qt.resolvedUrl('../assets/drawable-ldpi')
    property url symbolUrl: Qt.resolvedUrl('../assets/icons')
    property url gfxUrl: Qt.resolvedUrl('../assets/gfx/')
    property url fonts: Qt.resolvedUrl('../assets/fonts/')

    property bool isClient: typeof ApplicationInterface !== 'undefined'
    property string styleConfig: isClient ? ApplicationInterface.additionalConfiguration.styleConfig : ApplicationManager.additionalConfiguration.styleConfig

    property Loader styleLoader: Loader {
        source: styleConfig === "auto" ? Qt.resolvedUrl("AutoConfig.qml") : styleConfig

        onLoaded: {
            print("StyleConfig loaded: ", source)
            withCluster = configValue("withCluster", true)
            withPassengerView = configValue("withPassengerView", false)
            paddingXS = configValue("paddingXS", 2)
            paddingS = configValue("paddingS", 4)
            padding = configValue("padding", 10)
            paddingL = configValue("paddingL", 12)
            paddingXL = configValue("paddingXL", 16)
            paddingXXL = configValue("paddingXL", 20)

            cellWidth = configValue("cellWidth", 53)
            cellHeight = configValue("cellHeight", 33)
            tumblerWidth = configValue("tumblerWidth", 150)
            buttonRadius = configValue("buttomRadius", 5)

            fontFamily = configValue("fontFamily", true ? 'Lato' : fontRegular.name)
            fontWeight = configValue("fontWeight", Font.Light)
            fontSizeXXS = configValue("fontSizeXXS", 12)
            fontSizeXS = configValue("fontSizeXS", 18)
            fontSizeS = configValue("fontSizeS", 24)
            fontSizeM = configValue("fontSizeM", 28)
            fontSizeL = configValue("fontSizeL", 38)
            fontSizeXL = configValue("fontSizeXL", 60)
            fontSizeXXL = configValue("fontSizeXXL", 80)

            colorWhite = configValue("colorWhite", '#ffffff')
            colorOrange = configValue("colorOrange", '#f07d00')
            colorGrey = configValue("colorGrey", '#999999')
            colorBlack = configValue("colorBlack", '#000000')
            colorPink = configValue("colorPink", '#EF0080')

            debugMode = configValue("debugMode", false)
            fakeBackground = configValue("fakeBackground", false)
            displayBackground = configValue("displayBackground", "background_1280x800")
            disabledIconOpacity = configValue("disabledIconOpacity", 0.6)

            defaultSymbolSize = configValue("defaultSymbolSize", symbolSizeS)
            defaultGfxSize = configValue("defaultGfxSize", 1)
            symbolSizeXS = configValue("symbolSizeXS", 32)
            symbolSizeS = configValue("symbolSizeS", 48)
            symbolSizeL = configValue("symbolSizeL", 96)
            symbolSizeM = configValue("symbolSizeM", 72)
        }
    }

    property FontLoader fontRegular: FontLoader {
        source: font('Lato-Regular')
    }

    property FontLoader fontLight: FontLoader {
        source: font('Lato-Light')
    }

    function configValue(key, defaultValue) {
        //console.log("configValue: " + key + " = ")
        //console.log(styleLoader.item[key] ? styleLoader.item[key] : defaultValue)
        return styleLoader.item[key] ? styleLoader.item[key] : defaultValue
    }

    function symbol(name, active) {
        return symbolUrl + '/' + name + '.png'
    }

    function symbolXS(name, active) {
        return symbol(name, symbolSizeXS, active);
    }

    function symbolS(name, active) {
        return symbol(name, symbolSizeS, active);
    }

    function symbolM(name, active) {
        return symbol(name, symbolSizeM, active);
    }

    function symbolL(name, active) {
        return symbol(name, symbolSizeL, active);
    }

    function symbolXL(name, active) {
        return symbol(name, symbolSizeXL, active);
    }

    function symbolXXL(name, active) {
        return symbol(name, symbolSizeXXL, active);
    }


    function gfx2(name) {
        return gfxUrl + name + '.png'
    }

    function gfx2Dynamic(name, size) {
        return gfxUrl + name + '@' + size + 'x.png'
    }

    function icon(name) {
        return drawableUrl + '/' + name + '.png';
    }

    function gfx(name) {
        return drawableUrl + '/' + name + '.png';
    }

    function hspan(value) {
        return cellWidth * value
    }

    function vspan(value) {
        return cellHeight * value
    }

    function asset(name) {
        return Qt.resolvedUrl('../assets/' + name)
    }

    function font(name) {
        return Qt.resolvedUrl('../assets/fonts/' + name + '.ttf')
    }
}
