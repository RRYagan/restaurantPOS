import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    title: isEditMode ? "Edit Stock Item" : "Add New Stock"
    modal: true
    anchors.centerIn: parent

    standardButtons: Dialog.Save | Dialog.Cancel

    property bool isEditMode: false
    property var targetData: null

    // Access the controller passed from the parent page
    // Ensure 'invModel' (the InventoryViewController) is accessible here
    property var controller: inventoryPage.invModel

    width: Math.min(parent.width * 0.9, 400)

    background: Rectangle {
        color: "#2c0505"
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
                Text { text: "Packages"; color: "#95a5a6"; font.pixelSize: 12 }
                SpinBox {
                    id: pkgField
                    editable: true
                    from: 0
                    to: 9999
                    value: 0
                    Layout.fillWidth: true
                }
            }
            ColumnLayout {
                Text { text: "Package Unit"; color: "#95a5a6"; font.pixelSize: 12 }
                ComboBox {
                    id: pkgUnitField
                    // Using IDs to match your quantityUnitId schema
                    model: ListModel {
                        ListElement { text: "Bottles"; value: 1 }
                        ListElement { text: "Bags"; value: 2 }
                        ListElement { text: "Bundle"; value: 3 }
                    }
                    textRole: "text"
                    valueRole: "value"
                    Layout.fillWidth: true
                }
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
                    value: 0
                    Layout.fillWidth: true
                }
            }
            ColumnLayout {
                Text { text: "Unit ID"; color: "#95a5a6"; font.pixelSize: 12 }
                ComboBox {
                    id: unitField
                    // Using IDs to match your quantityUnitId schema
                    model: ListModel {
                        ListElement { text: "pcs"; value: 1 }
                        ListElement { text: "kg"; value: 2 }
                        ListElement { text: "ltr"; value: 3 }
                    }
                    textRole: "text"
                    valueRole: "value"
                    Layout.fillWidth: true
                }
            }
        }
    }

    function openForEdit(data) {
        if (!data) return;
        isEditMode = true
        targetData = data

        // Match the updated roles from your C++ model
        nameField.text = data.name
        pkgField.value = data.packagesAvailable
        // Find unit index by ID
        var pkg_idx = pkgUnitField.indexOfValue(data.packagingUnitId)
        pkgUnitField.currentIndex = pkg_idx !== -1 ? pkg_idx : 0

        qtyField.value = data.quantityAvailable
        // Find unit index by ID
        var idx = unitField.indexOfValue(data.quantityUnitId)
        unitField.currentIndex = idx !== -1 ? idx : 0
        open()
    }

    function openForAdd() {
        isEditMode = false
        targetData = null
        nameField.clear()
        pkgField.value = 0
        pkgUnitField.currentIndex=0
        qtyField.value = 0
        unitField.currentIndex = 0
        open()
    }

    onAccepted: {
        // Prepare the Map/Object for the C++ Controller
        var payload = {
            "name": nameField.text,
            "packagesAvailable": pkgField.value,
            "packagingUnitId": pkgUnitField,
            "quantityPerPackage":10,
            "quantityAvailable": qtyField.value,
            "quantityUnitId": unitField.currentValue
        }

        if (isEditMode && targetData) {
            payload["id"] = targetData.id // Include ID for updates
            controller.updateStock(payload)
        } else {
            controller.addStock(payload)
        }
    }
}
