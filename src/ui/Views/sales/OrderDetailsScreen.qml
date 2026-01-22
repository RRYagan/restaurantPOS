import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Rectangle {
    id: root
    property string currentOrderId: ""
    color: "transparent"
    // Ensure this property exists so the push() can fill it
    property var detailsModel: globalOrderDetailModel

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 25; spacing: 20

        // --- NAVIGATION HEADER ---
        RowLayout {
            spacing: 15
            Button {
                text: "←"
                flat: true; font.pixelSize: 24; palette.buttonText: "white"
                onClicked: contentStack.pop()
            }

            Text {
                text: "Order Details #" + currentOrderId.substring(0, 8)
                font.pixelSize: 24; font.bold: true; color: "white"
            }
            Item { Layout.fillWidth: true }
        }

        // --- ITEMS LIST ---
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: Qt.rgba(0, 0, 0, 0.4); radius: 12; border.color: Qt.rgba(255, 255, 255, 0.1); clip: true

            ListView {
                id: detailsList
                anchors.fill: parent; anchors.margins: 1
                model: detailsModel // OrderDetailView instance
                spacing: 0

                delegate: ItemDelegate {
                    width: detailsList.width; height: 70
                    background: Rectangle {
                        color: hovered ? Qt.rgba(1, 1, 1, 0.05) : "transparent"
                        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Qt.rgba(1,1,1,0.05) }
                    }

                    contentItem: RowLayout {
                        anchors.fill: parent; anchors.margins: 15
                        spacing: 15

                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Text { text: model.name; color: "white"; font.pixelSize: 16; font.bold: true }
                            Text {
                                text: model.service_state.toUpperCase()
                                color: model.service_state === "served" ? "#2ecc71" : "#f1c40f"
                                font.pixelSize: 10; font.bold: true
                            }
                        }

                        Text { text: model.quantity + "x"; color: "white"; Layout.preferredWidth: 40 }
                        Text { text: model.price.formatted; color: "#2ecc71"; font.bold: true; Layout.preferredWidth: 80 }

                        // --- CHANGE STATUS BUTTON ---
                        Button {
                            id: statusBtn
                            Layout.preferredHeight: 36; Layout.preferredWidth: 110
                            visible: model.service_state !== "served"

                            background: Rectangle {
                                color: "transparent"; border.color: "#3498db"; border.width: 1; radius: 6
                                opacity: statusBtn.pressed ? 0.5 : 1.0
                            }

                            contentItem: Text {
                                text: model.service_state === "ordered" ? "PREPARE" : "SERVE"
                                color: "#3498db"; font.bold: true; font.pixelSize: 11
                                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                let nextStatus = (model.service_state === "ordered") ? "preparing" : "served";
                                _salesModel.updateItemStatus(model.id, nextStatus);
                                detailsModel.loadOrder(root.currentOrderId); // Refresh local list
                            }
                        }
                    }
                }
            }
        }

        // --- ACTION BUTTONS ---
        RowLayout {
            Layout.fillWidth: true; spacing: 15
            Button { text: "Print Kitchen"; Layout.fillWidth: true; Layout.preferredHeight: 50 }
            Button { text: "Print Bill"; highlighted: true; Layout.fillWidth: true; Layout.preferredHeight: 50 }
            Button {
                text: "Proceed to Payment >"
                highlighted: true; palette.button: "#2ecc71"
                Layout.fillWidth: true; Layout.preferredHeight: 50
            }
        }
    }

    Component.onCompleted: {
        if (currentOrderId !== "" && detailsModel) {
            detailsModel.loadOrder(currentOrderId);
        }
    }
}
