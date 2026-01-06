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
    // CategoryView { id: catModel }

    // MenuView {
    //         id: itemModel
    //         currentCategory: "All"
    //     }

    RowLayout {
        anchors.fill: parent
        spacing: 0



        // --- Main Content Area ---
        StackLayout {
            currentIndex: setupRoot.currentSubMenu
            Layout.fillWidth: true
            Layout.fillHeight: true


            MenuSetup {
                // itemDeleteDialog: confirmDeleteItemDialog
                // catDeleteDialog: confirmDeleteCatDialog

            }
            TableSetup { }
            ThemeSetup { }
        }
    }

  }
