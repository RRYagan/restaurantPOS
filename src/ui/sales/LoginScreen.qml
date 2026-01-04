import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: loginRoot
    color: "#2c3e50" // Matching your professional dark theme

    // Signal to notify Main.qml to switch views
    signal loginSuccess()

    ColumnLayout {
        anchors.centerIn: parent
        width: 320
        spacing: 20

        Text {
            text: "RESTAURANT POS"
            color: "white"
            font.pixelSize: 28; font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        Rectangle {
            Layout.fillWidth: true; height: 280
            color: "white"; radius: 8

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 30; spacing: 15

                TextField {
                    id: userIn
                    placeholderText: "Username"
                    Layout.fillWidth: true
                    focus: true
                    // Enter moves to password
                    Keys.onReturnPressed: passIn.forceActiveFocus()
                }

                TextField {
                    id: passIn
                    placeholderText: "Password"
                    Layout.fillWidth: true
                    echoMode: TextInput.Password
                    // Enter triggers login
                    Keys.onReturnPressed: loginBtn.clicked()
                }

                Button {
                    id: loginBtn
                    text: "Login"
                    Layout.fillWidth: true; highlighted: true
                    onClicked: {
                        // Using the globalUserModel instance defined in Main.qml
                        if (globalUserModel.login(userIn.text, passIn.text)) {
                            loginRoot.loginSuccess()
                        } else {
                            errorMsg.visible = true
                        }
                    }
                }

                Text {
                    id: errorMsg
                    text: "Invalid username or password"
                    color: "#e74c3c"
                    visible: false
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }
}
