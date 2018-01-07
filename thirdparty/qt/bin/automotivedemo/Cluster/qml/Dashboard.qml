/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

import QtQuick 2.7
import service.valuesource 1.0
import "gauges"

Image
{
    id: dashboardEntity
    source: "../images/Cluster8Gauges.png"
    anchors.fill: parent

    property bool parentVisible: parent.visible

    property real timeScaleMultiplier: 1.0
    property bool startupAnimationsFinished: false

    property alias bottomPanelY: bottompanel.y
    property alias needleRotation: tachometer.tachometerNeedleRotation

    onNeedleRotationChanged: speedometer.speedometerNeedleRotation = needleRotation / 40.

    /*rotation: angle

    property int xlight: 0
    property int ylight: -500
    property real angle: 0

    PropertyAnimation {
        target: dashboardEntity
        property: "angle"
        from: 0
        to: 360
        duration: 10000
        running: true
    }*/

    //
    // Fuelmeter
    //
    FuelMeter {}

    //
    // Batterymeter
    //
    BatteryMeter {}

    //
    // Consumptionmeter
    //
    ConsumptionMeter {}

    //
    // Temperaturemeter
    //
    TemperatureMeter {}

    //
    // Turbometer
    //
    TurboMeter {}

    //
    // Fpsmeter
    //
    FpsMeter {}

    //
    // Bottom Panel
    //
    BottomPanel {
        id: bottompanel
        y: 720
    }

    //
    // Gauge center pieces
    //
    Image {
        source: "../images/ClusterGaugeCenters.png"
        anchors.fill: parent
    }

    // These two must be after center pieces, as they print text on top of the image

    //
    // Speedometer
    //
    SpeedoMeter {
        id: speedometer
    }

    //
    // Tachometer
    //
    TachoMeter {
        id: tachometer
    }

    //
    // 2.5D illusion for the 2.5D dashboard
    //
    /*ShaderEffect {
        id: bumpShader
        anchors.fill: parent
        property vector3d lightPos: Qt.vector3d(xlight, ylight, xlight)
        property var source: ShaderEffectSource {
            sourceItem: dashboardEntity
        }
        property var bump: Image {
            source: "../images/Gauges_normal.png"
        }
        property real angle: dashboardEntity.angle * Math.PI / 180.0
        fragmentShader:
            "
            uniform sampler2D source;
            uniform sampler2D bump;
            varying vec2 qt_TexCoord0;
            uniform float qt_Opacity;
            uniform vec3 lightPos;
            uniform float angle;

            highp vec3 dbModel(const highp vec3 norm)
            {
                // Transform position, normal, and tangent to eye coords
                vec3 normal = normalize(vec3(0, 1, 0));
                vec3 tangent = normalize(vec3(1, 0, 1));
                vec3 position = vec3(960, -1000, 360);

                // Calculate binormal vector
                vec3 binormal = normalize(cross(normal, tangent));

                // Construct matrix to transform from eye coords to tangent space
                mat3 tangentMatrix = mat3 (
                    tangent.x, binormal.x, normal.x,
                    tangent.y, binormal.y, normal.y,
                    tangent.z, binormal.z, normal.z);

                // Transform light direction and view direction to tangent space
                vec3 s = lightPos - position;
                vec3 lightDir = normalize(tangentMatrix * s);

                vec3 v = -position;
                vec3 viewDir = normalize(tangentMatrix * v);

                float oldX = lightDir.x;
                lightDir.x = (lightDir.x * cos(angle) - lightDir.z * sin(angle));
                lightDir.z = (oldX * sin(angle) + lightDir.z * cos(angle));

                vec3 ka = vec3(1.0, 1.0, 1.0);
                vec3 ks = vec3(0.25, 0.25, 0.25);
                float shininess = 30.0;//100.0;

                // Reflection of light direction about normal
                vec3 r = reflect(-lightDir, norm);

                vec3 diffuseColor = texture2D(source, qt_TexCoord0).rgb;

                // Calculate the ambient contribution
                vec3 ambient = ka * diffuseColor;

                // Calculate the diffuse contribution
                float sDotN = max(dot(lightDir, norm), 0.0);
                vec3 diffuse = diffuseColor * sDotN;

                // Calculate the specular highlight contribution
                vec3 specular = vec3(0.0);
                if (sDotN > 0.0)
                    specular = ks * pow(max(dot(r, viewDir), 0.0), shininess);

                //return specular;
                //return ambient;
                //return ambient + diffuse;
                return ambient + specular;
                //return ambient + diffuse + specular;
            }

            void main()
            {
                vec4 normal = 2.0 * texture2D(bump, qt_TexCoord0) - vec4(1.0);
                vec3 result = dbModel(normalize(normal.xyz));
                gl_FragColor = qt_Opacity * vec4(result, 1.0);
            }
            "
    }

    SequentialAnimation {
        running: true
        loops: Animation.Infinite

        PropertyAnimation {
            target: dashboardEntity
            property: "xlight"
            from: 0
            to: dashboardEntity.width
            duration: 15000
            easing.type: Easing.InOutQuad
        }
        PropertyAnimation {
            target: dashboardEntity
            property: "xlight"
            from: dashboardEntity.width
            to: 0
            duration: 15000
            easing.type: Easing.InOutQuad
        }
        PropertyAnimation {
            target: dashboardEntity
            property: "ylight"
            from: -500
            to: 5000
            duration: 10000
            easing.type: Easing.InOutQuad
        }
        PropertyAnimation {
            target: dashboardEntity
            property: "ylight"
            from: 5000
            to: -500
            duration: 10000
            easing.type: Easing.InOutQuad
        }
    }*/

    //
    // Startup animations
    //
    SequentialAnimation {
        id: speedometerStartupAnimations
        running: false

        SmoothedAnimation {
            target: dashboardEntity
            property: "bottomPanelY"
            from: 720
            to: 603
            duration: 1000 * timeScaleMultiplier
            easing.type: Easing.InCirc
        }

        SmoothedAnimation {
            target: dashboardEntity
            property: "needleRotation"
            from: 0.0
            to: -8000.0
            duration: 1000 * timeScaleMultiplier
            easing.type: Easing.InCubic
        }

        SmoothedAnimation {
            target: dashboardEntity
            property: "needleRotation"
            from: -8000.0
            to: 0.0
            duration: 1000 * timeScaleMultiplier
            easing.type: Easing.OutCubic
        }

        ScriptAction {
            script: {
                startupAnimationsFinished = true
                ValueSource.startAnimations = true
            }
        }
    }

    // Just a hack for now; wait for the fader to finish
    onParentVisibleChanged: {
        if (visible)
            initTimer.start()
    }

    Timer {
        id: initTimer
        running: false
        interval: 1000
        onTriggered: speedometerStartupAnimations.start()
    }
}
