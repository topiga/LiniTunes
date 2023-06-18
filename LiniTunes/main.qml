import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Window {
    id: root
    minimumWidth: 960
    minimumHeight: 600
    width: 960
    height: 600
    visible: true
    title: qsTr("LiniTunes")
    color: color_app
    property string color_sidebar: "#f6f6f6"
    property string color_text: "#2c2c2c"
    property string color_button_sidebar: "#e0e0e0"
    property string color_app: "#ffffff"
    property string color_badge: "#dddddd"
    Rectangle {
        id: sidebar
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: 250
        color: "#f6f6f6"
        Rectangle {
            id: clicked_rect
            color: color_button_sidebar
            radius: 10
            x: button_idevice.x
            y: button_idevice.y
            height: button_idevice.height
            width: button_idevice.width

            states: [ State {
                    name: "button_general"
                    PropertyChanges {
                        clicked_rect {
                            y: button_general.y
                            x: button_general.x
                            height: button_general.height
                            width: button_general.width
                        }
                    }
                },
                State {
                    name: "button_idevice"
                    PropertyChanges {
                        clicked_rect {
                            y: button_idevice.y
                            x: button_idevice.x
                            height: button_idevice.height
                            width: button_idevice.width
                        }
                    }
                },
                State {
                    name: "button_files"
                    PropertyChanges {
                        clicked_rect {
                            y: button_files.y
                            x: button_files.x
                            height: button_files.height
                            width: button_files.width
                        }
                    }
                } ]
            transitions: [ Transition {
                    to: "button_general"
                    NumberAnimation { properties: "y, x, height, width"; duration: 130; easing.type: Easing.OutQuart }
                },
                Transition {
                    to: "button_idevice"
                    NumberAnimation { properties: "y, x, height, width"; duration: 130; easing.type: Easing.OutQuart }
                },
                Transition {
                    to: "button_files"
                    NumberAnimation { properties: "y, x, height, width"; duration: 130; easing.type: Easing.OutQuart }
                }
            ]
        }
        Rectangle {
            id: button_idevice
            height: 100
            radius: 5
            color: "#00000000"
            anchors {
                left: parent.left
                top: parent.top
                right: parent.right
                leftMargin: 15
                topMargin: 15
                rightMargin: 15
            }
            Image {
                id: img_idevice
                source: "/images/iDevice/iDevice_90x90.png"
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                    leftMargin: 3
                }
                width: 90
                height: 90
                sourceSize.width: 90
                sourceSize.height: 90
                fillMode: Image.PreserveAspectCrop
                smooth: true
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
                text: "No device connected"
                anchors {
                    left: img_idevice.right
                    top: parent.top
                    right: parent.right
                    rightMargin: 5
                    leftMargin: 3
                    topMargin: 10
                }
                font {
                    bold: true
                    family: "Helvetica"
                    pointSize: 13
                }
                color: color_text
                maximumLineCount: 2
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                Connections {
                    target: DeviceWatcher
                    function onCurrentDeviceChanged() {
                        if (DeviceWatcher.device_connected) {
                            if (device_name.text != DeviceWatcher.device_name) {
                                device_name.text = DeviceWatcher.device_name
                            }
                        } else {
                            device_name.text = "No device connected"
                        }
                    }
                }
            }
            Rectangle {
                id: storage_capacity
                radius: 5
                color: "#00000000"
                width: storage_capacity_text.width + 8
                height: storage_capacity_text.height
                antialiasing: true
                border {
                    width: 1
                    color: color_text
                }
                anchors {
                    left: img_idevice.right
                    top: device_name.bottom
                    topMargin: 3
                }
                visible: false
                Text {
                    id: storage_capacity_text
                    font {
                        bold: false
                        family: "Helvetica"
                        pointSize: 8
                    }
                    anchors {
                        verticalCenter: storage_capacity.verticalCenter
                        horizontalCenter: storage_capacity.horizontalCenter
                    }
                    color: color_text
                    wrapMode: Text.WordWrap
                    Connections {
                        target: DeviceWatcher
                        function onCurrentDeviceChanged() {
                            if (DeviceWatcher.device_connected) {
                                storage_capacity_text.text = DeviceWatcher.storage_capacity
                                storage_capacity.visible = true
                            } else {
                                storage_capacity.visible = false
                            }
                        }
                    }
                }
            }
            MouseArea {
                id: m_idevice
                anchors.fill: parent
                onDoubleClicked: {
                    clicked_rect.state = "button_idevice"
                    popup_devices.open()
                }
                onClicked: {
                    clicked_rect.state = "button_idevice"
//                    main_page.source = "/idevices.qml"
                }
            }
        }
        Rectangle {
            id: button_general
            height: 38
            radius: 5
            color: "#00000000"
            anchors {
                left: parent.left
                top: button_idevice.bottom
                right: parent.right
                leftMargin: 15
                rightMargin: 15
            }
            Image {
                id: img_general
                source: "/images/General/General_30x30.png"
                width: 25
                height: 25
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                    leftMargin: 8
                }
            }
            Text {
                text: "General"
                anchors {
                    left: img_general.right
                    leftMargin: 5
                    verticalCenter: parent.verticalCenter
                }
                font {
                    family: "Helvetica"
                    pointSize: 10
                    weight: Font.DemiBold
                }
                color: color_text
            }
            MouseArea {
                id: m_button_general
                anchors.fill: parent
                onClicked: {
                    clicked_rect.state = "button_general"
                    main_page.source = "/general.qml"
                }
            }
        }
        Rectangle {
            id: button_files
            height: 38
            radius: 5
            color: "#00000000"
            anchors {
                left: parent.left
                top: button_general.bottom
                right: parent.right
                leftMargin: 15
                rightMargin: 15
            }
            Image {
                id: img_files
                source: "/images/Files/Files_30x30.png"
                width: 25
                height: 25
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                    leftMargin: 8
                }
            }
            Text {
                text: "Files"
                anchors {
                    left: img_files.right
                    leftMargin: 5
                    verticalCenter: parent.verticalCenter
                }
                font {
                    family: "Helvetica"
                    pointSize: 10
                    weight: Font.DemiBold
                }
                color: color_text
            }
            MouseArea {
                id: m_button_files
                anchors.fill: parent
                onClicked: {
                    clicked_rect.state = "button_files"
                    main_page.source = "/files.qml"
                }
            }
        }
    }
    Popup {
        id: popup_devices
        width: 500
        padding: 10
        background: Rectangle {
            border.color: color_app
            radius: 10
        }
        property real radius_blur: 1
        Overlay.modal: FastBlur {
            source: root.contentItem
            radius: popup_devices.radius_blur
        }
        anchors.centerIn: Overlay.overlay // milieu de la fenetre
        modal: true
        enter: Transition {
            NumberAnimation { property: "radius_blur"; from: 0.0; to: 1.0 }
                NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
            }
        exit: Transition {
            NumberAnimation { property: "radius_blur"; from: 1.0; to: 0.0 }
                NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
        }
        Grid {
            id: grid_devices
            spacing: 15
            anchors {
                fill: parent
            }
            ScrollBar.vertical: ScrollBar { }
            Repeater {
                id: popup_device_list
                delegate: Rectangle {
                    property string device_ecid: ""
                    property string device_name: ""
                    property string device_storage_capacity: ""
                    property string device_image: ""
                    height: 100
                    width: 230
                    radius: 10
                    color: color_sidebar
                    Image {
                        id: popup_img_idevice
                        source: device_image
                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                            leftMargin: 3
                        }
                        width: 90
                        height: 90
                        sourceSize.width: 90
                        sourceSize.height: 90
                        fillMode: Image.PreserveAspectCrop
                        smooth: true
                    }
                    Text {
                        id: popup_device_name
                        text: device_name
                        anchors {
                            left: popup_img_idevice.right
                            top: parent.top
                            right: parent.right
                            rightMargin: 5
                            leftMargin: 3
                            topMargin: 10
                        }
                        font {
                            bold: true
                            family: "Helvetica"
                            pointSize: 13
                        }
                        color: color_text
                        maximumLineCount: 2
                        wrapMode: Text.Wrap
                        elide: Text.ElideRight
                    }
                    Rectangle {
                        id: popup_storage_capacity
                        radius: 5
                        color: "#00000000"
                        width: popup_storage_capacity_text.width + 8
                        height: popup_storage_capacity_text.height
                        antialiasing: true
                        border {
                            width: 1
                            color: color_text
                        }
                        anchors {
                            left: popup_img_idevice.right
                            top: popup_device_name.bottom
                            topMargin: 3
                        }
                        Text {
                            id: popup_storage_capacity_text
                            text: device_storage_capacity
                            font {
                                bold: false
                                family: "Helvetica"
                                pointSize: 8
                            }
                            anchors {
                                verticalCenter: popup_storage_capacity.verticalCenter
                                horizontalCenter: popup_storage_capacity.horizontalCenter
                            }
                            color: color_text
                            wrapMode: Text.WordWrap
                        }
                    }
                    MouseArea {
                        id: popup_m_idevice
                        anchors.fill: parent
                        onPressed: {
                            parent.color = color_button_sidebar
                        }
                        onReleased: {
                            parent.color = color_sidebar
                        }
                        onClicked: {
                            DeviceWatcher.switchCurrentDevice(parent.device_ecid)
                            popup_devices.close()
                        }
                    }
                }
            }
            Connections {
                target: DeviceWatcher
                function onEcidListChanged() {
                    if (DeviceWatcher.device_connected) {
                        popup_device_list.model = DeviceWatcher.ecid_list.length
                        for (let i = 0; i < DeviceWatcher.ecid_list.length; i++) {
                            popup_device_list.itemAt(i).device_ecid = DeviceWatcher.ecid_list[i]
                            popup_device_list.itemAt(i).device_name = DeviceWatcher.device_name_list[i]
                            popup_device_list.itemAt(i).device_storage_capacity = DeviceWatcher.storage_capacity_list[i]
                            popup_device_list.itemAt(i).device_image = DeviceWatcher.device_image_list[i]
                        }
                    } else {
                        popup_device_list.model = DeviceWatcher.ecid_list.length
                        popup_devices.close()
                    }
                    if (DeviceWatcher.ecid_list.length > 2) {
                        grid_devices.columns = 2
                        grid_devices.rows = Math.round(DeviceWatcher.ecid_list.length/2)
                    }
                }
            }
        }
    }
    Loader {
        id: main_page
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
            left: sidebar.right
        }
        asynchronous: true

        // Allow loaded pages to access the main.qml properties
        property alias rootWindow: root
        Connections {
            target: DeviceWatcher
            function onEcidListChanged() {
                if (main_page.source == "") {
                    main_page.source = "/idevices.qml"
                }
            }
        }
    }
}
