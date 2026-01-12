import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    signal menuSetupClicked()
    signal floorPlanClicked()

    Flow {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 20

        // Menu Setup Link
        SetupLink {
            title: "Menu Setup"
            icon: "🍴"
            onClicked: menuSetupClicked()
        }

        // Floor Plan Link
        SetupLink {
            title: "Table/Floor Plan"
            icon: "🏠"
            onClicked: floorPlanClicked()
        }
    }

    // Local helper for link cards using whiteTheme [cite: 43, 44]
    component SetupLink : Rectangle {
        property string title
        property string icon
        signal clicked()

        width: 200; height: 150
        color: whiteTheme.surface
        border.color: whiteTheme.border
        radius: whiteTheme.cardRadius

        MouseArea {
            anchors.fill: parent;
            onClicked: parent.clicked()
            hoverEnabled: true
            onEntered: parent.color = whiteTheme.surfaceHighlight
            onExited: parent.color = whiteTheme.surface
        }

        ColumnLayout {
            anchors.centerIn: parent
            Text { text: icon; font.pixelSize: 40; Layout.alignment: Qt.AlignHCenter }
            Text { text: title; color: whiteTheme.textMain; font.bold: true; Layout.alignment: Qt.AlignHCenter }
        }
    }
}
