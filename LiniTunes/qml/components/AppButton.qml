import QtQuick

Rectangle {
    id: root

    property string label: "Button"
    property bool destructive: false
    property bool primary: false
    property var colors: ({})
    signal clicked()

    width: Math.max(118, buttonText.implicitWidth + 28)
    height: 32
    radius: 5
    opacity: enabled ? 1.0 : 0.45
    border.width: destructive || primary ? 0 : 1
    border.color: root.colors.cardStroke ?? "transparent"
    gradient: Gradient {
        orientation: Gradient.Vertical
        GradientStop { position: 1; color: root.topColor() }
        GradientStop { position: 0; color: root.bottomColor() }
    }

    function topColor() {
        if (destructive) return root.colors.darkRed ?? "#c01c28"
        if (primary) return root.colors.accent ?? "#3284ff"
        return root.colors.cardBackgroundTop ?? "transparent"
    }

    function bottomColor() {
        if (destructive) return root.colors.red ?? "#e01b24"
        if (primary) return root.colors.accent ?? "#3284ff"
        return root.colors.cardBackgroundBottom ?? "transparent"
    }

    function labelColor() {
        if (destructive || primary) return "#ffffff"
        return root.colors.textPrimary ?? "black"
    }

    Text {
        id: buttonText
        anchors.centerIn: parent
        text: root.label
        color: root.labelColor()
        font.weight: Font.DemiBold
        font.family: AppFontFamily
        font.pixelSize: 13
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.enabled
        onPressed: root.opacity = 0.7
        onCanceled: root.opacity = root.enabled ? 1.0 : 0.45
        onReleased: {
            root.opacity = root.enabled ? 1.0 : 0.45
            root.clicked()
        }
    }

}
