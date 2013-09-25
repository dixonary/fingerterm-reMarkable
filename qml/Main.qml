/*
    Copyright 2011-2012 Heikki Holstila <heikki.holstila@gmail.com>

    This file is part of FingerTerm.

    FingerTerm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    FingerTerm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FingerTerm.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.0
import TextRender 1.0
import QtQuick.Window 2.0
import com.nokia.meego 2.0

PageStackWindow {
    id: pageStackWindow

    focus: true

    Keys.onPressed: {
        window.vkbKeypress(event.key,event.modifiers);
    }

    initialPage:Page {
        id: page
        anchors.fill: parent

        orientationLock: window.getOrientationLockMode()

        Rectangle {
        property string fgcolor: "black"
        property string bgcolor: "#000000"
        property int fontSize: 14

        property int fadeOutTime: 80
        property int fadeInTime: 350

        property string windowTitle: util.currentWindowTitle();

        anchors.fill: parent

        id: window
        objectName: "window"
        color: bgcolor

        NotifyWin {
            id: aboutDialog
            property int termW: 0
            property int termH: 0
            text: {
                var str = "<font size=\"+3\">FingerTerm " + util.versionString() + "</font><br>\n" +
                        "<font size=\"+1\">" +
                        "by Heikki Holstila &lt;<a href=\"mailto:heikki.holstila@gmail.com?subject=FingerTerm\">heikki.holstila@gmail.com</a>&gt;<br><br>\n\n" +
                        "Config files for adjusting settings are at:<br>\n" +
                        util.configPath() + "/<br><br>\n" +
                        "Documentation:<br>\n<a href=\"http://hqh.unlink.org/harmattan\">http://hqh.unlink.org/harmattan</a>"
                if (termH != 0 && termW != 0) {
                    str += "<br><br>Current window title: <font color=\"gray\">" + windowTitle.substring(0,40) + "</font>"; //cut long window title
                    if(windowTitle.length>40)
                        str += "...";
                    str += "<br>Current terminal size: <font color=\"gray\">" + termW + "x" + termH + "</font>";
                    str += "<br>Charset: <font color=\"gray\">" + util.settingsValue("terminal/charset") + "</font>";
                }
                str += "</font>";
                return str;
            }
            onDismissed: {
                util.setSettingsValue("state/showWelcomeScreen", false);
            }
            z: 1001
        }

        NotifyWin {
            id: errorDialog
            text: ""
            z: 1002
        }

        UrlWindow {
            id: urlWindow
            z: 1000
        }

        LayoutWindow {
            id: layoutWindow
            z: 1000
        }


        Lineview {
            id: lineView
            x: 0
            y: -(height+1)
            z: 20
            property int duration: 0;
            onFontPointSizeChanged: {
                lineView.setPosition(vkb.active)
            }
        }

        Keyboard {
            id: vkb
            property bool visibleSetting: true
            x: 0
            y: parent.height-vkb.height
            visible: windowHasFocus && visibleSetting
        }

        // area that handles gestures/select/scroll modes and vkb-keypresses
        MultiPointTouchArea {
            id: multiTouchArea
            anchors.fill: parent

            property int firstTouchId: -1
            property var pressedKeys: ({})

            onPressed: {
                touchPoints.forEach(function (touchPoint) {
                    if (multiTouchArea.firstTouchId == -1) {
                        multiTouchArea.firstTouchId = touchPoint.pointId;

                        //gestures c++ handler
                        util.mousePress(touchPoint.x, touchPoint.y);
                    }

                    var key = vkb.keyAt(touchPoint.x, touchPoint.y);
                    if (key != null) {
                        key.handlePress(multiTouchArea, touchPoint.x, touchPoint.y);
                    }
                    multiTouchArea.pressedKeys[touchPoint.pointId] = key;
                });
            }
            onUpdated: {
                touchPoints.forEach(function (touchPoint) {
                    if (multiTouchArea.firstTouchId == touchPoint.pointId) {
                        //gestures c++ handler
                        util.mouseMove(touchPoint.x, touchPoint.y);
                    }

                    var key = multiTouchArea.pressedKeys[touchPoint.pointId];
                    if (key != null) {
                        if (!key.handleMove(multiTouchArea, touchPoint.x, touchPoint.y)) {
                            delete multiTouchArea.pressedKeys[touchPoint.pointId];
                        }
                    }
                });
            }
            onReleased: {
                touchPoints.forEach(function (touchPoint) {
                    if (multiTouchArea.firstTouchId == touchPoint.pointId) {
                        // Toggle keyboard wake-up when tapping outside the keyboard, but:
                        //   - only when not scrolling (y-diff < 20 pixels)
                        //   - not in select mode, as it would be hard to select text
                        if (touchPoint.y < vkb.y && touchPoint.startY < vkb.y &&
                                Math.abs(touchPoint.y - touchPoint.startY) < 20 &&
                                util.settingsValue("ui/dragMode") !== "select") {
                            if (vkb.active) {
                                window.sleepVKB();
                            } else {
                                window.wakeVKB();
                            }
                        }

                        //gestures c++ handler
                        util.mouseRelease(touchPoint.x, touchPoint.y);
                        multiTouchArea.firstTouchId = -1;
                    }

                    var key = multiTouchArea.pressedKeys[touchPoint.pointId];
                    if (key != null) {
                        key.handleRelease(multiTouchArea, touchPoint.x, touchPoint.y);
                    }
                    delete multiTouchArea.pressedKeys[touchPoint.pointId];
                });
            }
        }

        Rectangle {
            //top right corner menu button
            x: window.width - width
            y: 0
            z: 1
            width: menuImg.width + 60
            height: menuImg.height + 30
            color: "transparent"
            opacity: 0.5
            Image {
                anchors.centerIn: parent
                id: menuImg
                source: "qrc:/icons/menu.png"
                height: sourceSize.height
                width: sourceSize.width
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    menu.showMenu();
                }
            }
        }

        Image {
            // terminal buffer scroll indicator
            source: "qrc:/icons/scroll-indicator.png"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            visible: textrender.showBufferScrollIndicator
            z: 5
        }

        MenuFingerterm {
            id: menu
            x: window.width-width
            y: 0
        }

        TextRender {
            id: textrender
            objectName: "textrender"
            x: 0
            y: 0
            height: parent.height
            width: parent.width
            myWidth: width
            myHeight: height
            opacity: 1.0
            property int duration: 0;
            property int cutAfter: height

            Behavior on opacity {
                NumberAnimation { duration: textrender.duration; easing.type: Easing.InOutQuad }
            }
            Behavior on y {
                NumberAnimation { duration: textrender.duration; easing.type: Easing.InOutQuad }
            }

            onFontSizeChanged: {
                lineView.fontPointSize = textrender.fontPointSize;
            }

            onCutAfterChanged: {
                // this property is used in the paint function, so make sure that the element gets
                // painted with the updated value (might not otherwise happen because of caching)
                textrender.redraw();
            }

            z: 10
        }

        Timer {
            id: fadeTimer
            running: false
            repeat: false
            interval: menu.keyboardFadeOutDelay
            onTriggered: {
                window.sleepVKB();
            }
        }

        Timer {
            id: bellTimer
            running: false
            repeat: false
            interval: 80
            onTriggered: {
                window.color = window.bgcolor;
            }
        }

        Connections {
            target: util
            onVisualBell: {
                window.visualBell();
            }
            onGestureNotify: {
                textNotify.text = msg;
                textNotifyAnim.enabled = false;
                textNotify.opacity = 1.0;
                textNotifyAnim.enabled = true;
                textNotify.opacity = 0;
            }
            onWindowTitleChanged: {
                windowTitle = util.currentWindowTitle();
            }
        }

        Text {
            // shows large text notification in the middle of the screen (for gestures)
            id: textNotify
            anchors.centerIn: parent
            color: "#ffffff"
            z: 100
            opacity: 0
            text: ""
            font.pointSize: 40
            Behavior on opacity {
                id: textNotifyAnim
                NumberAnimation { duration: 500; }
            }
        }

        Rectangle {
            // visual key press feedback...
            // easier to work with the coordinates if it's here and not under keyboard element
            id: visualKeyFeedbackRect
            visible: false
            x: 0
            y: 0
            z: 200
            width: 0
            height: 0
            radius: 5
            color: "#ffffff"
            property string label: ""
            Text {
                color: "#000000"
                font.pointSize: 34
                anchors.centerIn: parent
                text: visualKeyFeedbackRect.label
            }
        }

        function vkbKeypress(key,modifiers) {
            wakeVKB();
            term.keyPress(key,modifiers);
        }

        function wakeVKB()
        {
            if(!vkb.visibleSetting)
                return;

            lineView.duration = window.fadeOutTime;
            textrender.duration = window.fadeOutTime;
            vkb.keyFgColor = "#cccccc";
            fadeTimer.restart();
            vkb.active = true;
            lineView.setPosition(vkb.active);
            util.updateSwipeLock(!vkb.active);
            setTextRenderAttributes();
            updateGesturesAllowed();
        }

        function sleepVKB()
        {
            textrender.duration = window.fadeInTime;
            lineView.duration = window.fadeInTime;
            vkb.keyFgColor = "#565656";
            vkb.active = false;
            lineView.setPosition(vkb.active);
            util.updateSwipeLock(!vkb.active);
            setTextRenderAttributes();
            updateGesturesAllowed();
        }

        function setTextRenderAttributes()
        {
            if(util.settingsValue("ui/vkbShowMethod")==="move")
            {
                vkb.visibleSetting = true;
                textrender.opacity = 1.0;
                if(vkb.active) {
                    var move = textrender.cursorPixelPos().y + textrender.fontHeight/2 + textrender.fontHeight*util.settingsValue("ui/showExtraLinesFromCursor");
                    if(move < vkb.y) {
                        textrender.y = 0;
                        textrender.cutAfter = vkb.y;
                    } else {
                        textrender.y = 0 - move + vkb.y
                        textrender.cutAfter = move;
                    }
                } else {
                    textrender.cutAfter = textrender.height;
                    textrender.y = 0;
                }
            }
            else if(util.settingsValue("ui/vkbShowMethod")==="fade")
            {
                vkb.visibleSetting = true;
                textrender.cutAfter = textrender.height;
                textrender.y = 0;
                if(vkb.active)
                    textrender.opacity = 0.3;
                else
                    textrender.opacity = 1.0;
            }
            else // "off" (vkb disabled)
            {
                vkb.visibleSetting = false;
                textrender.cutAfter = textrender.height;
                textrender.y = 0;
                textrender.opacity = 1.0;
            }
        }

        function displayBufferChanged()
        {
            lineView.lines = term.printableLinesFromCursor(util.settingsValue("ui/showExtraLinesFromCursor"));
            lineView.cursorX = textrender.cursorPixelPos().x;
            lineView.cursorWidth = textrender.cursorPixelSize().width;
            lineView.cursorHeight = textrender.cursorPixelSize().height;
            setTextRenderAttributes();
        }

        Component.onCompleted: {
            util.updateSwipeLock(vkb.active)
            if( util.settingsValue("state/showWelcomeScreen") === true )
                aboutDialog.state = "visible";
        }

        function showErrorMessage(string)
        {
            errorDialog.text = "<font size=\"+2\">" + string + "</font>";
            errorDialog.state = "visible"
        }

        function visualBell()
        {
            bellTimer.start();
            window.color = "#ffffff"
        }

        function updateGesturesAllowed()
        {
            if(vkb.active || menu.showing || urlWindow.state=="visible" ||
                    aboutDialog.state=="visible" || layoutWindow.state=="visible")
                util.allowGestures = false;
            else
                util.allowGestures = true;
        }

        function lockModeStringToQtEnum(stringMode) {
            switch (stringMode) {
            case "auto":
                return PageOrientation.Automatic
            case "landscape":
                return PageOrientation.LockLandscape
            case "portrait":
                return PageOrientation.LockPortrait
            }
        }

        function getOrientationLockMode()
        {
            var stringMode = util.settingsValue("ui/orientationLockMode");
            page.orientationLock = lockModeStringToQtEnum(stringMode)
        }

        function setOrientationLockMode(stringMode)
        {
            util.setSettingsValue("ui/orientationLockMode", stringMode);
            page.orientationLock = lockModeStringToQtEnum(stringMode)
        }
    }
    }

}
