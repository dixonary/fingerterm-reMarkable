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
import FingerTerm 1.0
import QtQuick.Window 2.0

Item {
    id: root

    width: 540
    height: 960

    Binding {
        target: util
        property: "windowOrientation"
        value: page.orientation
    }

    Binding {
        target: util
        property: "allowGestures"
        value: !vkb.active && !menu.showing && urlWindow.state != "visible" && aboutDialog.state != "visible"
               && layoutWindow.state != "visible"
    }

    Item {
        id: page

        property int orientation: forceOrientation ? forcedOrientation : Screen.orientation
        property bool forceOrientation: util.orientationMode != Util.OrientationAuto
        property int forcedOrientation: util.orientationMode == Util.OrientationLandscape ? Qt.LandscapeOrientation
                                                                                          : Qt.PortraitOrientation
        property bool portrait: rotation % 180 == 0

        width: portrait ? root.width : root.height
        height: portrait ? root.height : root.width
        anchors.centerIn: parent
        rotation: Screen.angleBetween(orientation, Screen.primaryOrientation)
        focus: true
        Keys.onPressed: {
            term.keyPress(event.key,event.modifiers,event.text);
        }

        Rectangle {
            id: window

            property string fgcolor: "black"
            property string bgcolor: "#000000"
            property int fontSize: 14*pixelRatio

            property int fadeOutTime: 80
            property int fadeInTime: 350
            property real pixelRatio: root.width / 540

            // layout constants
            property int buttonWidthSmall: 60*pixelRatio
            property int buttonWidthLarge: 180*pixelRatio
            property int buttonWidthHalf: 90*pixelRatio

            property int buttonHeightSmall: 48*pixelRatio
            property int buttonHeightLarge: 68*pixelRatio

            property int headerHeight: 20*pixelRatio

            property int radiusSmall: 5*pixelRatio
            property int radiusMedium: 10*pixelRatio
            property int radiusLarge: 15*pixelRatio

            property int paddingSmall: 5*pixelRatio
            property int paddingMedium: 10*pixelRatio

            property int fontSizeSmall: 14*pixelRatio
            property int fontSizeLarge: 24*pixelRatio

            property int uiFontSize: util.uiFontSize * pixelRatio

            property int scrollBarWidth: 6*window.pixelRatio

            anchors.fill: parent
            color: bellTimer.running ? "#ffffff" : bgcolor

            Lineview {
                id: lineView

                property int duration

                y: -(height+1)
                onFontPointSizeChanged: {
                    lineView.setPosition(vkb.active)
                }
            }

            Keyboard {
                id: vkb

                property bool visibleSetting: true

                y: parent.height-vkb.height
                visible: page.activeFocus && visibleSetting
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
                            textrender.mousePress(touchPoint.x, touchPoint.y);
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
                            textrender.mouseMove(touchPoint.x, touchPoint.y);
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
                                    util.dragMode == Util.DragSelect) {
                                if (vkb.active) {
                                    window.sleepVKB();
                                } else {
                                    window.wakeVKB();
                                }
                            }

                            //gestures c++ handler
                            textrender.mouseRelease(touchPoint.x, touchPoint.y);
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
                width: menuImg.width + 60*window.pixelRatio
                height: menuImg.height + 30*window.pixelRatio
                color: "transparent"
                opacity: 0.5
                Image {
                    anchors.centerIn: parent
                    id: menuImg
                    source: "qrc:/icons/menu.png"
                    height: sourceSize.height
                    width: sourceSize.width
                    scale: window.pixelRatio
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        menu.showing = true
                    }
                }
            }

            Image {
                // terminal buffer scroll indicator
                source: "qrc:/icons/scroll-indicator.png"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                visible: textrender.showBufferScrollIndicator
                scale: window.pixelRatio
            }

            TextRender {
                id: textrender

                property int duration
                property int cutAfter: height

                height: parent.height
                width: parent.width
                fontPointSize: util.fontSize

                Behavior on opacity {
                    NumberAnimation { duration: textrender.duration; easing.type: Easing.InOutQuad }
                }
                Behavior on y {
                    NumberAnimation { duration: textrender.duration; easing.type: Easing.InOutQuad }
                }

                onCutAfterChanged: {
                    // this property is used in the paint function, so make sure that the element gets
                    // painted with the updated value (might not otherwise happen because of caching)
                    textrender.redraw();
                }
            }

            Timer {
                id: fadeTimer

                interval: util.keyboardFadeOutDelay
                onTriggered: {
                    window.sleepVKB();
                }
            }

            Timer {
                id: bellTimer

                interval: 80
            }

            Connections {
                target: util
                onVisualBell: bellTimer.start()
                onNotify: {
                    textNotify.text = msg;
                    textNotifyAnim.enabled = false;
                    textNotify.opacity = 1.0;
                    textNotifyAnim.enabled = true;
                    textNotify.opacity = 0;
                }
            }

            MenuFingerterm {
                id: menu
                anchors.fill: parent
            }

            Text {
                // shows large text notification in the middle of the screen (for gestures)
                id: textNotify

                anchors.centerIn: parent
                color: "#ffffff"
                opacity: 0
                font.pointSize: 40*window.pixelRatio

                Behavior on opacity {
                    id: textNotifyAnim
                    NumberAnimation { duration: 500; }
                }
            }

            Rectangle {
                // visual key press feedback...
                // easier to work with the coordinates if it's here and not under keyboard element
                id: visualKeyFeedbackRect

                property string label

                visible: false
                radius: window.radiusSmall
                color: "#ffffff"

                Text {
                    color: "#000000"
                    font.pointSize: 34*window.pixelRatio
                    anchors.centerIn: parent
                    text: visualKeyFeedbackRect.label
                }
            }

            NotifyWin {
                id: aboutDialog

                property int termW
                property int termH

                text: {
                    var str = "<font size=\"+3\">FingerTerm " + util.versionString() + "</font><br>\n" +
                            "<font size=\"+1\">" +
                            "by Heikki Holstila &lt;<a href=\"mailto:heikki.holstila@gmail.com?subject=FingerTerm\">heikki.holstila@gmail.com</a>&gt;<br><br>\n\n" +
                            "Config files for adjusting settings are at:<br>\n" +
                            util.configPath() + "/<br><br>\n" +
                            "Documentation:<br>\n<a href=\"http://hqh.unlink.org/harmattan\">http://hqh.unlink.org/harmattan</a>"
                    if (termH != 0 && termW != 0) {
                        str += "<br><br>Current window title: <font color=\"gray\">" + util.windowTitle.substring(0,40) + "</font>"; //cut long window title
                        if(util.windowTitle.length>40)
                            str += "...";
                        str += "<br>Current terminal size: <font color=\"gray\">" + termW + "x" + termH + "</font>";
                        str += "<br>Charset: <font color=\"gray\">" + util.charset + "</font>";
                    }
                    str += "</font>";
                    return str;
                }
                onDismissed: util.showWelcomeScreen = false
            }

            NotifyWin {
                id: errorDialog
            }

            UrlWindow {
                id: urlWindow
            }

            LayoutWindow {
                id: layoutWindow
            }

            Connections {
                target: term
                onDisplayBufferChanged: window.displayBufferChanged()
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
                fadeTimer.restart();
                vkb.active = true;
                lineView.setPosition(vkb.active);
                setTextRenderAttributes();
            }

            function sleepVKB()
            {
                textrender.duration = window.fadeInTime;
                lineView.duration = window.fadeInTime;
                vkb.active = false;
                lineView.setPosition(vkb.active);
                setTextRenderAttributes();
            }

            function setTextRenderAttributes()
            {
                if (util.keyboardMode == Util.KeyboardMove)
                {
                    vkb.visibleSetting = true;
                    textrender.opacity = 1.0;
                    if(vkb.active) {
                        var move = textrender.cursorPixelPos().y + textrender.fontHeight/2
                                + textrender.fontHeight*util.extraLinesFromCursor
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
                else if (util.keyboardMode == Util.KeyboardFade)
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
                lineView.lines = term.printableLinesFromCursor(util.extraLinesFromCursor);
                lineView.cursorX = textrender.cursorPixelPos().x;
                lineView.cursorWidth = textrender.cursorPixelSize().width;
                lineView.cursorHeight = textrender.cursorPixelSize().height;
                setTextRenderAttributes();
            }

            Component.onCompleted: {
                if (util.showWelcomeScreen)
                    aboutDialog.state = "visible"
                if (startupErrorMessage != "") {
                    showErrorMessage(startupErrorMessage)
                }
            }

            function showErrorMessage(string)
            {
                errorDialog.text = "<font size=\"+2\">" + string + "</font>";
                errorDialog.state = "visible"
            }
        }
    }
}
