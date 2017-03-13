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

Item {
    id: keyboard

    property int keyModifiers
    property Key resetSticky
    property Key currentStickyPressed
    property Key currentKeyPressed

    property string keyFgColor: "#ffffff"
    property string keyBgColor: "#202020"
    property string keyHilightBgColor: "#ffffff"
    property string keyBorderColor: "#303030"

    property bool active

    property int outmargins: util.keyboardMargins
    property int keyspacing: 6
    property int keysPerRow: keyLoader.vkbColumns()
    property real keywidth: (keyboard.width - keyspacing*keysPerRow - outmargins*2)/keysPerRow;

    width: parent.width
    height: childrenRect.height + outmargins

    Component {
        id: keyboardContents

        Column {
            id: col

            x: (keyboard.width-width)/2
            spacing: keyboard.keyspacing

            Repeater {
                id: rowRepeater

                model: keyLoader.vkbRows()
                delegate: Row {
                    spacing: keyboard.keyspacing
                    Repeater {
                        id: colRepeater

                        property int rowIndex: index
                        model: keyLoader.vkbColumns()
                        delegate: Key {
                            property var keydata: keyLoader.keyAt(colRepeater.rowIndex, index)
                            label: keydata[0]
                            code: keydata[1]
                            label_alt: keydata[2]
                            code_alt: keydata[3]
                            width: keyboard.keywidth * keydata[4] + ((keydata[4]-1)*keyboard.keyspacing) + 1
                            sticky: keydata[5]
                        }
                    }
                }
            }
        }
    }

    Loader {
        id: keyboardLoader
    }

    Component.onCompleted: {
        keyboardLoader.sourceComponent = keyboardContents;
    }

    onCurrentKeyPressedChanged: {
        if(currentKeyPressed && currentKeyPressed.currentLabel.length === 1 && currentKeyPressed.currentLabel !== " ") {
            visualKeyFeedbackRect.label = currentKeyPressed.currentLabel
            visualKeyFeedbackRect.width = currentKeyPressed.width*1.5
            visualKeyFeedbackRect.height = currentKeyPressed.height*1.5
            var mappedCoord = window.mapFromItem(currentKeyPressed, 0, 0);
            visualKeyFeedbackRect.x = mappedCoord.x - (visualKeyFeedbackRect.width-currentKeyPressed.width)/2
            visualKeyFeedbackRect.y = mappedCoord.y - currentKeyPressed.height*1.5
            visualKeyFeedbackRect.visible = true;
        } else {
            visualKeyFeedbackRect.visible = false;
        }
    }

    Connections {
        target: util
        onKeyboardLayoutChanged: {
            var ret = keyLoader.loadLayout(util.keyboardLayout)
            if (!ret) {
                showErrorMessage("There was an error loading the keyboard layout.<br>\nUsing the default one instead.");
                util.keyboardLayout = "english"
                ret = keyLoader.loadLayout(":/data/english.layout"); //try the default as a fallback (load from resources to ensure it will succeed)
                if (!ret) {
                    console.log("keyboard layout fail");
                    Qt.quit();
                }
            }
            keyboard.keyModifiers = 0
            // makes the keyboard component reload itself with new data
            keyboardLoader.sourceComponent = undefined
            keyboardLoader.sourceComponent = keyboardContents
        }
    }

    //borrowed from nemo-keyboard
    //Parameters: (x, y) in view coordinates
    function keyAt(x, y) {
        var item = keyboard
        x -= keyboard.x
        y -= keyboard.y

        while ((item = item.childAt(x, y)) != null) {
            //return the first "Key" element we find
            if (typeof item.currentCode !== 'undefined') {
                return item
            }

            // Cheaper mapToItem, assuming we're not using anything fancy.
            x -= item.x
            y -= item.y
        }

        return null
    }
}
