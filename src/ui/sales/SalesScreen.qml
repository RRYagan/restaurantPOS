import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.Sales 1.0

Rectangle {
    id: root
    color: "#f8f9fa"

    // PROPERTIES: Use specific names to avoid QML Binding Loops
    // hModel is the HistoryModel (QAbstractTableModel)
    // cModel is the SalesModel (used here only for fetching details)
    property HistoryModel hModel: null
    property SalesModel cModel: null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // --- Header Section ---
        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 2
                Text {
                    text: "Order History"
                    font.pixelSize: 28
                    font.bold: true
                    color: "#2c3e50"
                }
                Text {
                    text: "View and manage past transactions"
                    font.pixelSize: 14
                    color: "#7f8c8d"
                }
            }

            Item { Layout.fillWidth: true }

            Button {
                text: "↻ Refresh List"
                flat: false
                onClicked: if (root.hModel) root.hModel.loadOrderHistory()
            }
        }

        // --- History Table/List ---
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            radius: 8
            border.color: "#e0e0e0"
            clip: true

            ListView {
                id: historyListView
                anchors.fill: parent
                model: root.hModel
                boundsBehavior: Flickable.StopAtBounds

                // Header for the list columns
                headerPositioning: ListView.OverlayHeader
                header: Rectangle {
                    width: historyListView.width
                    height: 40
                    color: "#f1f2f6"
                    z: 2
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 15
                        anchors.rightMargin: 15
                        Text { text: "Order Reference"; font.bold: true; Layout.fillWidth: true }
                        Text { text: "Date & Time"; font.bold: true; Layout.preferredWidth: 150 }
                        Text { text: "Action"; font.bold: true; Layout.preferredWidth: 80; horizontalAlignment: Text.AlignRight }
                    }
                }

                delegate: ItemDelegate {
                    width: historyListView.width
                    height: 60

                    background: Rectangle {
                        color: hovered ? "#f9f9f9" : "transparent"
                        Rectangle {
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: 1
                            color: "#eeeeee"
                        }
                    }

                    contentItem: RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 15
                        anchors.rightMargin: 15

                        ColumnLayout {
                            spacing: 2
                            Layout.fillWidth: true
                            Text {
                                // Using displayTitle role from HistoryModel
                                text: model.displayTitle
                                font.pixelSize: 16
                                font.bold: true
                                color: "#34495e"
                            }
                            Text {
                                text: "ID: " + model.orderId
                                font.pixelSize: 11
                                color: "#95a5a6"
                            }
                        }

                        Text {
                            text: model.date
                            Layout.preferredWidth: 150
                            color: "#7f8c8d"
                        }

                        Image {
                            source: "assets/icons/details.png" // Replace with your icon
                            Layout.preferredWidth: 20
                            Layout.preferredHeight: 20
                            fillMode: Image.PreserveAspectFit
                            opacity: 0.5
                        }
                    }

                    onClicked: {
                        if (root.cModel) {
                            // Tell the SalesModel to clear current cart and load these specific details
                            root.cModel.viewOrderDetails(model.orderId)

                            // Push to details screen
                            contentStack.push(orderDetailsView, {
                                "currentOrderId": model.orderId,
                                "salesModel": root.cModel
                            })
                        }
                    }
                }

                // Placeholder when list is empty
                Label {
                    anchors.centerIn: parent
                    text: "No orders found in database."
                    visible: historyListView.count === 0
                    color: "#bdc3c7"
                    font.pixelSize: 18
                }
            }
        }

        // --- Footer Info ---
        RowLayout {
            Layout.fillWidth: true
            Text {
                color: "#95a5a6"
                font.pixelSize: 12
                text: "Total Database Records: " + (root.hModel ? root.hModel.rowCount() : 0)
            }
        }
    }

    // Automatically load history when the component is created
    Component.onCompleted: {
        if (root.hModel) {
            root.hModel.loadOrderHistory()
        }
    }
}
