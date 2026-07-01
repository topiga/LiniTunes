import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    // ---- Public properties ----
    property string label: ""
    property string labelFull: ""
    property string tipText: ""
    required property color color1
    required property color color2
    required property color textColor
    required property color tipTextColor
    required property color tipBackgroundStroke
    required property color tipBackgroundFill
    required property color transparentTextColor
    property real barWidth: 0
    property bool textRight: false
    property bool transparent: false
    property int labelMinWidth: 60
    property int fullLabelMinWidth: 100

    // ---- Internal ----
    border.width: 0
    width: barWidth

    gradient: transparent ? null : segGradient
    color: transparent ? "transparent" : "transparent"

    Gradient {
        id: segGradient
        orientation: Gradient.Vertical
        GradientStop { position: 0; color: root.color1 }
        GradientStop { position: 1; color: root.color2 }
    }

    property alias hovered: segMouse.containsMouse

    Text {
        anchors {
            left: root.textRight ? undefined : parent.left
            right: root.textRight ? parent.right : undefined
            leftMargin: root.textRight ? 0 : 6
            rightMargin: root.textRight ? 6 : 0
            verticalCenter: parent.verticalCenter
        }
        text: {
            if (root.textRight) {
                if (parent.width > 70) return root.label + " " + qsTr("free")
                if (parent.width > 40) return root.label
                return ""
            }
            if (parent.width > root.fullLabelMinWidth && root.labelFull !== "") return root.labelFull
            if (parent.width > root.labelMinWidth) return root.label
            return ""
        }
        color: root.transparent ? root.transparentTextColor : root.textColor
        font.pixelSize: 12
        font.family: AppFontFamily
        font.weight: Font.DemiBold
    }

    AppTooltip {
        tipText: root.tipText
        textColor: root.tipTextColor
        backgroundStroke: root.tipBackgroundStroke
        backgroundFill: root.tipBackgroundFill
        hovered: root.hovered
    }

    MouseArea {
        id: segMouse
        anchors.fill: parent
        hoverEnabled: true
    }
}