import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sideBarRoot
    property var targetStack: null
    property var menuModel: []
    property bool isCollapsed: false
    property string activeCategory: ""

    readonly property color accentColor: "#00b4ff"
    readonly property color sidebarColor: "#05080c"
    readonly property color activeBg: "#1a2634"

    Layout.preferredWidth: isCollapsed ? 70 : 240
    Layout.fillHeight: true
    color: sidebarColor
    clip: true

    Behavior on Layout.preferredWidth { NumberAnimation { duration: 350; easing.type: Easing.InOutQuad } }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Navigation Header
        Rectangle {
            Layout.fillWidth: true; height: 70; color: "transparent"
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 10
                AbstractButton {
                    Layout.preferredWidth: 50; Layout.preferredHeight: 50
                    onClicked: sideBarRoot.isCollapsed = !sideBarRoot.isCollapsed
                    contentItem: Text {
                        text: sideBarRoot.isCollapsed ? "☰" : "✕"
                        color: "white"; font.pixelSize: 22; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    }
                }
                Text {
                    text: "Hoteli Plus"; color: "white"; font.letterSpacing: 2; font.bold: true;
                    visible: !sideBarRoot.isCollapsed
                }
            }
        }

        // Navigation List
        Repeater {
            model: sideBarRoot.menuModel
            delegate: ColumnLayout {
                id: navGroup
                Layout.fillWidth: true
                spacing: 0

                // Main Menu Item
                Rectangle {
                    Layout.fillWidth: true; height: 50
                    color: targetStack.currentItem && targetStack.currentItem.objectName === modelData.name ? activeBg : "transparent"

                    RowLayout {
                        anchors.fill: parent; spacing: 0

                        // Icon Area
                        Item {
                            Layout.preferredWidth: 70; Layout.fillHeight: true;
                            Text { anchors.centerIn: parent; text: modelData.icon; color: "white"; font.pixelSize: 18 }
                        }

                        // Text Label aligned left
                        Text {
                            Layout.fillWidth: true
                            text: modelData.name; color: "white"; font.pixelSize: 13;
                            visible: !sideBarRoot.isCollapsed
                        }

                        // Arrow indicator - Only visible if subItems exists
                        Text {
                            text: "›"
                            color: "white"
                            font.pixelSize: 18
                            Layout.preferredWidth: 30
                            horizontalAlignment: Text.AlignHCenter
                            visible: !sideBarRoot.isCollapsed && !!modelData.subItems
                            rotation: sideBarRoot.activeCategory === modelData.name ? 90 : 0
                            Behavior on rotation { NumberAnimation { duration: 200 } }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (modelData.subItems) {
                                sideBarRoot.activeCategory = (sideBarRoot.activeCategory === modelData.name) ? "" : modelData.name
                            } else {
                                targetStack.replace(modelData.view)
                                targetStack.currentItem.objectName = modelData.name
                                sideBarRoot.activeCategory = ""
                            }
                        }
                    }
                }

                // Sub-Items (The Slick Reveal)
                ColumnLayout {
                    Layout.fillWidth: true
                    visible: !sideBarRoot.isCollapsed && sideBarRoot.activeCategory === modelData.name
                    clip: true
                    spacing: 0

                    Repeater {
                        model: modelData.subItems || []
                        delegate: Rectangle {
                            Layout.fillWidth: true; height: 40
                            color: subMouseArea.containsMouse ? "#14ffffff" : "transparent"

                            RowLayout {
                                anchors.fill: parent;
                                anchors.leftMargin: 70 // Aligned left, equal spacing from icon area
                                Text {
                                    text: modelData.name
                                    color: subMouseArea.containsMouse ? "white" : "#8899aa";
                                    font.pixelSize: 12
                                }
                            }
                            MouseArea {
                                id: subMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    targetStack.replace(modelData.view)
                                    targetStack.currentItem.objectName = modelData.name
                                }
                            }
                        }
                    }
                }
            }
        }
        Item { Layout.fillHeight: true }
    }
}
