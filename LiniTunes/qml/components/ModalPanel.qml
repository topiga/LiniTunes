import QtQuick
import QtQuick.Controls

Popup {
    id: root

    property string title: ""
    property var colors: ({})
    default property alias content: popupContent.data

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    readonly property var win: root.parent ? root.parent.Window.window : null
    readonly property real winW: win ? win.width : 960
    readonly property real winH: win ? win.height : 630
    readonly property real parentX: root.parent ? root.parent.mapToItem(null, 0, 0).x : 0
    readonly property real centeredY: Math.max(24, Math.round((winH - height) / 2))
    readonly property real slideOffset: 8

    width: Math.min(520, winW - 40)
    x: Math.round((winW - width) / 2) - parentX
    y: centeredY
    padding: 0
    scale: 1.0
    opacity: 1.0
    Overlay.modal: Rectangle {
        color: "#80000000"
        opacity: root.opacity

        Behavior on opacity {
            NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
        }
    }
    enter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 220; easing.type: Easing.OutCubic }
            NumberAnimation { property: "scale"; from: 0.96; to: 1.0; duration: 220; easing.type: Easing.OutCubic }
            NumberAnimation { property: "y"; from: root.centeredY + root.slideOffset; to: root.centeredY; duration: 220; easing.type: Easing.OutCubic }
        }
    }
    exit: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 160; easing.type: Easing.InCubic }
            NumberAnimation { property: "scale"; from: 1.0; to: 0.97; duration: 160; easing.type: Easing.InCubic }
            NumberAnimation { property: "y"; from: root.centeredY; to: root.centeredY + root.slideOffset; duration: 160; easing.type: Easing.InCubic }
        }
    }
    background: Rectangle {
        radius: 10
        border.width: 0
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0; color: root.colors.cardStroke ?? "transparent" }
            GradientStop { position: 1; color: "#02000000" }
        }
        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: 10
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop { position: 0; color: root.colors.cardBackgroundTop ?? "transparent" }
                GradientStop { position: 1; color: root.colors.cardBackgroundBottom ?? "transparent" }
            }
        }
    }

    contentItem: Column {
        readonly property real innerWidth: width - leftPadding - rightPadding

        width: root.width
        spacing: 14
        padding: 18

        Text {
            text: root.title
            color: root.colors.textPrimary ?? "black"
            font.pixelSize: 16
            font.family: AppFontFamily
            font.weight: Font.Bold
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            width: parent.innerWidth
        }

        Rectangle {
            width: parent.innerWidth
            height: 1
            color: root.colors.divider ?? "transparent"
        }

        Column {
            id: popupContent
            width: parent.innerWidth
            spacing: 12
        }
    }
}
