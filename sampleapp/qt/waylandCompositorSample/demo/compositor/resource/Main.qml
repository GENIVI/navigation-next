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
import QtWayland.Compositor 1.0
import comtechWaylandDemo 1.0

WaylandCompositor {
    id: demoCompositor
    property var lastItem: null
    property var lastItem2: null


    Screen {
        signal screenSizeChanged(double width,double height)
        id: screen
        compositor: demoCompositor
        onScreenSizeChanged: {
            //console.log("on screen size changed")

           if (lastItem != null) {
               //console.log("inside screen size changed")
               customExtension.compositorScreenSizeChanged(lastItem.shellSurface.surface,width,height)
           }
           if (lastItem2 != null) {
               lastItem2.y = screen.window.height/1.5
               //console.log("inside screen size changed")
               customExtension.compositorScreenSizeChanged(lastItem2.shellSurface.surface,width,height)
           }
        }
    }
    extensions: [
        WlShell {
            id: defaultShell

            onWlShellSurfaceCreated: {
                //var item = chromeComponent.createObject(defaultOutput.window, { "shellSurface": shellSurface } );
                if(lastItem == null){
                    var item = chromeComponent.createObject(screen.window, { "shellSurface": shellSurface } );
                    lastItem = item;
                    //screen.surfaceList.append(item);

                }else
                    {
                    var item2 = chromeComponent.createObject(screen.window, { "shellSurface": shellSurface } );
                    lastItem2 = item2
                    lastItem2.y = screen.window.height/1.5

                    //screen.surfaceList.append(item2);

                }
                //screen.surfaceList.append({"shellSurface": shellSurface});

            }

            Component.onCompleted: {
                console.log ("WL shell set up")
            }
        },
    /*WlShell {
        id: defaultShell
        onWlShellSurfaceCreated: {
            screen.surfaceList.append({"shellSurface": shellSurface});
        }
    }*/
    CustomExtension {
        id: customExtension
        signal changedScreenSize(double width,double height);
        onNotificationReceived: {
            screen.showNotification(text)
        }
    }]
    Component {
        id: chromeComponent
        ShellSurfaceItem {
            id: chrome
            onSurfaceDestroyed: {
                //console.log("surface destroyed")
                if (chrome === lastItem)
                    lastItem = null;
                chrome.destroy()
            }
            onShellSurfaceChanged: {
                //console.log("foo1" + shellSurface.className)
            }
            Component.onCompleted: {
                //console.log(chrome.shellSurface)
                //console.log("foo2" + shellSurface.className)
            }
            Connections {
                target: chrome.shellSurface
                onClassNameChanged: {
                    //console.log("baas1")
                }
            }
        }
    }
}
