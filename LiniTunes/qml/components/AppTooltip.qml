import QtQuick
import QtQuick.Controls

ToolTip {
    id: root
    property string tipText: ""
    required property color textColor
    required property color backgroundStroke
    required property color backgroundFill
    property bool hovered: false
    property double verticalMargin: 45

    visible: hovered
    text: tipText
    delay: 400
    y: - verticalMargin

    contentItem: Text {
        text: root.text
        color: root.textColor
        font.pixelSize: 12
        font.family: AppFontFamily
    }

    background: Rectangle {
        border.width: 0
        radius: 5
        gradient: Gradient {
            GradientStop { position: 0; color: root.backgroundStroke }
            GradientStop { position: 1; color: "#02000000" }
            orientation: Gradient.Vertical
        }
        Rectangle {
            anchors {
                fill: parent
                topMargin: 1; leftMargin: 1; bottomMargin: 1; rightMargin: 1
            }
            radius: 5
            border.width: 0
            color: root.backgroundFill
        }
    }
}