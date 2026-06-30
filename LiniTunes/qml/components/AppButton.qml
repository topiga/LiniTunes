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
    border.width: destructive || primary ? 0 : (root.activeFocus ? 2 : 1)
    border.color: root.activeFocus ? (root.colors.accent ?? "#3284ff") : (root.colors.cardStroke ?? "transparent")
    gradient: Gradient {
        orientation: Gradient.Vertical
        GradientStop { position: 1; color: root.topColor() }
        GradientStop { position: 0; color: root.bottomColor() }
    }

    focus: true
    activeFocusOnTab: true

    Keys.onReturnPressed: if (root.enabled) root.clicked()
    Keys.onEnterPressed: if (root.enabled) root.clicked()
    Keys.onSpacePressed: if (root.enabled) root.clicked()

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
        onPressed: {
            root.opacity = 0.7
            root.forceActiveFocus()
        }
        onCanceled: root.opacity = root.enabled ? 1.0 : 0.45
        onReleased: {
            root.opacity = root.enabled ? 1.0 : 0.45
            root.clicked()
        }
    }

    Connections {
        target: root.Keys
        function onPressed(event) {
            if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) && root.enabled)
                root.opacity = 0.7
        }
        function onReleased(event) {
            if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) && root.enabled)
                root.opacity = root.enabled ? 1.0 : 0.45
        }
    }
}
