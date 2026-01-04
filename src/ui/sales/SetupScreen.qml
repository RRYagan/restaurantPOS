import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.Sales

Rectangle {
    id: setupRoot
    color: "#f4f7f6"

    // Navigation State
    property int currentSubMenu: 0
    property bool sidebarCollapsed: false

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // --- Collapsible Sub-Menu Sidebar ---
        Rectangle {
            id: subSidebar
            Layout.preferredWidth: setupRoot.sidebarCollapsed ? 50 : 200
            Layout.fillHeight: true
            color: "#ecf0f1"
            border.color: "#e0e0e0"

            // Smooth transition for collapsing
            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 250; easing.type: Easing.InOutQuad }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 5
                spacing: 10

                // Toggle Button
                Button {
                    text: setupRoot.sidebarCollapsed ? "☰" : "◀ Collapse"
                    Layout.fillWidth: true
                    flat: true
                    onClicked: setupRoot.sidebarCollapsed = !setupRoot.sidebarCollapsed
                }

                // Sub-menu Items
                Repeater {
                    model: [
                        { name: "Menu", icon: "🍴" },
                        { name: "Tables", icon: "🪑" },
                        { name: "Theme", icon: "🎨" }
                    ]

                    Button {
                        Layout.fillWidth: true
                        flat: setupRoot.currentSubMenu !== index
                        highlighted: setupRoot.currentSubMenu === index

                        // Show only icon when collapsed, name + icon when expanded
                        contentItem: Text {
                            text: setupRoot.sidebarCollapsed ? modelData.icon : modelData.icon + "  " + modelData.name
                            horizontalAlignment: Text.AlignLeft
                            leftPadding: 10
                            font.bold: setupRoot.currentSubMenu === index
                        }

                        onClicked: setupRoot.currentSubMenu = index
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        // --- Main Content Area ---
        StackLayout {
            currentIndex: setupRoot.currentSubMenu
            Layout.fillWidth: true
            Layout.fillHeight: true

            // Nested Views
            MenuSetup { } // Your existing logic [cite: 8, 21]
            TableSetup { }
            ThemeSetup { }
        }
    }
}
