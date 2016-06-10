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

Item {
    id: menuWin

    property bool showing
    property string currentShowMethod: util.settingsValue("ui/vkbShowMethod")
    property string currentDragMode: util.settingsValue("ui/dragMode")
    property string currentOrientationLockMode: util.settingsValue("ui/orientationLockMode")
    property int keyboardFadeOutDelay: util.settingsValue("ui/keyboardFadeOutDelay")

    visible: rect.x < menuWin.width

    Rectangle {
        id: fader

        color: "#000000"
        opacity: menuWin.showing ? 0.5 : 0.0
        anchors.fill: parent

        Behavior on opacity { NumberAnimation { duration: 100; } }

        MouseArea {
            anchors.fill: parent
            onClicked: menuWin.showing = false
        }
    }
    Rectangle {
        id: rect

        color: "#e0e0e0"
        anchors.left: parent.right
        anchors.leftMargin: menuWin.showing ? -width : 1
        width: flickableContent.width + 22*window.pixelRatio;
        height: menuWin.height

        MouseArea {
            // event eater
            anchors.fill: parent
        }

        Behavior on anchors.leftMargin {
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
                enabled: disableOn.length === 0 || util.windowTitle.search(disableOn) === -1
                onClicked: {
                    menuWin.showing = false;
                    term.putString(command, true);
                }
            }
        }

        Rectangle {
            y: menuFlickArea.visibleArea.yPosition*menuFlickArea.height + window.scrollBarWidth
            x: parent.width-window.paddingMedium
            width: window.scrollBarWidth
            height: menuFlickArea.visibleArea.heightRatio*menuFlickArea.height
            radius: 3*window.pixelRatio
            color: "#202020"
        }

        Flickable {
            id: menuFlickArea

            anchors.fill: parent
            anchors.topMargin: window.scrollBarWidth
            anchors.bottomMargin: window.scrollBarWidth
            anchors.leftMargin: window.scrollBarWidth
            anchors.rightMargin: 16*window.pixelRatio
            contentHeight: flickableContent.height

            Column {
                id: flickableContent

                spacing: 12*window.pixelRatio

                Row {
                    id: menuBlocksRow
                    spacing: 8*window.pixelRatio

                    Column {
                        spacing: 12*window.pixelRatio
                        Repeater {
                            model: xmlModel
                            delegate: xmlDelegate
                        }
                    }

                    Column {
                        spacing: 12*window.pixelRatio

                        Row {
                            Button {
                                text: "Copy"
                                onClicked: {
                                    menuWin.showing = false;
                                    term.copySelectionToClipboard();
                                }
                                width: window.buttonWidthHalf
                                height: window.buttonHeightLarge
                                enabled: util.terminalHasSelection
                            }
                            Button {
                                text: "Paste"
                                onClicked: {
                                    menuWin.showing = false;
                                    term.pasteFromClipboard();
                                }
                                width: window.buttonWidthHalf
                                height: window.buttonHeightLarge
                                enabled: util.canPaste
                            }
                        }
                        Button {
                            text: "URL grabber"
                            width: window.buttonWidthLarge
                            height: window.buttonHeightLarge
                            onClicked: {
                                menuWin.showing = false;
                                urlWindow.urls = term.grabURLsFromBuffer();
                                urlWindow.state = "visible";
                            }
                        }
                        Rectangle {
                            width: window.buttonWidthLarge
                            height: window.buttonHeightLarge
                            radius: window.radiusSmall
                            color: "#606060"
                            border.color: "#000000"
                            border.width: 1

                            Column {
                                Text {
                                    width: window.buttonWidthLarge
                                    height: window.headerHeight
                                    color: "#ffffff"
                                    font.pointSize: window.uiFontSize-1
                                    text: "Font size"
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                Row {
                                    Button {
                                        text: "<font size=\"+3\">+</font>"
                                        onClicked: {
                                            textrender.fontPointSize = textrender.fontPointSize + window.pixelRatio;
                                            lineView.fontPointSize = textrender.fontPointSize;
                                            util.notifyText(term.termSize().width+"x"+term.termSize().height);
                                        }
                                        width: window.buttonWidthHalf
                                        height: window.buttonHeightSmall
                                    }
                                    Button {
                                        text: "<font size=\"+3\">-</font>"
                                        onClicked: {
                                            textrender.fontPointSize = textrender.fontPointSize - window.pixelRatio;
                                            lineView.fontPointSize = textrender.fontPointSize;
                                            util.notifyText(term.termSize().width+"x"+term.termSize().height);
                                        }
                                        width: window.buttonWidthHalf
                                        height: window.buttonHeightSmall
                                    }
                                }
                            }
                        }
                        Rectangle {
                            width: window.buttonWidthLarge
                            height: window.buttonHeightLarge
                            radius: window.radiusSmall
                            color: "#606060"
                            border.color: "#000000"
                            border.width: 1

                            Column {
                                Text {
                                    width: window.buttonWidthLarge
                                    height: window.headerHeight
                                    color: "#ffffff"
                                    font.pointSize: window.uiFontSize-1
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
                                        width: window.buttonWidthSmall
                                        height: window.buttonHeightSmall
                                    }
                                    Button {
                                        text: "<font size=\"-1\">L<font>"
                                        highlighted: currentOrientationLockMode=="landscape"
                                        onClicked: {
                                            currentOrientationLockMode = "landscape";
                                            window.setOrientationLockMode("landscape");
                                        }
                                        width: window.buttonWidthSmall
                                        height: window.buttonHeightSmall
                                    }
                                    Button {
                                        text: "<font size=\"-1\">P</font>"
                                        highlighted: currentOrientationLockMode=="portrait"
                                        onClicked: {
                                            currentOrientationLockMode = "portrait";
                                            window.setOrientationLockMode("portrait");
                                        }
                                        width: window.buttonWidthSmall
                                        height: window.buttonHeightSmall
                                    }
                                }
                            }
                        }
                        Rectangle {
                            width: window.buttonWidthLarge
                            height: window.buttonHeightLarge
                            radius: window.radiusSmall
                            color: "#606060"
                            border.color: "#000000"
                            border.width: 1

                            Column {
                                Text {
                                    width: window.buttonWidthLarge
                                    height: window.headerHeight
                                    color: "#ffffff"
                                    font.pointSize: window.uiFontSize-1
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
                                            menuWin.showing = false;
                                        }
                                        width: window.buttonWidthSmall
                                        height: window.buttonHeightSmall
                                    }
                                    Button {
                                        text: "<font size=\"-1\">Scroll</font>"
                                        highlighted: currentDragMode=="scroll"
                                        onClicked: {
                                            util.setSettingsValue("ui/dragMode", "scroll");
                                            currentDragMode = "scroll";
                                            term.clearSelection();
                                            menuWin.showing = false;
                                        }
                                        width: window.buttonWidthSmall
                                        height: window.buttonHeightSmall
                                    }
                                    Button {
                                        text: "<font size=\"-1\">Select</font>"
                                        highlighted: currentDragMode=="select"
                                        onClicked: {
                                            util.setSettingsValue("ui/dragMode", "select");
                                            currentDragMode = "select";
                                            menuWin.showing = false;
                                        }
                                        width: window.buttonWidthSmall
                                        height: window.buttonHeightSmall
                                    }
                                }
                            }
                        }
                        Rectangle {
                            width: window.buttonWidthLarge
                            height: window.buttonHeightLarge
                            radius: window.radiusSmall
                            color: "#606060"
                            border.color: "#000000"
                            border.width: 1

                            Column {
                                Text {
                                    width: window.buttonWidthLarge
                                    height: window.headerHeight
                                    color: "#ffffff"
                                    font.pointSize: window.uiFontSize-1
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
                                            menuWin.showing = false;
                                        }
                                        width: window.buttonWidthSmall
                                        height: window.buttonHeightSmall
                                    }
                                    Button {
                                        text: "Fade"
                                        highlighted: currentShowMethod=="fade"
                                        onClicked: {
                                            util.setSettingsValue("ui/vkbShowMethod", "fade");
                                            currentShowMethod = "fade";
                                            window.setTextRenderAttributes();
                                            menuWin.showing = false;
                                        }
                                        width: window.buttonWidthSmall
                                        height: window.buttonHeightSmall
                                    }
                                    Button {
                                        text: "Move"
                                        highlighted: currentShowMethod=="move"
                                        onClicked: {
                                            util.setSettingsValue("ui/vkbShowMethod", "move");
                                            currentShowMethod = "move";
                                            window.setTextRenderAttributes();
                                            menuWin.showing = false;
                                        }
                                        width: window.buttonWidthSmall
                                        height: window.buttonHeightSmall
                                    }
                                }
                            }
                        }
                        Button {
                            text: "New window"
                            onClicked: {
                                menuWin.showing = false;
                                util.openNewWindow();
                            }
                        }
                        Button {
                            text: "VKB layout..."
                            onClicked: {
                                menuWin.showing = false;
                                layoutWindow.layouts = keyLoader.availableLayouts();
                                layoutWindow.state = "visible";
                            }
                        }
                        Button {
                            text: "About"
                            onClicked: {
                                menuWin.showing = false;
                                aboutDialog.termW = term.termSize().width
                                aboutDialog.termH = term.termSize().height
                                aboutDialog.state = "visible"
                            }
                        }
                        Button {
                            text: "Quit"
                            onClicked: {
                                menuWin.showing = false;
                                Qt.quit();
                            }
                        }
                    }
                }
                // VKB delay slider
                Rectangle {
                    id: vkbDelaySliderArea

                    width: menuBlocksRow.width
                    height: window.buttonHeightLarge
                    radius: window.radiusSmall
                    color: "#606060"
                    border.color: "#000000"
                    border.width: 1

                    Text {
                        width: parent.width
                        height: window.headerHeight
                        color: "#ffffff"
                        font.pointSize: window.uiFontSize-1
                        text: "VKB delay: " + vkbDelaySlider.keyboardFadeOutDelayLabel + " ms"
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Rectangle {
                        x: window.paddingSmall
                        y: vkbDelaySlider.y + vkbDelaySlider.height/2 - height/2
                        width: menuBlocksRow.width - window.paddingMedium
                        height: window.paddingMedium
                        radius: window.radiusSmall
                        color: "#909090"
                    }
                    Rectangle {
                        id: vkbDelaySlider

                        property int keyboardFadeOutDelayLabel: keyboardFadeOutDelay

                        x: (keyboardFadeOutDelay-1000)/9000 * (vkbDelaySliderArea.width - vkbDelaySlider.width)
                        y: window.headerHeight
                        width: window.buttonWidthSmall
                        radius: window.radiusLarge
                        height: parent.height-window.headerHeight
                        color: "#202020"
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
}
