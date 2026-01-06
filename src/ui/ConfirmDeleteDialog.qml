import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.Yes | Dialog.No

    // --- Dynamic Properties ---
    property string itemLabel: ""      // The name of the thing being deleted (e.g., "Pizza")
    property var targetId: null        // The ID (int) or Name (string) to delete
    property var targetModel: null     // The C++ model instance (catModel or itemModel)
    property string deleteMethod: ""   // The name of the C++ function to call

    // Dynamic sizing (Dark Red Glassy Theme)
    width: Math.min(parent.width * 0.8, 400)
    height: 200

    background: Rectangle {
        color: Qt.rgba(0.2, 0.02, 0.02, 0.95) // Dark Glassy Red
        border.color: Qt.rgba(1, 1, 1, 0.2)
        radius: 12
    }

    contentItem: ColumnLayout {
        spacing: 15
        anchors.margins: 20

        Text {
            text: "Confirm Deletion"
            color: "white"
            font.bold: true
            font.pixelSize: 18
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "Are you sure you want to delete <b>" + root.itemLabel + "</b>?"
            color: "#cccccc"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }
    }

    // --- Unified Logic Execution ---
    onAccepted: {
        if (targetModel && targetId !== null && deleteMethod !== "") {
            // Dynamically call the C++ method (deleteItem or deleteCategory)
            targetModel[deleteMethod](targetId);

            // Cleanup after deletion
            root.targetId = null;
            root.itemLabel = "";
        }
    }
}
