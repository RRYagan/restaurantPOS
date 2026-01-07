import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
    id: control
    property string itemName: ""
    property string priceCents: ""
    property string iconSource: ""

    // Set fixed dimensions to ensure all cards fit the view uniformly
    implicitWidth: 160
    implicitHeight: 210



    background: Rectangle {
        anchors.fill: parent
        // White background with 15% opacity to show the Main.qml background design
        // Darker opacity when pressed (down)
        color: control.down ? Qt.rgba(1, 1, 1, 0.25) : Qt.rgba(1, 1, 1, 0.15)
        radius: 12
        // Subtle white border to define the card against the background
        border.color: control.visualFocus ? "#3498db" : Qt.rgba(1, 1, 1, 0.2)
        border.width: 1
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Icon Area - Uniformly sized and centered
        Image {
            source: control.iconSource || "qrc:/qt/qml/POS/UI/assets/icons/default.png"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 64
            Layout.preferredHeight: 64
            fillMode: Image.PreserveAspectFit
            opacity: 0.9
        }

        // Flexible spacer to push text to the bottom area for alignment
        Item { Layout.fillHeight: true }

        // Item Name - Centered, Bold, and Uniformly sized
        Text {
            text: control.itemName
            font.pixelSize: 15
            font.bold: true
            color: "white" // Contrast for dark red/transparent theme
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }

        // Price - Centered at the bottom
        Text {
            text: control.priceCents ? (control.priceCents)/100 + " Ksh." : "0.00 Ksh."
            color: "#2ecc71" // Vibrant green to remain visible on the design
            font.pixelSize: 14
            font.bold: true
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
