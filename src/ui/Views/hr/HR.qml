import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: hrModule
    anchors.fill: parent
    color: "#121212"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 1. Sidebar Navigation
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 240
            color: "#1A1A1A"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 15

                Image {
                    source: "qrc:/assets/banana_logo.png"
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    text: "HR MANAGEMENT"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#FFE135"
                }

                // Nav Buttons - These change the 'stack.currentIndex'
                HRNavButton {
                    text: "Hiring"
                    onClicked: stack.currentIndex = 0
                }
                HRNavButton {
                    text: "Payroll"
                    onClicked: stack.currentIndex = 1
                }
                HRNavButton {
                    text: "Leave / Off-days"
                    onClicked: stack.currentIndex = 2
                }
                HRNavButton {
                    text: "Duty Rota"
                    onClicked: stack.currentIndex = 3
                }

                Item { Layout.fillHeight: true } // Pushes content up
            }
        }

        // 2. Main Content Area (The Viewport)
        StackLayout {
            id: stack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: 0 // Controlled by sidebar buttons

            // These files (HiringView.qml, etc.) must be in the same folder
            HiringView {
                id: hiringPage
            }

            PayrollView {
                id: payrollPage
            }

            LeaveView {
                id: leavePage
            }

            RotaView {
                id: rotaPage
            }
        }
    }
}
