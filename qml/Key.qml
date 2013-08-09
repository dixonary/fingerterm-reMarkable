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
    property int stickiness: 0      // current stickiness status

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

    MouseArea {
        enabled: label!=""
        id: keyMouseArea
        anchors.fill: parent
        anchors.margins: -3  // -(half of keyspacing)
        onExited: {
            keyRepeatStarter.stop();
            keyRepeatTimer.stop();

            key.color = keyboard.keyBgColor
            keyboard.currentKeyPressed = 0;
        }
        onPressedChanged: {
            if(pressed) {
                key.color = keyboard.keyHilightBgColor
                keyboard.currentKeyPressed = key;
                util.keyPressFeedback();

                keyRepeatStarter.start();
            } else {
                keyboard.currentKeyPressed = 0;
                if(containsMouse) {

                    util.keyReleaseFeedback();

                    keyRepeatStarter.stop();
                    keyRepeatTimer.stop();

                    key.color = keyboard.keyBgColor

                    setStickiness(-1)
                    window.vkbKeypress(currentCode, keyboard.keyModifiers);

                    if( !sticky && keyboard.resetSticky != 0 && keyboard.resetSticky !== key ) {
                        resetSticky.setStickiness(0);
                    }
                }
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
            vkbKeypress(currentCode, keyboard.keyModifiers);
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
