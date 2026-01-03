import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.Sales 1.0

Rectangle {
    id: root
    property HistoryModel hModel: null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20

        Text { text: "Order History"; font.pixelSize: 28; font.bold: true }

        ListView {
            id: historyList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: hModel
            spacing: 5
            clip: true
            delegate: ItemDelegate {
                width: parent.width
                contentItem: RowLayout {
                    Text { text: model.displayTitle; font.bold: true; Layout.fillWidth: true }
                    Text { text: model.date; color: "#7f8c8d" }
                }
                onClicked: {
                    globalOrderDetailModel.loadOrder(model.orderId);
                    contentStack.push(orderDetailsView, { "currentOrderId": model.orderId });
                }
            }
        }
    }
}
