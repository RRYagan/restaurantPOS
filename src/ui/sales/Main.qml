import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.Sales 1.0

ApplicationWindow {
    id: window
    width: 1080
    height: 720
    visible: true
    title: "Restaurant POS"

    // --- Global Model Instances ---
    SalesModel { id: globalCartModel }
    HistoryModel { id: globalHistoryModel }
    OrderDetailModel { id: globalOrderDetailModel }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Sidebar Navigation
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 200
            color: "#2c3e50"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15

                Label {
                    text: "POS SYSTEM"
                    color: "white"
                    font.bold: true
                    font.pixelSize: 20
                    Layout.alignment: Qt.AlignHCenter
                    Layout.bottomMargin: 20
                }

                Button {
                    text: "Menu / Order"
                    Layout.fillWidth: true
                    onClicked: contentStack.replace(menuView)
                }

                Button {
                    text: "Order History"
                    Layout.fillWidth: true
                    onClicked: {
                        globalHistoryModel.loadOrderHistory();
                        contentStack.replace(salesView);

                    }
                }

                Button {
                    text: "Settings"
                    Layout.fillWidth: true
                    onClicked: contentStack.replace(setupView)
                }

                Item { Layout.fillHeight: true }
            }
        }

        // Main Content Area
        StackView {
            id: contentStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            initialItem: menuView

            Component { id: menuView; MenuScreen { salesModel: globalCartModel } }
            Component { id: salesView; SalesScreen { hModel: globalHistoryModel } }
            Component { id: orderDetailsView; OrderDetailsScreen { detailsModel: globalOrderDetailModel } }
            Component { id: setupView; SetupScreen { } }
        }
    }
}
