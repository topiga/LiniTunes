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
        color: "#303030"
        Label {
            id: device_name
        }
    }
}
