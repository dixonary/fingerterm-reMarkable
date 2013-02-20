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
    id: lineView
    property variant lines: [""]
    property int fontPointSize: util.settingsValue("ui/fontSize");
    property int cursorX: 1
    property int cursorWidth: 10
    property int cursorHeight: 10
    property int extraLines: 1

    color: "#404040"
    border.width: 2
    border.color: "#909090"
    radius: 5
    height: 0
    width: parent.width

    Text {
        id: fontHeightHack
        visible: false
        text: "X"
        font.family: util.settingsValue("ui/fontFamily");
        font.pointSize: lineView.fontPointSize
    }

    Rectangle {
        visible: vkb.active
        x: cursorX
        y: lineTextCol.y + fontHeightHack.height*(extraLines+1) - cursorHeight - 3
        width: cursorWidth
        height: cursorHeight
        color: "transparent"
        border.color: "#ffffff"
        border.width: 1
    }

    Column {
        id: lineTextCol
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        Repeater {
            model: lines
            delegate:
                Rectangle {
                height: fontHeightHack.height
                width: lineTextCol.width
                color: "transparent"
                Text {
                    color: "#ffffff"
                    font.family: util.settingsValue("ui/fontFamily");
                    font.pointSize: lineView.fontPointSize
                    text: modelData
                    textFormat: Text.PlainText
                    wrapMode: Text.NoWrap
                    elide: Text.ElideNone
                    maximumLineCount: 1
                }
            }
        }
        onHeightChanged: {
            if(lineView.visible)
                lineView.height = height+8
            setPosition(vkb.active)
        }
    }

    Component.onCompleted: {
        extraLines = util.settingsValue("ui/showExtraLinesFromCursor");
    }

    function setPosition(vkbActive)
    {
        if( util.settingsValue("ui/vkbShowMethod")==="off" ) {
            lineView.visible = false;
            return;
        }

        lineView.visible = true;
        if(vkbActive && util.settingsValue("ui/vkbShowMethod")!=="move") {
            y = 0;
        } else {
            y = -(height+5)
        }
    }
}
