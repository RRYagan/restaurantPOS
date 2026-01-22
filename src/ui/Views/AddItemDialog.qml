import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Dialog {
    id: root
    title: isEditMode ? ("Edit " + itemData.name) : ("Add to " + category)
    modal: true

    property string category: "All"
    property var itemModel

    property var itemData: null
    readonly property bool isEditMode: itemData !== null

    property string selectedImagePath: "qrc:/assets/icons/default.svg"

    width: Math.min(Math.max(parent.width * 0.5, 450), 850)
    height: Math.min(Math.max(parent.height * 0.7, 550), 800)
    anchors.centerIn: parent

    background: Rectangle {
        color: Qt.rgba(0.15, 0.01, 0.01, 0.95) // Dark Glassy Red
        border.color: Qt.rgba(1, 1, 1, 0.2)
        radius: 15
    }

    FileDialog {
        id: imagePicker
        title: "Select Item Image"
        nameFilters: ["Image files (*.png *.jpg *.svg)"]
        onAccepted: root.selectedImagePath = imagePicker.selectedFile
    }

    contentItem: ColumnLayout {
        spacing: 20
        Layout.margins: 30

        // Image Preview Area
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 10
            Rectangle {
                width: 120; height: 120; radius: 10
                color: Qt.rgba(1, 1, 1, 0.05); border.color: Qt.rgba(1, 1, 1, 0.3)
                clip: true
                Image {
                    anchors.fill: parent; anchors.margins: 5
                    source: root.selectedImagePath; fillMode: Image.PreserveAspectFit
                }
            }
            Button {
                text: "Upload Image"
                onClicked: imagePicker.open()
            }
        }

        TextField {
            id: nameIn
            placeholderText: "Item Name"
            text: isEditMode ? itemData.name : ""
            color: "white"
            Layout.fillWidth: true
            background: Rectangle { color: Qt.rgba(1, 1, 1, 0.1); radius: 8 }
        }

        TextField {
            id: priceIn
            placeholderText: "Price (KSH)"
            text: isEditMode ? ((itemData.price_cents)/100).toString() : ""
            color: "white"
            Layout.fillWidth: true
            inputMethodHints: Qt.ImhDigitsOnly
            background: Rectangle { color: Qt.rgba(1, 1, 1, 0.1); radius: 8 }
        }

        Text {
            id: validationError
            text: "Please provide a name and valid price."
            color: "#ff8888"; visible: false
        }
    }

    footer: DialogButtonBox {
        background: Rectangle { color: "transparent" }
        Button {
                        text: "Cancel"
                        onClicked: addItemDialog.close()
                    }
        Button {
            text: "Save"
            highlighted: true
            onClicked: {
                if (nameIn.text.trim() !== "" && !isNaN(parseInt(priceIn.text))) {
                    if (itemModel.addMenuItem(nameIn.text, category, parseInt(priceIn.text)*100, root.selectedImagePath)) {
                        nameIn.clear(); priceIn.clear();
                        root.selectedImagePath = "qrc:/assets/icons/default.svg";
                        root.close();
                    }
                } else {
                    validationError.visible = true;
                }
            }
        }

    }
}
