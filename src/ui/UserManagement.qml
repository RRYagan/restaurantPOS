import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.UI

Rectangle {
    id: userMgmtRoot
    color: "transparent" // Reveal main app background

    property var staffModel
    readonly property bool isManager: staffModel ? staffModel.isAdmin : false

    // --- External Components ---
    ConfirmDeleteDialog { id: userDeleteDialog }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // --- Header Section ---
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Text {
                    text: "Staff Management"
                    font.pixelSize: 24; font.bold: true; color: "white"
                }
                Text {
                    text: "Manage system access and roles"
                    font.pixelSize: 14; color: "#bdc3c7"
                }
            }
            Item { Layout.fillWidth: true }

            Button {
                text: "+ Add New Staff"
                visible: userMgmtRoot.isManager
                palette.button: "#c0392b"
                palette.buttonText: "white"
                onClicked: addUserDialog.open()
            }
        }

        // --- User List ---
        ListView {
            id: userListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: staffModel
            clip: true
            spacing: 12

            delegate: Rectangle {
                width: userListView.width
                height: 80
                color: Qt.rgba(0.15, 0.02, 0.02, 0.8) // Dark red glassy
                radius: 12
                border.color: Qt.rgba(1, 1, 1, 0.1)

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 20

                    // Avatar Circle
                    Rectangle {
                        width: 45; height: 45; radius: 22.5
                        color: Qt.rgba(1, 1, 1, 0.1)
                        Text {
                            anchors.centerIn: parent
                            text: model.username ? model.username.charAt(0).toUpperCase() : "?"
                            color: "white"; font.bold: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 2
                        Text {
                            text: model.username; color: "white"
                            font.bold: true; font.pixelSize: 16
                        }
                        Text {
                            text: "Role: " + (model.role || "staff")
                            color: "#95a5a6"; font.pixelSize: 12
                        }
                    }

                    Item { Layout.fillWidth: true } // Spacer

                    // --- Action Menu Button ---
                    Button {
                        id: userOptBtn
                        text: "⋮"
                        flat: true; font.pixelSize: 22; palette.buttonText: "white"
                        visible: userMgmtRoot.isManager && model.username !== "admin"
                        onClicked: userMenu.open()

                        Menu {
                            id: userMenu
                            y: userOptBtn.height
                            x: -width + userOptBtn.width

                            background: Rectangle {
                                color: "#2c0202"
                                border.color: Qt.rgba(1, 1, 1, 0.2)
                                radius: 8
                            }

                            MenuItem {
                                text: "📝 Edit Permissions"
                                contentItem: Text { text: parent.text; color: "white" }
                                onClicked: console.log("Edit user:", model.username)
                            }

                            MenuItem {
                                text: "🗑 Remove Staff"
                                contentItem: Text { text: parent.text; color: "#ff7675" }
                                onClicked: {
                                    userDeleteDialog.itemLabel = model.username;
                                    userDeleteDialog.targetId = model.id;
                                    userDeleteDialog.targetModel = staffModel;
                                    userDeleteDialog.deleteMethod = "deleteUser";
                                    userDeleteDialog.open();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // --- Add Staff Dialog ---
    Dialog {
        id: addUserDialog
        title: "Create Staff Account"
        anchors.centerIn: parent
        modal: true
        width: Math.min(parent.width * 0.4, 400)

        background: Rectangle {
            color: Qt.rgba(0.2, 0.02, 0.02, 0.95); radius: 15
            border.color: Qt.rgba(1, 1, 1, 0.2)
        }

        contentItem: ColumnLayout {
            spacing: 15; Layout.margins: 20
            TextField {
                id: nameIn; placeholderText: "Username"; Layout.fillWidth: true
                color: "white"
                background: Rectangle { color: Qt.rgba(1, 1, 1, 0.1); radius: 8 }
            }
            TextField {
                id: passIn; placeholderText: "Password"; Layout.fillWidth: true
                echoMode: TextInput.Password; color: "white"
                background: Rectangle { color: Qt.rgba(1, 1, 1, 0.1); radius: 8 }
            }
            ComboBox {
                id: roleIn; Layout.fillWidth: true
                model: ["staff", "manager"]
            }
        }

        footer: DialogButtonBox {
            background: Rectangle { color: "transparent" }
            Button {
                text: "Save Account"; highlighted: true; palette.button: "#c0392b"
                onClicked: {
                    if (staffModel.addUser(nameIn.text, passIn.text, roleIn.currentText)) {
                        nameIn.clear(); passIn.clear(); addUserDialog.close();
                    }
                }
            }
        }
    }
}
