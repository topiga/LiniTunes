import QtQuick 2.0

Item {
    Rectangle {
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
        color: "#FFFFFF"
    }
    Text {
        id: hello_test
        text: qsTr("Hello, LiniTunes")
        color: "#2C2C2C"
        anchors {
            verticalCenter: parent.verticalCenter
            horizontalCenter: parent.horizontalCenter
        }
    }
}
