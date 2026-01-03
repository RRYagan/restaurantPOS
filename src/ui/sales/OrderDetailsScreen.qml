import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.Sales 1.0

Rectangle {
    property string currentOrderId: ""
    property OrderDetailModel detailsModel: null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25

        RowLayout {
            Button { text: "← Back"; onClicked: contentStack.pop() }
            Text { text: "Details for Order #" + currentOrderId; font.pixelSize: 22; font.bold: true }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: detailsModel
            delegate: RowLayout {
                width: parent.width
                Text { text: model.quantity + "x"; Layout.preferredWidth: 40 }
                Text { text: model.name; Layout.fillWidth: true }
                Text { text: model.price.formatted }
            }
        }
    }
}
