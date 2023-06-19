import QtQuick 2.0

Item {
    Rectangle {
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
        color: rootWindow.color_app
    }
    Text {
        id: hello_test
        text: qsTr("Hello, LiniTunes")
        color: rootWindow.color_text
        anchors {
            verticalCenter: parent.verticalCenter
            horizontalCenter: parent.horizontalCenter
        }
    }
}
