import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.Sales 1.0

Rectangle {
    id: inventoryPage
    color: "#f4f7f6"

    property InventoryModel invModel
    property UserModel usrModel
    readonly property bool isManager: usrModel.isAdmin

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        Text { text: "Stock & Inventory"; font.pixelSize: 22; font.bold: true }

        ListView {
            id: invList
            Layout.fillWidth: true; Layout.fillHeight: true
            model: invModel
            clip: true
            spacing: 8

            delegate: Rectangle {
                width: invList.width; height: 60
                color: "white"; radius: 5; border.color: "#e0e0e0"

                RowLayout {
                    anchors.fill: parent; anchors.margins: 10
                    Text { text: model.name; font.bold: true; Layout.fillWidth: true }

                    Label {
                        text: model.quantity + " " + model.unit
                        color: model.quantity < 10 ? "red" : "black"
                    }

                    Button {
                        text: "Edit"
                        visible: inventoryPage.isManager // Restrict to managers
                        onClicked: { /* Open Edit Dialog */ }
                    }
                }
            }
        }
    }
}
