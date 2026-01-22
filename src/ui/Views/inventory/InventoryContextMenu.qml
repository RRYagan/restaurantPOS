import QtQuick
import QtQuick.Controls

Menu {
    id: root
    property var targetData

    // Signals to communicate with the parent
    signal editRequested(var data)
    signal deleteRequested(var data)
    signal updateRequested(var data)

    width: 170

    background: Rectangle {
        color: "#2c3e50"
        border.color: "#34495e"
        radius: 6
        layer.enabled: true
    }

    MenuItem {
        text: "📝 Edit Item"
        onTriggered: root.editRequested(targetData)
    }

    MenuItem {
        text: "🔄 +1 Stock"
        onTriggered: root.updateRequested(targetData)
    }

    MenuSeparator { contentItem: Rectangle { implicitHeight: 1; color: "#34495e" } }

    MenuItem {
        text: "🗑️ Delete"
        palette.windowText: "#ff7675"
        onTriggered: root.deleteRequested(targetData)
    }
}
