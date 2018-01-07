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

import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Extras 2.0
import QtQuick 2.7 as Quick
import com.qtcompany.clusterscenehelper 1.0

Entity
{
    id: carModelEntity

    property bool isReady: false
    property bool hidden: true
    property int width
    property int height
    property real carRotation: -30.0
    property var previousComponent: undefined
    property var previousMaterial
    property vector3d defaultCameraPosition: Qt.vector3d(0.0, 5.0, 25.0)
    property vector3d defaultLightPosition: Qt.vector3d(0.0, 20.0, 20.0)
    property vector3d lightPosition: defaultLightPosition
    property vector3d cameraPos: defaultCameraPosition
    property vector3d lightPos: defaultLightPosition

    property vector3d welcomeEndPosition: Qt.vector3d(-5.0, 5.0, 25.0)
    property vector3d lightEndPosition: Qt.vector3d(20.0, 40.0, -10.0)
    property vector3d statusEndPosition: Qt.vector3d(0.0, 40.0, 1.0)

    property real lightPosMultiplier: 1.75

    property int door_left: 1
    property int door_right: 2
    property int door_trunk: 4
    property int door_hood: 8

    property bool highlighting: false
    property bool doorAction: false

    property int highlightType: 0
    property int defaultHighlight: 99

    // Preset camera positions for highlights
    // Light positions can use the same vectors, but with a multiplier to move it further or closer
    // Lamp highlights
    property vector3d positionFrontLeftHigh: Qt.vector3d(5.0, 4.0, 15.0)       // Left headlight
    property vector3d positionFrontRightHigh: Qt.vector3d(-5.0, 4.0, 15.0)     // Right headlight
    property vector3d positionFrontLeftLow: Qt.vector3d(3.0, 2.0, 15.0)        // Left day light
    property vector3d positionFrontRightLow: Qt.vector3d(-3.0, 2.0, 15.0)      // Right day light
    property vector3d positionRearLeft: Qt.vector3d(5.0, 5.0, -15.0)           // Left tail light
    property vector3d positionRearRight: Qt.vector3d(-5.0, 5.0, -15.0)         // Right tail light
    // Tire highlights
    property vector3d positionLeftRear: Qt.vector3d(10.0, 2.0, -12.5)
    property vector3d positionLeftFront: Qt.vector3d(10.0, 2.0, 12.5)
    property vector3d positionRightRear: Qt.vector3d(-10.0, 2.0, -12.5)
    property vector3d positionRightFront: Qt.vector3d(-10.0, 2.0, 12.5)
    // Door highlights
    property vector3d positionLeft: Qt.vector3d(35.0, 10.0, 0.0)                // Doors on the left
    property vector3d positionRight: Qt.vector3d(-35.0, 10.0, 0.0)              // Doors on the right
    property vector3d positionTop: Qt.vector3d(0.0, 40.0, 1.0)                  // Doors on both sides
    property vector3d positionBack: Qt.vector3d(0.0, 20.0, -20.0)               // Trunk
    property vector3d positionFront: Qt.vector3d(0.0, 20.0, 20.0)               // Hood

    property color defaultBodyColor: "white"//Qt.rgba(0.5270588, 0.03137255, 0.03137255, 1.0)

    property int topViewAnimationDuration: 2500

    Camera {
        id: cameraCar
        projectionType: CameraLens.PerspectiveProjection
        fieldOfView: 45
        aspectRatio: carModelEntity.width / carModelEntity.height
        nearPlane: 0.1
        farPlane: 100.0
        position: defaultCameraPosition
        upVector: Qt.vector3d(0.0, 1.0, 0.0)
        viewCenter: Qt.vector3d(0.0, 0.0, 0.0)
    }

    Entity {
        components: [
            Transform {
                translation: lightPosition
            },
            PointLight {
                color: "white"
                intensity: 1.0
            }
        ]
    }

    RenderSettings {
        activeFrameGraph: TechniqueFilter {
            matchAll: [ FilterKey { name: "renderingStyle"; value: "forward" } ]
            id: carFramegraph

            RenderSurfaceSelector {
                id: surfaceSelector
                Viewport {
                    normalizedRect: Qt.rect(0.0, 0.0, 1.0, 1.0)

                    CameraSelector {
                        camera: cameraCar
                        NoDraw {
                            enabled: hidden
                        }
                        FrustumCulling {
                            ClearBuffers {
                                clearColor: "transparent"
                                buffers : ClearBuffers.ColorDepthBuffer
                            }
                        }
                    }
                }
            }
        }
    }

    // Materials for the parts that need highlighting
    PhongMaterial {
        id: bodyMaterial
        ambient: "black"
        diffuse: defaultBodyColor
        specular: Qt.rgba(0.7686275, 0.6196079, 0.3568628, 1.0)
        shininess: 164
    }

    PhongMaterial {
        id: bodyMaterialHighlight
        ambient: "black"
        diffuse: "orange"
        shininess: 164
    }

    PhongMaterial {
        id: tireMaterial
        ambient: Qt.rgba(0.05, 0.05, 0.05, 1.0)
        specular: Qt.rgba(0.594, 0.594, 0.594, 1.0)
        diffuse: "black"
        shininess: 51
    }

    PhongMaterial {
        id: tireMaterialHighlight
        ambient: "orange"
        specular: Qt.rgba(0.594, 0.594, 0.594, 1.0)
        diffuse: "black"
        shininess: 51
    }

    DiffuseMapMaterial {
        id: lampsMaterial
        ambient: Qt.rgba(0.3, 0.3, 0.3, 1.0)
        specular: Qt.rgba(0.279, 0.279, 0.279, 1.0)
        // Use with appman
        diffuse: "models/Map11.jpg"
        // Use if running standalone
        //diffuse: "qrc:/Map11.jpg"
        shininess: 31
    }

    // bodyMaterialHighlight is used for lamp highlight

    // Materials for the parts that do not otherwise work correctly
    Material {
        id: transparentGlassMaterial

        parameters: [
            Parameter { name: "alpha"; value: 0.95 },
            Parameter { name: "ka"; value: Qt.vector3d(0.0, 0.0, 0.0) },
            Parameter { name: "kd"; value: Qt.vector3d(0.1608937, 0.16512, 0.154057) },
            Parameter { name: "ks"; value: Qt.vector3d(1.0, 1.0, 1.0) },
            Parameter { name: "shininess"; value: 33 }
        ]

        effect: DefaultAlphaEffect {
            sourceRgbArg: BlendEquationArguments.SourceColor
            destinationRgbArg: BlendEquationArguments.OneMinusSourceColor
        }
    }

//    PhongAlphaMaterial {
//        id: transparentGlassMaterial
//        diffuse: Qt.rgba(0.1608937, 0.16512, 0.154057, 1.0)
//        specular: Qt.rgba(1.0, 1.0, 1.0, 1.0)
//        alpha: 0.75
//        shininess: 33
//    }

    PhongMaterial {
        id: interiorMaterial
        ambient: "black"
        diffuse: Qt.rgba(0.17, 0.17, 0.18, 1.0)
        shininess: 10
    }

    SceneHelper {
        id: sceneHelper
    }

    Entity {
        Transform {
            id: scenePosition
            matrix: {
                var m = Qt.matrix4x4()
                m.scale(1.15)
                return m
            }
        }

        Entity {
            id: carModel

            Transform {
                id: carTransform
                matrix: {
                    var m = Qt.matrix4x4()
                    m.rotate(carRotation, Qt.vector3d(0, 1, 0))
                    m.rotate(-90, Qt.vector3d(1, 0, 0))
                    return m
                }
            }

            SceneLoader {
                id: modelLoader
                source: "models/sportscar.qgltf" // Use with appman
                //source: "qrc:/sportscar.qgltf" // Use if running standalone
                property var lampParts: [ "headlight_right", "headlight_left", "daylight_right",
                    "daylight_left", "taillight_left", "taillight_right" ]
                property var bodyParts: [ "body", "door_left", "door_right",
                    "trunk", "hood" ]
                property var transparentGlassParts: [ "d_glass" ]
                property var tireParts: [ "tire_front_left", "tire_front_right",
                    "tire_rear_left", "tire_rear_right" ]
                property var interiorParts: [ "interior" ]

                // Note: If there are problems with transparent materials etc. check that you have
                // exported the Collada file used to create the qgltf binary files using the following
                // options in Blender (in Collada options category):
                // - Triangulate (off)
                // - Use Object Instances (on)
                // - Sort by Object name (on)
                // If just setting those is not enough, try changing the object names so that the
                // object will be loaded in a different order.
                // Use the following syntax for qgltf.exe:
                // qgltf.exe file.dae -b -S

                onStatusChanged: {
                    console.log("carmodel loading: " + status)
                    if (status === SceneLoader.Ready) {
                        console.log("carmodel loaded")
                        sceneHelper.addBasicMaterials(modelLoader, bodyMaterial, bodyParts)
                        sceneHelper.addBasicMaterials(modelLoader, transparentGlassMaterial,
                                                      transparentGlassParts)
                        sceneHelper.addBasicMaterials(modelLoader, interiorMaterial, interiorParts)
                        // Comment out if running standalone..
                        sceneHelper.addTextureMaterial(modelLoader, lampsMaterial, lampParts[0])
                        sceneHelper.addTextureMaterial(modelLoader, lampsMaterial, lampParts[1])
                        sceneHelper.addTextureMaterial(modelLoader, lampsMaterial, lampParts[2])
                        sceneHelper.addTextureMaterial(modelLoader, lampsMaterial, lampParts[3])
                        sceneHelper.addTextureMaterial(modelLoader, lampsMaterial, lampParts[4])
                        sceneHelper.addTextureMaterial(modelLoader, lampsMaterial, lampParts[5])
                        // ..Comment out if running standalone
                        sceneHelper.addBasicMaterials(modelLoader, tireMaterial, tireParts)
                        //floorPlane.enabled = true // Disable floor for now
                    }
                }
            }

            components: [carTransform, modelLoader]
        }

        Entity {
            id: floorPlane
            enabled: false

             DiffuseMapMaterial {
                id: planeMaterial
                ambient: Qt.rgba(0, 0, 0, 1.0)
                specular: Qt.rgba(0, 0, 0, 1.0)
                // Use with appman
                diffuse: "../images-optimized/SportCarFloorShadow.png"
                // Use if running standalone
                //diffuse: "qrc:/images/SportCarFloorShadow.png"
            }

            Transform {
                id: planeRotation
                matrix: {
                    var m = Qt.matrix4x4()
                    m.rotate(carRotation, Qt.vector3d(0, 1, 0))
                    return m
                }
            }

            PlaneMesh {
                id: planeMesh
                width: 70
                height: 70
            }

            components: [planeMesh, planeMaterial, planeRotation]
        }

        components: [ scenePosition ]
    }

    function highlightItem(idx) {
        carRotationAnimation.stop()
        carResetRotationAnimation.start()
        highlighting = true
        var highlightComponent
        var highlightMaterial
        var originalMaterial

        switch (idx) {
        case 1:
            highlightComponent = "tire_front_left"
            highlightMaterial = tireMaterialHighlight
            originalMaterial = tireMaterial
            lightPos = positionLeftFront.times(lightPosMultiplier)
            cameraPos = positionLeftFront
            break
        case 2:
            highlightComponent = "tire_front_right"
            highlightMaterial = tireMaterialHighlight
            originalMaterial = tireMaterial
            lightPos = positionRightFront.times(lightPosMultiplier)
            cameraPos = positionRightFront
            break
        case 3:
            highlightComponent = "tire_rear_right"
            highlightMaterial = tireMaterialHighlight
            originalMaterial = tireMaterial
            lightPos = positionRightRear.times(lightPosMultiplier)
            cameraPos = positionRightRear
            break
        case 4:
            highlightComponent = "tire_rear_left"
            highlightMaterial = tireMaterialHighlight
            originalMaterial = tireMaterial
            lightPos = positionLeftRear.times(lightPosMultiplier)
            cameraPos = positionLeftRear
            break
        case 5:
            highlightComponent = "headlight_left"
            highlightMaterial = bodyMaterialHighlight
            originalMaterial = lampsMaterial
            lightPos = positionFrontLeftHigh.times(lightPosMultiplier)
            cameraPos = positionFrontLeftHigh
            break
        case 6:
            highlightComponent = "headlight_right"
            highlightMaterial = bodyMaterialHighlight
            originalMaterial = lampsMaterial
            lightPos = positionFrontRightHigh.times(lightPosMultiplier)
            cameraPos = positionFrontRightHigh
            break
        case 7:
            highlightComponent = "daylight_right"
            highlightMaterial = bodyMaterialHighlight
            originalMaterial = lampsMaterial
            lightPos = positionFrontRightLow.times(lightPosMultiplier)
            cameraPos = positionFrontRightLow
            break
        case 8:
            highlightComponent = "daylight_left"
            highlightMaterial = bodyMaterialHighlight
            originalMaterial = lampsMaterial
            lightPos = positionFrontLeftLow.times(lightPosMultiplier)
            cameraPos = positionFrontLeftLow
            break
        case 9:
            highlightComponent = "taillight_left"
            highlightMaterial = bodyMaterialHighlight
            originalMaterial = lampsMaterial
            lightPos = positionRearLeft.times(lightPosMultiplier)
            cameraPos = positionRearLeft
            break
        case 10:
            highlightComponent = "taillight_right"
            highlightMaterial = bodyMaterialHighlight
            originalMaterial = lampsMaterial
            lightPos = positionRearRight.times(lightPosMultiplier)
            cameraPos = positionRearRight
            break
        default:
            lightPos = defaultLightPosition
            cameraPos = defaultCameraPosition
        }

        if (previousComponent !== undefined)
            sceneHelper.replaceMaterial(modelLoader, previousComponent, previousMaterial)

        if (highlightComponent !== undefined)
            sceneHelper.replaceMaterial(modelLoader, highlightComponent, highlightMaterial)

        previousComponent = highlightComponent
        previousMaterial = originalMaterial
    }

    function highlightOpenDoors(openDoors) {
        carRotationAnimation.stop()
        carResetRotationAnimation.start()
        highlighting = true
        var openLeft = false
        var openRight = false
        var openBack = false
        var openFront = false

        // Check with bitwise operators, as they can be open in any combination
        if (openDoors & door_left) {
            sceneHelper.replaceMaterial(modelLoader, "door_left", bodyMaterialHighlight)
            openLeft = true
        } else {
            sceneHelper.replaceMaterial(modelLoader, "door_left", bodyMaterial)
        }

        if (openDoors & door_right) {
            sceneHelper.replaceMaterial(modelLoader, "door_right", bodyMaterialHighlight)
            openRight = true
        } else {
            sceneHelper.replaceMaterial(modelLoader, "door_right", bodyMaterial)
        }

        if (openDoors & door_trunk) {
            sceneHelper.replaceMaterial(modelLoader, "trunk", bodyMaterialHighlight)
            openBack = true
        } else {
            sceneHelper.replaceMaterial(modelLoader, "trunk", bodyMaterial)
        }

        if (openDoors & door_hood) {
            openFront = true
            sceneHelper.replaceMaterial(modelLoader, "hood", bodyMaterialHighlight)
        } else {
            sceneHelper.replaceMaterial(modelLoader, "hood", bodyMaterial)
        }

        if (openRight && openLeft || openBack && openFront) {
            lightPos = positionTop.times(0.5)
            cameraPos = positionTop
        } else if (openRight) {
            lightPos = positionRight.times(0.33)
            lightPos.y += 15.0
            cameraPos = positionRight
        } else if (openLeft) {
            lightPos = positionLeft.times(0.33)
            lightPos.y += 15.0
            cameraPos = positionLeft
        } else if (openBack) {
            lightPos = positionBack.times(0.75)
            cameraPos = positionBack
        } else if (openFront) {
            lightPos = positionFront.times(1.0)
            cameraPos = positionFront
        } else {
            lightPos = defaultLightPosition
            cameraPos = defaultCameraPosition
        }
    }

    onCameraPosChanged: {
        // Update both camera and light positions
        cameraAnimation.to = cameraPos
        lightAnimation.to = lightPos
        cameraAnimation.restart()
        lightAnimation.restart()
    }

    Quick.PropertyAnimation {
        running: false
        id: cameraAnimation
        target: cameraCar
        property: "position"
        duration: topViewAnimationDuration
        easing.type: Easing.InOutQuad
    }

    Quick.PropertyAnimation {
        running: false
        id: lightAnimation
        target: carModelEntity
        property: "lightPosition"
        duration: topViewAnimationDuration * 4//1000
        easing.type: Easing.Linear
    }

    Quick.RotationAnimation on carRotation {
        id: carRotationAnimation
        running: false
        from: -30.0
        to: 180.0
        duration: topViewAnimationDuration
        easing.type: Easing.InOutQuad
    }

    function resetHighlight() {
        if (doorAction)
            highlightOpenDoors(0)
        else
            highlightItem(defaultHighlight)
        doorAction = false
    }

    function highlightLamp() {
        highlightType = Math.floor(Math.random() * 6) + 5
        highlightItem(highlightType)
        return highlightType
    }

    function highlightTire() {
        highlightType = Math.floor(Math.random() * 4) + 1
        highlightItem(highlightType)
        return highlightType
    }

    function startWelcomeAnimation() {
        cameraAnimation.duration = 1000
        cameraPos = welcomeEndPosition
        idleTimer.restart()
    }

    Quick.Timer {
        id: idleTimer
        interval: 2000
        onTriggered: {
            cameraAnimation.duration = topViewAnimationDuration
            lightPos = lightEndPosition
            cameraPos = statusEndPosition
            carRotationAnimation.restart()
        }
    }
}
