import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI

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
