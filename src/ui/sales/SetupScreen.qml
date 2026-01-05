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
    CategoryView { id: catModel }

    MenuView {
            id: itemModel
            currentCategory: "All"
        }

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

            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 250; easing.type: Easing.InOutQuad }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 5
                spacing: 10

                Button {
                    text: setupRoot.sidebarCollapsed ? "☰" : "◀ Collapse"
                    Layout.fillWidth: true
                    flat: true
                    onClicked: setupRoot.sidebarCollapsed = !setupRoot.sidebarCollapsed
                }

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

            MenuSetup {
                itemDeleteDialog: confirmDeleteItemDialog
                catDeleteDialog: confirmDeleteCatDialog
            }
            TableSetup { }
            ThemeSetup { }
        }
    }

    // --- Fixed Confirmation Dialogs ---
    Dialog {
        id: confirmDeleteItemDialog
        title: "Confirm Deletion"
        width: 350
        standardButtons: Dialog.Yes | Dialog.No
        anchors.centerIn: parent
        modal: true
        property int itemIdToDelete: -1

        // Use a ColumnLayout to manage content without binding loops
        contentItem: ColumnLayout {
            spacing: 20
            Label {
                text: "Are you sure you want to delete this item? This action cannot be undone."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: 300 // Set a fixed width to break the loop
            }
        }

        onAccepted: {
            if (itemIdToDelete !== -1) {
                itemModel.deleteItem(itemIdToDelete) // itemModel must be defined in SetupScreen.qml
            }
        }
    }
    Dialog {
        id: confirmDeleteCatDialog
        title: "Delete Category"
        width: 350
        standardButtons: Dialog.Yes | Dialog.No
        anchors.centerIn: parent
        modal: true
        property string catNameToDelete: ""

        contentItem: ColumnLayout {
            spacing: 20
            Label {
                text: "Delete '" + confirmDeleteCatDialog.catNameToDelete + "'? All items in this category will be affected."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: 300
            }
        }

        onAccepted: {
            if (catNameToDelete !== "") {
                catModel.deleteCategory(catNameToDelete) // catModel must be defined in SetupScreen.qml
            }
        }
    }
}
