import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Item {
    id: setupRoot
    anchors.fill: parent


    // Fix: Changed from alias to var to receive globalProductModel from Main.qml
    property var controller
    property var categoryModel
    property var taxModel
    property var unitModel
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Navigation Header using whiteTheme [cite: 44]
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: whiteTheme.surface
            border.color: whiteTheme.border

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                Button {
                    text: "← Back to Dashboard"
                    visible: mainStack.currentIndex !== 0
                    onClicked: mainStack.currentIndex = 0
                }
                Text {
                    text: mainStack.currentIndex === 0 ? "System Setup" : "Menu & Ingredient Setup"
                    font.pixelSize: 20
                    font.bold: true
                    color: whiteTheme.textMain
                }
            }
        }

        StackLayout {
            id: mainStack
            Layout.fillWidth: true
            Layout.fillHeight: true

            // Requirement 1: Navigation to various setup views
            SetupDashboard {
                onMenuSetupClicked: mainStack.currentIndex = 1
                onFloorPlanClicked: console.log("Floor plan nav")
            }

            // Requirement 2: Menu setup to add product and ingredients
            MenuSetupView {
                controller: setupRoot.controller
                categoryModel: setupRoot.categoryModel
                taxModel: setupRoot.taxModel
                unitModel: setupRoot.unitModel
            }
        }
    }
}
