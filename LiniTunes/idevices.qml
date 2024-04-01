import QtQuick 2.0

Item {
    Rectangle {
        anchors.fill: parent
        color: rootWindow.color_app
        Rectangle {
            height: 75
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            color: rootWindow.color_sidebar
            Image {
                id: img_idevice
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }
                width: 60
                height: 60
                sourceSize.width: 60
                sourceSize.height: 60
                fillMode: Image.PreserveAspectCrop
                smooth: true
                source: DeviceWatcher.device_image
                Connections {
                    target: DeviceWatcher
                    function onCurrentDeviceChanged() {
                        if (img_idevice.source != DeviceWatcher.device_image) {
                            img_idevice.source = DeviceWatcher.device_image
                        }
                    }
                }
            }
            Text {
                id: device_name
                text: "Name: "+DeviceWatcher.device_name
                anchors {
                    left: img_idevice.right
                    top: parent.top
                    leftMargin: 0
                    topMargin: 5
                }
                font {
                    family: "Helvetica"
                    pointSize: 10.5
                }
                color: rootWindow.color_text
                maximumLineCount: 1
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                Connections {
                    target: DeviceWatcher
                    function onCurrentDeviceChanged() {
                        if (DeviceWatcher.device_connected) {
                            if (device_name.text != "Name: "+DeviceWatcher.device_name) {
                                device_name.text = "Name: "+DeviceWatcher.device_name
                            }
                        } else {
                            device_name.text = "Name: Unknown"
                        }
                    }
                }
            }
            Text {
                id: device_info
                property string ecid: "Ecid: "+DeviceWatcher.ecid
                property string udid: "Udid: "+DeviceWatcher.udid
                property string serial: "Serial: "+DeviceWatcher.serial
                text: udid
                anchors {
                    left: img_idevice.right
                    top: device_name.bottom
                    leftMargin: 0
                    topMargin: 0
                }
                font {
    //                bold: true
                    family: "Helvetica"
                    pointSize: 10.5
                }
                color: rootWindow.color_text
                maximumLineCount: 1
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                Connections {
                    target: DeviceWatcher
                    function onCurrentDeviceChanged() {
                        if (DeviceWatcher.device_connected) {
                            ecid = "Ecid: "+DeviceWatcher.ecid
                            udid = "Udid: "+DeviceWatcher.udid
                            serial = "Serial: "+DeviceWatcher.serial
                        }
                    }
                }
            }
        }
    }
}
