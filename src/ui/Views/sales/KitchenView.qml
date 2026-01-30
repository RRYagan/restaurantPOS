import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import POS.UI 1.0

Rectangle {
    id: kitchenRoot
    anchors.fill: parent
    color: "transparent"

    property var _globalSalesModel: null
    property string currentFilter: "ordered"

    // UPDATED: Use a JS Array instead of ListModel to keep Array functions like .some()
    property var ordersArray: []

    function refresh(currentFilter) {
        if (!_globalSalesModel) return;
        // Directly assign the QVariantList from C++ to the JS property
        ordersArray = _globalSalesModel.kitchenOrders(currentFilter);
    }
    // This runs as soon as the KitchenView is created
    Component.onCompleted: {
        if (_globalSalesModel) {
            refresh(currentFilter);
        }
    }

    // This handles updates if the user switches tabs (Ordered -> Preparing)
    onCurrentFilterChanged: refresh(currentFilter)
    // Inside KitchenView.qml
    onVisibleChanged: {
        if (visible) {
            console.log("KDS: Tab opened, refreshing data...");
            refresh(currentFilter);
        }
    }
    // This listens for data changes emitted from C++ (SalesViewController)
    Connections {
        target: _globalSalesModel
        function onKitchenDataChanged() {
            console.log("KDS: Data change detected, refreshing...");
            refresh(currentFilter);
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        RowLayout {
            Layout.fillWidth: true

            Column {
                Text {
                    text: "Kitchen Display System"
                    font.pixelSize: 24; font.bold: true; color: "#2d3436"
                }
                Text {
                    // UPDATED: Use .length for the array
                    text: (ordersArray ? ordersArray.length : 0) + " active orders"
                    font.pixelSize: 14; color: "#636e72"
                }
            }

            Item { Layout.fillWidth: true }
            Button {
                text: "↻ Refresh"
                flat: true
                onClicked: refresh(currentFilter)
                contentItem: Text {
                    text: parent.text
                    color: "#3498db"; font.bold: true
                }
            }
            Item { Layout.fillWidth: true }

            Rectangle {
                width: 300; height: 45; radius: 22; color: "#dfe6e9"
                Row {
                    anchors.centerIn: parent
                    spacing: 5
                    Repeater {
                        model: ["ordered", "preparing", "served"]
                        delegate: Button {
                            flat: true
                            text: modelData
                            onClicked: { currentFilter = modelData; refresh(currentFilter); }
                            contentItem: Text {
                                text: parent.text
                                font.bold: true
                                color: currentFilter === modelData ? "#2d3436" : "#636e72"
                                horizontalAlignment: Text.AlignHCenter
                            }
                            background: Rectangle {
                                implicitWidth: 90; implicitHeight: 35; radius: 18
                                color: currentFilter === modelData ? "white" : "transparent"
                            }
                        }
                    }
                }
            }
        }

        ListView {
            id: mainListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 15
            clip: true
            // UPDATED: Use the JS Array as the model
            model: ordersArray

            delegate: Rectangle {
                id: orderContainer
                width: mainListView.width
                height: cardContent.implicitHeight + 40
                radius: 12
                color: "white"
                border.color: "#dfe6e9"

                // modelData is used when the model is a JS Array
                property var orderData: modelData

                MultiEffect {
                    source: orderContainer
                    anchors.fill: orderContainer
                    shadowEnabled: true
                    shadowBlur: 0.3
                    shadowVerticalOffset: 3
                    z: -1
                }

                ColumnLayout {
                    id: cardContent
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    RowLayout {
                        Layout.fillWidth: true
                        Column {
                            Text {
                                text: "TABLE " + orderData.tableNumber
                                font.pixelSize: 20; font.bold: true; color: "#2d3436"
                            }
                            Text {
                                text: "Received: " + orderData.time
                                font.pixelSize: 12; color: "#b2bec3"
                            }
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: "#" + orderData.orderId
                            font.family: "Monospace"; color: "#636e72"
                        }
                    }

                    ListView {
                        id: innerItemsList
                        Layout.fillWidth: true
                        Layout.preferredHeight: contentHeight
                        interactive: false
                        // UPDATED: Access itemModel from the orderData object
                        model: orderData.itemModel

                        delegate: RowLayout {
                            width: innerItemsList.width
                            height: 45
                            spacing: 15
                            opacity: modelData.status === "served" ? 0.5 : 1.0

                            Rectangle {
                                width: 30; height: 30; radius: 6; color: "#f1f2f6"
                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.quantity
                                    font.bold: true; color: "#2d3436"
                                }
                            }

                            Text {
                                text: modelData.name
                                Layout.fillWidth: true
                                font.pixelSize: 16
                                font.strikeout: modelData.status === "served"
                                color: "#2d3436"
                            }

                            Row {
                                spacing: 8
                                RoundButton {
                                    width: 32; height: 32;
                                    text: modelData.status === "ordered" ? "P" : "S"
                                    font.bold: true
                                    palette.button: modelData.status === "preparing" ? "#f39c12" : "#f1f2f6"
                                    palette.buttonText: modelData.status === "preparing" ? "white" : "#636e72"
                                    onClicked: _globalSalesModel.updateItemStatus(modelData.itemId, "preparing")
                                }
                                RoundButton {
                                    width: 32; height: 32; text: "✓"
                                    font.bold: true
                                    palette.button: modelData.status === "served" ? "#2ecc71" : "#f1f2f6"
                                    palette.buttonText: modelData.status === "served" ? "white" : "#636e72"
                                    onClicked: _globalSalesModel.updateItemStatus(modelData.itemId, "served")
                                }
                            }
                        }
                    }

                    Button {
                        text: "Prepare All"
                        Layout.fillWidth: true
                        palette.button: "#2d3436"
                        palette.buttonText: "white"

                        // UPDATED: Now .some() works because itemModel is a JS Array
                        visible: orderData.itemModel.some(item => item.status === "ordered")

                        onClicked: _globalSalesModel.updateAllStatus(orderData.orderId, "preparing")
                    }
                    Button {
                        text: "Serve All"
                        Layout.fillWidth: true
                        palette.button: "#27ae60"
                        palette.buttonText: "white"

                        // UPDATED: Only show if there are items in 'preparing' state
                        visible: orderData.itemModel.some(item => item.status === "ordered" || item.status === "preparing")

                        onClicked: _globalSalesModel.updateAllStatus(orderData.orderId, "served")
                    }
                }
            }
        }
    }
}
