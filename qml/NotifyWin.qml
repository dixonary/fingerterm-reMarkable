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
    id: notifyWin

    property string text: ""

    width: window.width-1
    height: window.height-1
    color: "#000000"
    z: 100
    y: -(height+1)
    state: ""
    border.color: "#c0c0c0"
    border.width: 1
    radius: 10

    signal dismissed();

    MouseArea {
        // event eater
        anchors.fill: parent
    }
    Rectangle {
        color: "transparent"
        anchors.top: notifyWin.top
        anchors.left: notifyWin.left
        anchors.right: notifyWin.right
        anchors.bottom: okButton.top

        Text {
            anchors.centerIn: parent

            color: "#ffffff"
            text: notifyWin.text
            font.pointSize: util.uiFontSize();

            onLinkActivated: {
                Qt.openUrlExternally(link)
            }
        }
    }

    Button {
        id: okButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        text: "OK"
        onClicked: {
            notifyWin.state = ""
            notifyWin.dismissed();
        }
    }

    states: [
        State {
            name: "visible"
            PropertyChanges {
                target: notifyWin
                y: 0
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"
            SequentialAnimation {
                PropertyAnimation { target: notifyWin; properties: "y"; duration: 200; easing.type: Easing.InOutCubic }
                ScriptAction { script: window.updateGesturesAllowed(); }
            }
        }
    ]
}
