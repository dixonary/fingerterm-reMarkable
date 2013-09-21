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

Rectangle {
    id: key
    property string label: ""
    property string label_alt: ""
    property int code: 0
    property int code_alt: 0
    property int currentCode: code
    property string currentLabel: keyLabel.text
    property bool sticky: false     // can key be stickied?
    property bool becomesSticky: false // will this become sticky after release?
    property int stickiness: 0      // current stickiness status

    // mouse input handling
    property int clickThreshold: 20
    property bool isClick: false
    property int pressMouseY: 0
    property int pressMouseX: 0

    width: window.width/12   // some default
    height: window.height/8 < 55 ? window.height/8 : 55
    color: label=="" ? "transparent" : keyboard.keyBgColor
    border.color: label=="" ? "transparent" : keyboard.keyBorderColor
    border.width: 1
    radius: 5

    Connections {
        target: keyboard
        onKeyModifiersChanged: {
            if( (keyboard.keyModifiers & Qt.ShiftModifier) && !sticky ) {
                if(key.label_alt!="") {
                    keyLabel.text = key.label_alt
                    keyAltLabel.text = key.label
                    key.currentCode = key.code_alt
                } else if(key.label.length==1) {
                    keyLabel.text = key.label.toUpperCase()
                }
            } else if(!sticky) {
                if(key.label.length==1)
                    keyLabel.text = key.label.toLowerCase()
                else
                    keyLabel.text = key.label

                keyAltLabel.text = key.label_alt
                key.currentCode = key.code
            }
        }
    }

    Image {
        id: keyImage
        anchors.centerIn: parent
        opacity: 0.4
        source: { if(key.label.length>1 && key.label.charAt(0)==':') return "qrc:/icons/"+key.label.substring(1)+".png"; else return ""; }
    }

    Text {
        visible: keyImage.source == ""
        id: keyLabel
        anchors.centerIn: parent
        text: key.label
        color: keyboard.keyFgColor
        font.pointSize: key.label.length>1 ? 18 : 26
    }
    Text {
        id: keyAltLabel
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        text: key.label_alt
        color: keyboard.keyFgColor
        font.pointSize: key.label.length>1 ? 10 : 13
    }
    Rectangle {
        id: stickIndicator
        visible: sticky && stickiness>0
        color: keyboard.keyHilightBgColor
        anchors.fill: parent
        radius: key.radius
        opacity: 0.5
        z: 1
        anchors.topMargin: key.height/2
    }

    function handlePress(touchArea, x, y) {
        isClick = true;
        pressMouseX = x;
        pressMouseY = y;

        key.color = keyboard.keyHilightBgColor
        keyboard.currentKeyPressed = key;
        util.keyPressFeedback();

        keyRepeatStarter.start();

        if (sticky) {
            keyboard.keyModifiers |= code;
            key.becomesSticky = true;
            keyboard.currentStickyPressed = key;
        } else {
            if (keyboard.currentStickyPressed != null) {
                // Pressing a non-sticky key while a sticky key is pressed:
                // the sticky key will not become sticky when released
                keyboard.currentStickyPressed.becomesSticky = false;
            }
        }
    }

    function handleMove(touchArea, x, y) {
        var mappedPoint = key.mapFromItem(touchArea, x, y)
        if (!key.contains(Qt.point(mappedPoint.x, mappedPoint.y))) {
            key.handleRelease(touchArea, x, y);
            return false;
        }

        if (key.isClick) {
            if (Math.abs(x - key.pressMouseX) > key.clickThreshold ||
            Math.abs(y - key.pressMouseY) > key.clickThreshold )
            key.isClick = false
        }

        return true;
    }

    function handleRelease(touchArea, x, y) {
        keyRepeatStarter.stop();
        keyRepeatTimer.stop();
        key.color = keyboard.keyBgColor;
        keyboard.currentKeyPressed = 0;

        // Wake up the keyboard if the user has tapped/clicked on it and we're not in select mode
        //(or it would be hard to select text)
        if (y < vkb.y && key.pressMouseY < vkb.y && util.settingsValue("ui/dragMode") !== "select") {
            if (vkb.active)
                window.sleepVKB();
            else
                window.wakeVKB();
        }

        if (sticky) {
            keyboard.keyModifiers &= ~code
            keyboard.currentStickyPressed = null;
        }

        if (vkb.keyAt(x, y) == key) {
            util.keyReleaseFeedback();

            if (key.sticky && key.becomesSticky) {
                setStickiness(-1);
            }

            window.vkbKeypress(currentCode, keyboard.keyModifiers);

            // first non-sticky press will cause the sticky to be released
            if( !sticky && keyboard.resetSticky != 0 && keyboard.resetSticky !== key ) {
                resetSticky.setStickiness(0);
            }
        }
    }

    Timer {
        id: keyRepeatStarter
        running: false
        repeat: false
        interval: 400
        triggeredOnStart: false
        onTriggered: {
            keyRepeatTimer.start();
        }
    }

    Timer {
        id: keyRepeatTimer
        running: false
        repeat: true
        triggeredOnStart: true
        interval: 80
        onTriggered: {
            window.vkbKeypress(currentCode, keyboard.keyModifiers);
        }
    }

    function setStickiness(val)
    {
        if(sticky) {
            if( keyboard.resetSticky != 0 && keyboard.resetSticky !== key ) {
                resetSticky.setStickiness(0)
            }

            if(val===-1)
                stickiness = (stickiness+1) % 3
            else
                stickiness = val

            // stickiness == 0 -> not pressed
            // stickiness == 1 -> release after next keypress
            // stickiness == 2 -> keep pressed

            if(stickiness>0) {
                keyboard.keyModifiers |= code
            } else {
                keyboard.keyModifiers &= ~code
            }

            keyboard.resetSticky = 0

            if(stickiness==1) {
                stickIndicator.anchors.topMargin = key.height/2
                keyboard.resetSticky = key
            } else if(stickiness==2) {
                stickIndicator.anchors.topMargin = 0
            }
        }
    }
}
