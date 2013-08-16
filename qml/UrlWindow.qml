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
    id: urlWindow

    width: window.width-1
    height: window.height-1
    color: "#000000"
    z: 100
    property variant urls: [""]
    state: ""
    y: -(height+1)
    border.color: "#c0c0c0"
    border.width: 1
    radius: 10

    MouseArea {
        // event eater
        anchors.fill: parent
    }

    Component {
        id: listDelegate
        Rectangle {
            color: "#909090"
            width: parent.width
            height: openButton.height+4
            border.width: 1
            border.color: "#ffffff"
            radius: 5
            clip: true

            Text {
                text: modelData
                color: "#ffffff"
                anchors.verticalCenter: parent.verticalCenter
                x: 8
                width: openButton.x - x
                font.pointSize: util.uiFontSize();
                elide: Text.ElideRight
            }
            Button {
                id: openButton
                text: "Open"
                anchors.right: copyButton.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 5
                width: 70
                onClicked: {
                    Qt.openUrlExternally(modelData);
                }
            }
            Button {
                id: copyButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: "Copy"
                width: 70
                anchors.rightMargin: 5
                onClicked: {
                    util.copyTextToClipboard(modelData);
                }
            }
        }
    }

    Text {
        visible: urlWindow.urls.length == 0
        anchors.centerIn: parent
        color: "#ffffff"
        text: "No URLs"
        font.pointSize: util.uiFontSize() + 4;
    }

    ListView {
        anchors.fill: parent
        delegate: listDelegate
        model: urlWindow.urls
        spacing: 5
        anchors.margins: 5
        clip: true
    }

    Button {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        text: "Back"
        onClicked: {
            urlWindow.state = ""
        }
    }

    states: [
        State {
            name: "visible"
            PropertyChanges {
                target: urlWindow
                y: 0
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"
            SequentialAnimation {
                PropertyAnimation { target: urlWindow; properties: "y"; duration: 200; easing.type: Easing.InOutCubic }
                ScriptAction { script: window.updateGesturesAllowed(); }
            }
        }
    ]
}
