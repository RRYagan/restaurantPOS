import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import POS.Sales 1.0

Rectangle {
    id: userMgmtRoot
    color: "#f4f7f6" // Matches the UI theme [cite: 1]

    // Instantiate the UserModel to handle the staff list
    property UserView staffModel

    // Reference the global DatabaseManager to check permissions [cite: 1]
    readonly property bool isManager: staffModel.isAdmin

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // --- Header Section ---
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Text {
                    text: "Staff Management";
                    font.pixelSize: 24; font.bold: true; color: "#2c3e50"
                }
                Text {
                    text: "Manage system access and roles";
                    font.pixelSize: 14; color: "#7f8c8d"
                }
            }
            Item { Layout.fillWidth: true }

            // Add Staff button is restricted to managers
            Button {
                text: "Add New Staff"
                highlighted: true
                visible: userMgmtRoot.isManager
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
            spacing: 10

            delegate: Rectangle {
                width: userListView.width
                height: 70
                color: "white"
                radius: 8
                border.color: "#dcdde1"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 15

                    // User Info display using model roles
                    ColumnLayout {
                        Layout.fillWidth: true
                        Text {
                            text: model.username;
                            font.bold: true; font.pixelSize: 16
                        }
                        Text {
                            text: "Role: " + model.role;
                            color: "#666"; font.pixelSize: 12
                        }
                    }

                    // Remove button with permission check and ID-based deletion [cite: 3, 8]
                    Button {
                        text: "Remove"
                        visible: userMgmtRoot.isManager && model.username !== "admin"
                        onClicked: staffModel.deleteUser(model.id)
                    }
                }
            }
        }
    }

    // --- Add User Dialog ---
    Dialog {
        id: addUserDialog
        title: "Create Staff Account"
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        ColumnLayout {
            spacing: 15
            TextField {
                id: nameIn; placeholderText: "Username"; Layout.fillWidth: true
            }
            TextField {
                id: passIn; placeholderText: "Password";
                echoMode: TextInput.Password; Layout.fillWidth: true
            }
            ComboBox {
                id: roleIn; Layout.fillWidth: true
                model: ["staff", "manager"]
            }
        }

        onAccepted: {
            // Calls invokable C++ method which handles hashing
            staffModel.addUser(nameIn.text, passIn.text, roleIn.currentText)
            nameIn.clear(); passIn.clear();
        }
    }
}
