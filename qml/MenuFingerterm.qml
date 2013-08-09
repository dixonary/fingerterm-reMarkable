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
import QtQuick.XmlListModel 2.0

Rectangle {
    id: menuWin
    color: "transparent"
    z: 30
    width: window.width
    height: window.height
    visible: false
    property bool showing: false
    property bool enableCopy: false
    property bool enablePaste: false
    property string currentSwipeLocking: util.settingsValue("ui/allowSwipe")
    property string currentShowMethod: util.settingsValue("ui/vkbShowMethod")
    property string currentDragMode: util.settingsValue("ui/dragMode")
    property string currentOrientationLockMode: util.settingsValue("ui/orientationLockMode")
    property int keyboardFadeOutDelay: util.settingsValue("ui/keyboardFadeOutDelay")

    Rectangle {
        id: fader
        color: "#ffffff"
        opacity: 0
        y: 0
        x: 0
        width: menuWin.width
        height: menuWin.height
        MouseArea {
            anchors.fill: parent
            onClicked: {
                hideMenu();
            }
        }
        Behavior on opacity {
            SequentialAnimation {
                NumberAnimation { duration: 100; }
                ScriptAction { script: menuWin.visible = menuWin.showing; }
            }
        }
    }
    Rectangle {
        id: rect
        color: "#e0e0e0"
        y: 0
        x: menuWin.width+1;
        width: flickableContent.width + 22;
        height: menuWin.height
        z: 35

        MouseArea {
            // event eater
            anchors.fill: parent
        }

        Behavior on x {
            NumberAnimation { duration: 100; easing.type: Easing.InOutQuad; }
        }

        XmlListModel {
            id: xmlModel
            xml: term.getUserMenuXml()
            query: "/userMenu/item"

            XmlRole { name: "title"; query: "title/string()" }
            XmlRole { name: "command"; query: "command/string()" }
            XmlRole { name: "disableOn"; query: "disableOn/string()" }
        }

        Component {
            id: xmlDelegate
            Button {
                text: title
                isShellCommand: true
                enabled: disableOn.length === 0 || window.windowTitle.search(disableOn) === -1
                onClicked: {
                    hideMenu();
                    term.putString(command, true);
                }
            }
        }

        Rectangle {
            id: scrollIndicator
            y: menuFlickArea.visibleArea.yPosition*menuFlickArea.height + 6
            x: parent.width-10
            width: 6
            height: menuFlickArea.visibleArea.heightRatio*menuFlickArea.height
            radius: 3
            color: "#202020"
        }

        Flickable {
            id: menuFlickArea
            anchors.fill: parent
            anchors.topMargin: 6
            anchors.bottomMargin: 6
            anchors.leftMargin: 6
            anchors.rightMargin: 16
            contentHeight: flickableContent.height

            Column {
                id: flickableContent
                spacing: 12

                Row {
                    id: menuBlocksRow
                    spacing: 8

                    Column {
                        spacing: 12
                        Repeater {
                            model: xmlModel
                            delegate: xmlDelegate
                        }
                    }

                    Column {
                        spacing: 12

                        Row {
                            Button {
                                text: "Copy"
                                onClicked: {
                                    hideMenu();
                                    term.copySelectionToClipboard();
                                }
                                width: 90
                                enabled: menuWin.enableCopy
                            }
                            Button {
                                text: "Paste"
                                onClicked: {
                                    hideMenu();
                                    term.pasteFromClipboard();
                                }
                                width: 90
                                enabled: menuWin.enablePaste
                            }
                        }
                        Button {
                            text: "URL grabber"
                            onClicked: {
                                hideMenu();
                                urlWindow.urls = term.grabURLsFromBuffer();
                                urlWindow.state = "visible";
                            }
                        }
                        Rectangle {
                            width: 180
                            height: 68
                            radius: 5
                            color: "#606060"
                            border.color: "#000000"
                            border.width: 1
                            Column {
                                Text {
                                    width: 180
                                    height: 20
                                    color: "#ffffff"
                                    font.pointSize: util.uiFontSize()-1;
                                    text: "Font size"
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                Row {
                                    Button {
                                        text: "<font size=\"+3\">+</font>"
                                        onClicked: {
                                            textrender.fontPointSize = textrender.fontPointSize + 1;
                                            lineView.fontPointSize = textrender.fontPointSize;
                                            util.notifyText(term.termSize().width+"x"+term.termSize().height);
                                        }
                                        width: 90
                                        height: 48
                                    }
                                    Button {
                                        text: "<font size=\"+3\">-</font>"
                                        onClicked: {
                                            textrender.fontPointSize = textrender.fontPointSize - 1;
                                            lineView.fontPointSize = textrender.fontPointSize;
                                            util.notifyText(term.termSize().width+"x"+term.termSize().height);
                                        }
                                        width: 90
                                        height: 48
                                    }
                                }
                            }
                        }
                        Rectangle {
                            width: 180
                            height: 68
                            radius: 5
                            color: "#606060"
                            border.color: "#000000"
                            border.width: 1
                            Column {
                                Text {
                                    width: 180
                                    height: 20
                                    color: "#ffffff"
                                    font.pointSize: util.uiFontSize()-1;
                                    text: "UI Orientation"
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                Row {
                                    Button {
                                        text: "<font size=\"-1\">Auto</font>"
                                        highlighted: currentOrientationLockMode=="auto"
                                        onClicked: {
                                            currentOrientationLockMode = "auto";
                                            window.setOrientationLockMode("auto");
                                        }
                                        width: 60
                                        height: 48
                                    }
                                    Button {
                                        text: "<font size=\"-1\">L<font>"
                                        highlighted: currentOrientationLockMode=="landscape"
                                        onClicked: {
                                            currentOrientationLockMode = "landscape";
                                            window.setOrientationLockMode("landscape");
                                        }
                                        width: 60
                                        height: 48
                                    }
                                    Button {
                                        text: "<font size=\"-1\">P</font>"
                                        highlighted: currentOrientationLockMode=="portrait"
                                        onClicked: {
                                            currentOrientationLockMode = "portrait";
                                            window.setOrientationLockMode("portrait");
                                        }
                                        width: 60
                                        height: 48
                                    }
                                }
                            }
                        }
                        Rectangle {
                            width: 180
                            height: 68
                            radius: 5
                            color: "#606060"
                            border.color: "#000000"
                            border.width: 1
                            Column {
                                Text {
                                    width: 180
                                    height: 20
                                    color: "#ffffff"
                                    font.pointSize: util.uiFontSize()-1;
                                    text: "Drag mode"
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                Row {
                                    Button {
                                        text: "<font size=\"-1\">Gesture</font>"
                                        highlighted: currentDragMode=="gestures"
                                        onClicked: {
                                            util.setSettingsValue("ui/dragMode", "gestures");
                                            term.clearSelection();
                                            currentDragMode = "gestures";
                                            hideMenu();
                                        }
                                        width: 60
                                        height: 48
                                    }
                                    Button {
                                        text: "<font size=\"-1\">Scroll</font>"
                                        highlighted: currentDragMode=="scroll"
                                        onClicked: {
                                            util.setSettingsValue("ui/dragMode", "scroll");
                                            currentDragMode = "scroll";
                                            term.clearSelection();
                                            hideMenu();
                                        }
                                        width: 60
                                        height: 48
                                    }
                                    Button {
                                        text: "<font size=\"-1\">Select</font>"
                                        highlighted: currentDragMode=="select"
                                        onClicked: {
                                            util.setSettingsValue("ui/dragMode", "select");
                                            currentDragMode = "select";
                                            hideMenu();
                                        }
                                        width: 60
                                        height: 48
                                    }
                                }
                            }
                        }
                        Rectangle {
                            width: 180
                            height: 68
                            radius: 5
                            color: "#606060"
                            border.color: "#000000"
                            border.width: 1
                            Column {
                                Text {
                                    width: 180
                                    height: 20
                                    color: "#ffffff"
                                    font.pointSize: util.uiFontSize()-1;
                                    text: "VKB behavior"
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                Row {
                                    Button {
                                        text: "Off"
                                        highlighted: currentShowMethod=="off"
                                        onClicked: {
                                            util.setSettingsValue("ui/vkbShowMethod", "off");
                                            currentShowMethod = "off";
                                            window.setTextRenderAttributes();
                                            hideMenu();
                                        }
                                        width: 60
                                        height: 48
                                    }
                                    Button {
                                        text: "Fade"
                                        highlighted: currentShowMethod=="fade"
                                        onClicked: {
                                            util.setSettingsValue("ui/vkbShowMethod", "fade");
                                            currentShowMethod = "fade";
                                            window.setTextRenderAttributes();
                                            hideMenu();
                                        }
                                        width: 60
                                        height: 48
                                    }
                                    Button {
                                        text: "Move"
                                        highlighted: currentShowMethod=="move"
                                        onClicked: {
                                            util.setSettingsValue("ui/vkbShowMethod", "move");
                                            currentShowMethod = "move";
                                            window.setTextRenderAttributes();
                                            hideMenu();
                                        }
                                        width: 60
                                        height: 48
                                    }
                                }
                            }
                        }
                        Rectangle {
                            visible: util.isHarmattan()
                            width: 180
                            height: 68
                            radius: 5
                            color: "#606060"
                            border.color: "#000000"
                            border.width: 1
                            Column {
                                Text {
                                    width: 180
                                    height: 20
                                    color: "#ffffff"
                                    font.pointSize: util.uiFontSize()-1;
                                    text: "Allow swiping"
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                Row {
                                    Button {
                                        text: "No"
                                        width: 60
                                        height: 48
                                        highlighted: currentSwipeLocking=="false"
                                        onClicked: {
                                            changeSwipeLocking("false")
                                        }
                                    }
                                    Button {
                                        text: "Yes"
                                        width: 60
                                        height: 48
                                        highlighted: currentSwipeLocking=="true"
                                        onClicked: {
                                            changeSwipeLocking("true")
                                        }
                                    }
                                    Button {
                                        text: "Auto"
                                        width: 60
                                        height: 48
                                        highlighted: currentSwipeLocking=="auto"
                                        onClicked: {
                                            changeSwipeLocking("auto")
                                        }
                                    }
                                }
                            }
                        }
                        Button {
                            text: "New window"
                            onClicked: {
                                hideMenu();
                                util.openNewWindow();
                            }
                        }
                        Button {
                            text: "VKB layout..."
                            onClicked: {
                                hideMenu();
                                layoutWindow.layouts = keyLoader.availableLayouts();
                                layoutWindow.state = "visible";
                            }
                        }
                        Button {
                            text: "About"
                            onClicked: {
                                hideMenu();
                                aboutDialog.termW = term.termSize().width
                                aboutDialog.termH = term.termSize().height
                                aboutDialog.state = "visible"
                            }
                        }
                        Button {
                            visible: (currentSwipeLocking=="false" && util.isHarmattan()) || !util.isHarmattan();
                            text: "Minimize"
                            onClicked: {
                                hideMenu();
                                util.windowMinimize();
                            }
                        }
                        Button {
                            text: "Quit"
                            onClicked: {
                                hideMenu();
                                Qt.quit();
                            }
                        }
                    }
                }
                // VKB delay slider
                Rectangle {
                    id: vkbDelaySliderArea
                    width: menuBlocksRow.width
                    height: 68
                    radius: 5
                    color: "#606060"
                    border.color: "#000000"
                    border.width: 1
                    Text {
                        width: parent.width
                        height: 20
                        color: "#ffffff"
                        font.pointSize: util.uiFontSize()-1;
                        text: "VKB delay: " + vkbDelaySlider.keyboardFadeOutDelayLabel + " ms"
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Rectangle {
                        x: 5
                        y: vkbDelaySlider.y + vkbDelaySlider.height/2 - height/2
                        width: menuBlocksRow.width - 10
                        height: 10
                        radius: 5
                        z: 1
                        color: "#909090"
                    }
                    Rectangle {
                        id: vkbDelaySlider
                        property int keyboardFadeOutDelayLabel: keyboardFadeOutDelay
                        x: (keyboardFadeOutDelay-1000)/9000 * (vkbDelaySliderArea.width - vkbDelaySlider.width)
                        y: 20
                        width: 60
                        radius: 15
                        height: parent.height-20
                        color: "#202020"
                        z: 2
                        onXChanged: {
                            if (vkbDelaySliderMA.drag.active)
                                vkbDelaySlider.keyboardFadeOutDelayLabel =
                                        Math.floor((1000+vkbDelaySlider.x/vkbDelaySliderMA.drag.maximumX*9000)/250)*250;
                        }
                        MouseArea {
                            id: vkbDelaySliderMA
                            anchors.fill: parent
                            drag.target: vkbDelaySlider
                            drag.axis: Drag.XAxis
                            drag.minimumX: 0
                            drag.maximumX: vkbDelaySliderArea.width - vkbDelaySlider.width
                            drag.onActiveChanged: {
                                if (!drag.active) {
                                    keyboardFadeOutDelay = vkbDelaySlider.keyboardFadeOutDelayLabel
                                    util.setSettingsValue("ui/keyboardFadeOutDelay", keyboardFadeOutDelay);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    onWidthChanged: {
        if(showing) {
            showMenu();
        } else {
            hideMenu();
        }
    }

    Connections {
        target: util
        onClipboardOrSelectionChanged: {
            enableCopy = util.terminalHasSelection();
            enablePaste = util.canPaste();
        }
    }

    function showMenu()
    {
        showing = true;
        visible = true;
        fader.opacity = 0.5;
        rect.x = menuWin.width-rect.width;
        window.updateGesturesAllowed();
        enableCopy = util.terminalHasSelection();
        enablePaste = util.canPaste();
    }

    function hideMenu()
    {
        showing = false;
        fader.opacity = 0;
        rect.x = menuWin.width+1;
        window.updateGesturesAllowed();
    }

    function changeSwipeLocking(state)
    {
        currentSwipeLocking = state
        util.setSettingsValue("ui/allowSwipe", state)
        util.updateSwipeLock(!vkb.active);
    }
}
