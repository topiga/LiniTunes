import QtQuick

Rectangle {
    id: root

    property int strokeRadius: 5
    property real innerMargin: 1
    required property color strokeColor

    radius: strokeRadius
    border.width: 0

    gradient: Gradient {
        orientation: Gradient.Vertical
        GradientStop { position: 0; color: root.strokeColor }
        GradientStop { position: 1; color: "#02000000" }
    }

    default property alias content: innerPlaceholder.data

    Item {
        id: innerPlaceholder
        anchors {
            fill: parent
            topMargin: root.innerMargin; leftMargin: root.innerMargin
            bottomMargin: root.innerMargin; rightMargin: root.innerMargin
        }
    }
}