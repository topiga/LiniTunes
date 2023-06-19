import QtQuick 2.0
import QtQuick.Controls 2.0

Item {
    Rectangle {
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
        color: rootWindow.color_app
        Label {
            id: device_name
        }
    }
}
