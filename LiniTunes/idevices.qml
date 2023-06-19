import QtQuick 2.0

Item {
    Rectangle {
        anchors.fill: parent
        color: rootWindow.color_app
        Rectangle {
            height: 65
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            color: rootWindow.color_sidebar
        }
    }
}
