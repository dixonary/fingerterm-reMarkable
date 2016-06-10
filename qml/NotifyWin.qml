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

    property string text
    property bool show

    signal dismissed()

    width: window.width-1
    height: window.height-1
    color: "#000000"
    y: show ? 0 : -(height+1)
    border.color: "#c0c0c0"
    border.width: 1
    radius: window.radiusMedium

    Behavior on y { NumberAnimation { duration: 200; easing.type: Easing.InOutCubic } }

    MouseArea {
        // event eater
        anchors.fill: parent
    }
    Item {
        anchors.top: notifyWin.top
        anchors.left: notifyWin.left
        anchors.right: notifyWin.right
        anchors.bottom: okButton.top

        Text {
            anchors.centerIn: parent

            color: "#ffffff"
            text: notifyWin.text
            font.pointSize: window.uiFontSize

            onLinkActivated: {
                Qt.openUrlExternally(link)
            }
        }
    }

    Button {
        id: okButton

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: window.paddingMedium
        text: "OK"
        onClicked: {
            notifyWin.show = false
            notifyWin.dismissed();
        }
    }
}
