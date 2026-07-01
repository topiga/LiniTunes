import QtQuick

Rectangle {
    id: root

    property string title: ""
    property var colors: ({})
    default property alias content: cardContent.data

    width: parent ? parent.width : 0
    implicitHeight: cardTitle.implicitHeight + cardContent.implicitHeight + 30
    radius: 8
    border.width: 0
    gradient: Gradient {
        orientation: Gradient.Vertical
        GradientStop { position: 0; color: root.colors.cardStroke ?? "transparent" }
        GradientStop { position: 1.5; color: "#02000000" }
    }

    Rectangle {
        id: cardInner
        anchors.fill: parent
        anchors.margins: 1
        radius: 8
        border.width: 0
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 1.2; color: root.colors.cardBackgroundTop ?? "transparent" }
            GradientStop { position: 0; color: root.colors.cardBackgroundBottom ?? "transparent" }
        }
    }

    Text {
        id: cardTitle
        opacity: 0.5
        color: root.colors.textPrimary ?? "black"
        text: root.title
        font.pixelSize: 12
        font.family: AppFontFamily
        anchors {
            left: cardInner.left
            top: cardInner.top
            leftMargin: 10
            topMargin: 8
        }
    }

    Column {
        id: cardContent
        anchors {
            left: cardInner.left
            right: cardInner.right
            top: cardTitle.bottom
            margins: 14
            topMargin: 8
        }
        spacing: 10
    }
}
