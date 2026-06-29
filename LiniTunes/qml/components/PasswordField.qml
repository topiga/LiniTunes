import QtQuick

Rectangle {
    id: root

    property alias text: passwordInput.text
    property alias placeholderText: placeholderLabel.text
    property var colors: ({})

    height: 34
    radius: 6
    color: root.colors.cardBackgroundTop ?? "transparent"
    border.color: root.colors.cardStroke ?? "transparent"
    border.width: 1
    clip: true

    Text {
        id: placeholderLabel
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            leftMargin: 10
            rightMargin: 10
        }
        visible: passwordInput.text.length === 0
        color: root.colors.sideTextInactive ?? "gray"
        font.pixelSize: 12
        font.family: AppFontFamily
        elide: Text.ElideRight
    }

    TextInput {
        id: passwordInput
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            leftMargin: 10
            rightMargin: 10
        }
        height: implicitHeight
        echoMode: TextInput.Password
        color: root.colors.textPrimary ?? "black"
        selectionColor: root.colors.accent ?? "#3284ff"
        selectedTextColor: "#ffffff"
        font.pixelSize: 12
        font.family: AppFontFamily
        activeFocusOnTab: true
        clip: true
    }
}
