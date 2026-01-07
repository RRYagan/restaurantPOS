import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    // Dynamic Title based on mode [cite: 54]
    title: isEditMode ? "Edit Stock Item" : "Add New Stock"
    modal: true
    anchors.centerIn: parent

    standardButtons: Dialog.Save | Dialog.Cancel

    property bool isEditMode: false
    property var targetData: null // Stores the full item map when editing

    width: Math.min(parent.width * 0.9, 400)
    x: {
            var preferredX = (parent.width - width) / 2 // Default to center
            // Clamp: Math.max(0, Math.min(preferredX, ScreenWidth - DialogWidth))
            return Math.max(10, Math.min(preferredX, parent.width - width - 10))
        }

        y: {
            var preferredY = (parent.height - height) / 2
            return Math.max(10, Math.min(preferredY, parent.height - height - 10))
        }

    background: Rectangle {
        color: "#2c0505" // Matches your dark glassy red theme [cite: 8, 30, 53]
        border.color: Qt.rgba(1, 1, 1, 0.2)
        radius: 12
    }

    contentItem: ColumnLayout {
        spacing: 15
        anchors.margins: 10

        TextField {
            id: nameField
            placeholderText: "Item Name"
            Layout.fillWidth: true
            color: "white"
            background: Rectangle {
                color: Qt.rgba(1, 1, 1, 0.1)
                radius: 6
            }
        }

        RowLayout {
            spacing: 10
            ColumnLayout {
                Text { text: "Quantity"; color: "#95a5a6"; font.pixelSize: 12 }
                SpinBox {
                    id: qtyField
                    editable: true
                    from: 0
                    to: 9999
                    Layout.fillWidth: true
                }
            }
            ColumnLayout {
                Text { text: "Unit"; color: "#95a5a6"; font.pixelSize: 12 }
                ComboBox {
                    id: unitField
                    model: ["pcs", "kg", "ltr", "box", "pkt"]
                    Layout.fillWidth: true
                }
            }
        }
    }

    // Function to prepare the dialog for editing [cite: 52]
    function openForEdit(data) {
        if (!data) {
                    console.error("StockDialog: Received null data for edit");
                    return;
                }
        isEditMode = true
        targetData = data
        nameField.text = data.name
        qtyField.value = data.quantity
        var idx = unitField.find(data.unit || "pcs")
        unitField.currentIndex = idx !== -1 ? idx : 0
        open()
    }

    // Function to prepare the dialog for adding [cite: 52]
    function openForAdd() {
        isEditMode = false
        targetData = null
        nameField.clear()
        qtyField.value = 0
        open()
    }

    onAccepted: {
        if (isEditMode && targetData) {
            // Call the C++ update method
            globalInventoryModel.updateStock(targetData.id, nameField.text, qtyField.value, unitField.currentText)
        } else {
            // Call the C++ add method
            globalInventoryModel.addStock(nameField.text, qtyField.value, unitField.currentText)
        }
    }
}
