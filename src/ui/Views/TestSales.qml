import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI 1.0

Column {
    OrderViewController {
        id: orderController
    }

    ListView {
        width: 400; height: 300
        model: orderController.orderModel
        delegate: Text {
            text: "Table " + tableNumber + ": $" + (totalAmount/100) + " [" + status + "]"
        }
    }

    Button {
        text: "Add Random Order"
        onClicked: orderController.createNewOrder(Math.floor(Math.random() * 10), 2500)
    }
}
