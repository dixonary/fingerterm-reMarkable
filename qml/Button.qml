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

import QtQuick 1.1

Rectangle {
    id: button
    property string text: ""
    property string textColor: "#ffffff"
    property bool enabled: true
    property bool highlighted: false
    signal clicked();

    // for custom user menu actions
    property bool isShellCommand: false

    color: highlighted ? "#606060" : "#202020"
    border.color: "#303030"
    border.width: 1
    radius: 5
    z: 0
    clip: true

    width: 180
    height: 68

    onHighlightedChanged: {
        if(highlighted)
            button.color = "#606060"
        else
            button.color = "#202020"
    }

    Text {
        // decoration for user-defined command buttons
        visible: isShellCommand
        anchors.fill: parent
        font.pointSize: 46
        text: "$"
        color: "#305030"
    }

    Text {
        text: button.text
        color: button.enabled ? button.textColor : "#606060"
        anchors.centerIn: parent
        font.pointSize: util.uiFontSize();
    }

    MouseArea {
        id: btnMouseArea
        enabled: button.enabled
        anchors.fill: parent
        onClicked: {
            button.clicked();
        }
        onPressedChanged: {
            if(pressed) {
                button.color = "#ffffff"
            } else {
                if(highlighted)
                    button.color = "#606060"
                else
                    button.color = "#202020"
            }
        }
    }

}
