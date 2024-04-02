import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Qt5Compat.GraphicalEffects

Window {
    id: root
    visible: true
    title: qsTr("LiniTunes")
    minimumWidth: 960
    minimumHeight: 630
    color: Qt.rgba(0.0, 0.0, 0.0, 0.0)

    Rectangle {
        id: app
        color: "#181818"
        border.width: 0
        anchors.fill: parent

        Loader {
            id: content
            anchors {
                left: sidebar_rectangle.right
                right: parent.right
                top: parent.top
                bottom: parent.bottom
                bottomMargin: 0
                leftMargin: 0
                rightMargin: 0
                topMargin: 0
            }
            asynchronous: true

            source: "/general.qml"

            // Allow loaded pages to access the main.qml properties
            property alias rootWindow: root
//            Connections {
//                target: DeviceWatcher
//                function onEcidListChanged() {
//                    if (main_page.source == "") {
//                        main_page.source = "/idevices.qml"
//                    }
//                }
//            }
        }
        Rectangle {
            id: content_sidebar_shadow
            visible: true
            anchors {
                left: content.left
                right: content.right
                top: storage_info_stroke.top
                bottom: content.bottom
                rightMargin: 0
                leftMargin: 0
                bottomMargin: 0
                topMargin: 0
            }
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop {
                    position: 0
                    color: "#00000000"
                }

                GradientStop {
                    position: 1
                    color: "#1a000000"
                }
            }
        }

        Rectangle {
            id: storage_info_stroke
            x: 314
            y: 526
            height: 50
            visible: true
            radius: 8
            border.width: 0
            anchors {
                left: content.left
                right: content.right
                bottom: content.bottom
                bottomMargin: 11
            }
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#424242"
                }

                GradientStop {
                    position: 1
                    color: "#00000000"
                }
                orientation: Gradient.Vertical
            }
            anchors.leftMargin: 11
            anchors.rightMargin: 11

            Rectangle {
                id: storage_info
                x: 0
                radius: 8
                border.width: 0
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                    topMargin: 1
                    leftMargin: 1
                    bottomMargin: 1
                    rightMargin: 1
                }
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop {
                        position: 0
                        color: "#383838"
                    }

                    GradientStop {
                        position: 1
                        color: "#343434"
                    }
                }
                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Item {
                        width: storage_info.width
                        height: storage_info.height
                        Rectangle {
                            anchors.centerIn: parent
                            width: storage_info.width
                            height: storage_info.height
                            radius: 8
                        }
                    }
                }

                Rectangle {
                    id: rectangle
                    radius: 5
                    border.color: "#1f1f1f"
                    border.width: 0
                    anchors {
                        left: parent.left
                        right: strorage_sync_button_stroke.left
                        top: parent.top
                        bottom: parent.bottom
                        topMargin: 9
                        bottomMargin: 9
                        rightMargin: 10
                        leftMargin: 9
                    }
                    clip: true
                    gradient: Gradient {
                        orientation: Gradient.Vertical
                        GradientStop {
                            position: 0
                            color: "#424242"
                        }

                        GradientStop {
                            position: 1
                            color: "#00000000"
                        }
                    }

                    Rectangle {
                        id: rectangle3
                        x: -1
                        y: -1
                        color: "#3b3b3b"
                        radius: 5
                        border.color: "#1f1f1f"
                        border.width: 0
                        anchors {
                            left: parent.left
                            right: parent.right
                            top: parent.top
                            bottom: parent.bottom
                            topMargin: 1
                            leftMargin: 1
                            bottomMargin: 1
                            rightMargin: 1
                        }
                        clip: true
                    }
                }

                Rectangle {
                    id: strorage_sync_button_stroke
                    x: 393
                    y: 20
                    width: 100
                    radius: 5
                    border.color: "#1f1f1f"
                    border.width: 0
                    anchors {
                        right: parent.right
                        top: parent.top
                        bottom: parent.bottom
                        topMargin: 9
                        bottomMargin: 9
                        rightMargin: 9
                    }
                    gradient: Gradient {
                        GradientStop {
                            position: 0
                            color: "#424242"
                        }

                        GradientStop {
                            position: 1
                            color: "#00000000"
                        }
                        orientation: Gradient.Vertical
                    }

                    Rectangle {
                        id: strorage_sync_button
                        y: 0
                        color: "#3b3b3b"
                        radius: 5
                        border.color: "#1f1f1f"
                        border.width: 0
                        anchors {
                            left: parent.left
                            right: parent.right
                            top: parent.top
                            bottom: parent.bottom
                            leftMargin: 1
                            topMargin: 1
                            bottomMargin: 1
                            rightMargin: 1
                        }
                    }
                }
            }
        }


        Rectangle {
            id: sidebar_rectangle
            x: 9
            y: 9
            width: 280
            color: "#353535"
            radius: 0
            border.width: 0
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
                bottomMargin: 0
                leftMargin: 0
                topMargin: 0
            }

            Rectangle {
                id: device_stroke
                radius: 8
                border.color: "#00ffffff"
                border.width: 0
                height: 85
                anchors {
                    left: parent.left
                    right: parent.right
                    top: app_title.bottom
                    rightMargin: 10
                    leftMargin: 10
                    topMargin: 12
                }
                layer.enabled: false
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop {
                        position: 0
                        color: "#424242"
                    }

                    GradientStop {
                        position: 1
                        color: "#02000000"
                    }
                }
                states: [ State {
                        name: "device_normal"
                        PropertyChanges {
                            device_stroke {
                                height: 85
                            }
                        }
                    },
                    State {
                        name: "device_extended"
                        PropertyChanges {
                            device_stroke {
                                height: 155
                            }
                        }
                    } ]
                transitions: [ Transition {
                        to: "device_normal"
                        NumberAnimation { properties: "height"; duration: 200; easing.type: Easing.OutCubic }
                    },
                    Transition {
                        to: "device_extended"
                        NumberAnimation { properties: "height"; duration: 200; easing.type: Easing.OutCubic }
                    }
                ]
                state: "device_normal"

                Rectangle {
                    id: device
                    radius: 8
                    border.color: "#00ffffff"
                    border.width: 0
                    anchors {
                        left: device_stroke.left
                        right: device_stroke.right
                        top: device_stroke.top
                        bottom: device_stroke.bottom
                        rightMargin: 1
                        leftMargin: 1
                        bottomMargin: 1
                        topMargin: 1
                    }
                    gradient: Gradient {
                        orientation: Gradient.Vertical
                        GradientStop {
                            position: 1
                            color: "#3a3a3a"
                        }

                        GradientStop {
                            position: 0
                            color: "#3e3e3e"
                        }
                    }

                    Text {
                        id: device_chosen_text
                        opacity: 0.5
                        color: "white"
                        text: qsTr("Device Chosen")
                        anchors {
                            left: parent.left
                            top: parent.top
                            topMargin: 8
                            leftMargin: 10
                        }
                        font.pointSize: 9
                    }

                    Image {
                        id: device_image
                        width: 40
                        height: 50
                        anchors {
                            left: parent.left
                            top: device_chosen_text.bottom
                            topMargin: 4
                            leftMargin: 8
                        }
                        source: "/images/iphone.png"
                        sourceSize.height: device_image.height
                        fillMode: Image.PreserveAspectCrop
                        smooth: true
                    }

                    Text {
                        id: device_name_text
                        color: "#ffffff"
                        text: qsTr("No device connected")
                        anchors {
                            left: device_image.right
                            top: device_image.top
                            topMargin: 0
                            leftMargin: 8
                        }
                        font.weight: Font.DemiBold
                        font.pointSize: 12
                        font.family: "Arial"
                        maximumLineCount: 2
                        wrapMode: Text.Wrap
                        elide: Text.ElideRight
                        Connections {
                            target: DeviceWatcher
                            function onCurrentDeviceChanged() {
                                if (DeviceWatcher.device_connected) {
                                    if (device_name_text.text != DeviceWatcher.device_name) {
                                        device_name_text.text = DeviceWatcher.device_name
                                    }
                                } else {
                                    device_name_text.text = "No device connected"
                                }
                            }
                        }
                    }

                    Rectangle {
                        id: content_info_circle
                        width: 15
                        height: 15
                        opacity: 0.5
                        color: "#00000000"
                        border.color: "white"
                        border.width: 1
                        anchors.right: parent.right
                        anchors.bottom: device_image.bottom
                        anchors.rightMargin: 10
                        anchors.bottomMargin: 2
                        radius: width*0.5
                        Text {
                            id: content_info_circle_text
                            text: "i"
                            height: 10
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: "white"
                            anchors.fill: parent
                            font.family: "Arial"
                            font.pointSize: 9
                            font.bold: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            onPressed: parent.opacity=0.7
                            onReleased: {
                                parent.opacity=0.5
                                if (DeviceWatcher.device_connected) {
                                    if (device_stroke.state == "device_normal") {
                                        device_stroke.state = "device_extended"
                                        serial_info_rect.state = "device_extended"
                                    } else {
                                        device_stroke.state = "device_normal"
                                        serial_info_rect.state = "device_normal"
                                    }
                                }
                            }
                        }
                    }

                    Image {
                        id: content_develop_arrow
                        width: 18
                        height: 18
                        opacity: 0.5
                        anchors {
                            verticalCenter: device_chosen_text.verticalCenter
                            right: parent.right
                            rightMargin: 8
                        }
                        source: "../images/develop_arrow.png"
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        id: content_iphone_model
                        opacity: 1
                        color: "#d9d9d9"
                        text: qsTr("Please connect a device to begin the sync")
                        anchors {
                            left: device_image.right
                            top: device_name_text.bottom
                            leftMargin: 8
                            topMargin: 1
                            right: parent.right
                            rightMargin: 40
                        }
                        font.family: "Arial"
                        font.weight: Font.Medium
                        font.pointSize: 9
                        wrapMode: Text.WordWrap
                        Connections {
                            target: DeviceWatcher
                            function onCurrentDeviceChanged() {
                                if (DeviceWatcher.device_connected) {
                                    content_iphone_model.text = DeviceWatcher.product_type
                                } else {
                                    content_iphone_model.text = "Please connect a device to begin"
                                }
                            }
                        }
                    }

                    Rectangle {
                        id: content_storage_capacity
                        width: content_storage_capacity_text.width + 6
                        height: 14
                        opacity: 1
                        color: "#00ffffff"
                        radius: 3
                        border.color: "#d9d9d9"
                        border.width: 1
                        anchors {
                            left: device_image.right
                            top: content_iphone_model.bottom
                            topMargin: 2
                            leftMargin: 8
                        }
                        visible: false

                        Text {
                            id: content_storage_capacity_text
                            height: 14
                            opacity: 1
                            color: "#d9d9d9"
                            text: qsTr("8GB")
                            anchors {
                                verticalCenter: parent.verticalCenter
                                horizontalCenter: parent.horizontalCenter
                            }
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pointSize: 8
                            font.family: "Arial"
                            wrapMode: Text.WordWrap
                            Connections {
                                target: DeviceWatcher
                                function onCurrentDeviceChanged() {
                                    if (DeviceWatcher.device_connected) {
                                        content_storage_capacity_text.text = DeviceWatcher.storage_capacity
                                        content_storage_capacity.visible = true
                                    } else {
                                        content_storage_capacity.visible = false
                                    }
                                }
                            }
                        }
                    }

                    Text {
                        id: content_storage_left
                        width: 114
                        height: 14
                        opacity: 1
                        color: "#d9d9d9"
                        text: qsTr("160.4 GB Available")
                        anchors {
                            verticalCenter: content_storage_capacity.verticalCenter
                            left: content_storage_capacity.right
                            leftMargin: 4
                        }
                        font.weight: Font.Medium
                        font.pointSize: 9
                        font.family: "Arial"
                        visible: false
                        // TotalDataAvailable doesn't give the real storage left. To investigate.
//                        Connections {
//                            target: DeviceWatcher
//                            function onCurrentDeviceChanged() {
//                                if (DeviceWatcher.device_connected) {
//                                    content_storage_left.text = DeviceWatcher.storage_left + " Available"
//                                    content_storage_left.visible = true
//                                } else {
//                                    content_storage_left.visible = false
//                                }
//                            }
//                        }
                    }

                    Rectangle {
                        id: content_battery
                        width: 24
                        height: 11
                        anchors {
                            verticalCenter: device_name_text.verticalCenter
                            right: parent.right
                            rightMargin: 8
                        }
                        color: "transparent"
                        visible: true

                        Rectangle {
                            id: device_battery_rect1
                            width: 22
                            opacity: 0.5
                            color: "#ffffff"
                            radius: 3
                            border.color: "#ffffff"
                            anchors {
                                left: parent.left
                                top: parent.top
                                bottom: parent.bottom
                                leftMargin: 0
                                topMargin: 0
                                bottomMargin: 0
                            }
                        }

                        Rectangle {
                            id: device_battery_rect2
                            width: 1
                            height: 5
                            opacity: 0.5
                            color: "#ffffff"
                            anchors {
                                right: parent.right
                                rightMargin: 0
                                verticalCenter: parent.verticalCenter
                            }
                        }

                        Rectangle {
                            id: device_battery_fill
                            width: 18
                            color: "#6bcc43"
                            radius: 3
                            anchors {
                                left: parent.left
                                top: parent.top
                                bottom: parent.bottom
                                leftMargin: 0
                                topMargin: 0
                                bottomMargin: 0
                            }
                            visible: true
                        }

                        Text {
                            id: device_battery_text
                            width: 20
                            height: 11
                            color: "#3b3b3b"
                            text: qsTr("82")
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.family: "Arial"
                            font.weight: Font.Bold
                            font.pointSize: 9
                            anchors.fill: device_battery_rect1
                        }
                        Connections {
                            target: DeviceWatcher
                            function onCurrentDeviceChanged() {
                                if (DeviceWatcher.device_connected) {
                                    device_battery_text.text = DeviceWatcher.battery_string
                                    device_battery_fill.width = DeviceWatcher.battery
                                    if (DeviceWatcher.battery_string === "100") {
                                        device_battery_rect2.color = "#6bcc43"
                                        device_battery_rect2.opacity = 1
                                    } else {
                                        device_battery_rect2.color = "#ffffff"
                                        device_battery_rect2.opacity = 0.5
                                    }
                                    content_battery.visible = true
                                } else {
                                    content_battery.visible = false
                                }
                            }
                        }
                    }
                    Rectangle {
                        id: serial_info_rect
                        opacity: 0
                        color: "transparent"
                        anchors {
                            top: device_image.bottom
                            left: parent.left
                            right: parent.right
                            bottom: parent.bottom
                            topMargin: 4
                            leftMargin: 4
                            rightMargin: 4
                        }
                        height: 90
                        visible: false

                        states: [ State {
                                name: "device_normal"
                                PropertyChanges {
                                    serial_info_rect {
                                        opacity: 0
                                    }
                                }
                            },
                            State {
                                name: "device_extended"
                                PropertyChanges {
                                    serial_info_rect {
                                        opacity: 1.0
                                    }
                                }
                            } ]
                        transitions: [ Transition {
                                to: "device_normal"
                                NumberAnimation { properties: "opacity"; duration: 200; easing.type: Easing.OutCubic }
                            },
                            Transition {
                                to: "device_extended"
                                NumberAnimation { properties: "opacity"; duration: 200; easing.type: Easing.OutCubic }
                            }
                        ]

                        Text {
                            id: serial_text
                            opacity: 0.5
                            color: "white"
                            text: qsTr("SERIAL")
                            anchors {
                                top: parent.top
                                left: parent.left
                                topMargin: 4
                                leftMargin: 6
                            }
                            font.pointSize: 8
                        }
                        Text {
                            id: serial_info_text
                            text: qsTr("")
                            color: "white"
                            anchors {
                                top: serial_text.bottom
                                left: serial_text.left
                                topMargin: 1
                                leftMargin: 0
                            }
                            font.pointSize: 8
                        }
                        Text {
                            id: imei_text
                            opacity: 0.5
                            color: "white"
                            text: qsTr("IMEI")
                            anchors {
                                top: parent.top
                                left: parent.left
                                topMargin: 4
                                leftMargin: parent.width/2
                            }
                            font.pointSize: 8
                        }
                        Text {
                            id: imei_info_text
                            text: qsTr("")
                            color: "white"
                            anchors {
                                top: imei_text.bottom
                                left: imei_text.left
                                topMargin: 1
                                leftMargin: 0
                            }
                            font.pointSize: 8
                        }
                        Text {
                            id: udid_text
                            opacity: 0.5
                            color: "white"
                            text: qsTr("UDID")
                            anchors {
                                top: serial_info_text.bottom
                                left: parent.left
                                topMargin: 2
                                leftMargin: 6
                            }
                            font.pointSize: 8
                        }
                        Text {
                            id: udid_info_text
                            text: qsTr("")
                            color: "white"
                            anchors {
                                top: udid_text.bottom
                                left: udid_text.left
                                topMargin: 1
                                leftMargin: 0
                            }
                            font.pointSize: 8
                        }
                        Connections {
                            target: DeviceWatcher
                            function onCurrentDeviceChanged() {
                                if (DeviceWatcher.device_connected) {
                                    serial_info_text.text = DeviceWatcher.serial
                                    imei_info_text.text = DeviceWatcher.imei
                                    udid_info_text.text = DeviceWatcher.udid
                                    serial_info_rect.visible = true
                                } else {
                                    serial_info_text.text = ""
                                    imei_info_text.text = ""
                                    udid_info_text.text = ""
                                }
                            }
                        }
                    }
                }
            }

            Text {
                id: app_title
                x: 17
                y: 17
                width: 112
                height: 27
                color: "#ffffff"
                text: qsTr("LiniTunes")
                anchors {
                    left: parent.left
                    top: parent.top
                    leftMargin: 14
                    topMargin: 14
                }
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignTop
                font.family: "Arial"
                font.pointSize: 18
                font.styleName: "Bold"
            }

            Rectangle {
                id: clicked_rect
                color: "#3284ff"
                radius: 8
                x: sidebar_general_button.x
                y: sidebar_general_button.y
                height: sidebar_general_button.height
                width: sidebar_general_button.width

                states: [ State {
                        name: "sidebar_general_button"
                        PropertyChanges {
                            clicked_rect {
                                y: sidebar_general_button.y
                                x: sidebar_general_button.x
                                height: sidebar_general_button.height
                                width: sidebar_general_button.width
                            }
                        }
                    },
                    State {
                        name: "sidebar_music_button"
                        PropertyChanges {
                            clicked_rect {
                                y: sidebar_music_button.y
                                x: sidebar_music_button.x
                                height: sidebar_music_button.height
                                width: sidebar_music_button.width
                            }
                        }
                    },
                    State {
                        name: "sidebar_movies_button"
                        PropertyChanges {
                            clicked_rect {
                                y: sidebar_movies_button.y
                                x: sidebar_movies_button.x
                                height: sidebar_movies_button.height
                                width: sidebar_movies_button.width
                            }
                        }
                    },
                    State {
                        name: "sidebar_tv_button"
                        PropertyChanges {
                            clicked_rect {
                                y: sidebar_tv_button.y
                                x: sidebar_tv_button.x
                                height: sidebar_tv_button.height
                                width: sidebar_tv_button.width
                            }
                        }
                    },
                    State {
                        name: "sidebar_podcasts_button"
                        PropertyChanges {
                            clicked_rect {
                                y: sidebar_podcasts_button.y
                                x: sidebar_podcasts_button.x
                                height: sidebar_podcasts_button.height
                                width: sidebar_podcasts_button.width
                            }
                        }
                    },
                    State {
                        name: "sidebar_audiobooks_button"
                        PropertyChanges {
                            clicked_rect {
                                y: sidebar_audiobooks_button.y
                                x: sidebar_audiobooks_button.x
                                height: sidebar_audiobooks_button.height
                                width: sidebar_audiobooks_button.width
                            }
                        }
                    },
                    State {
                        name: "sidebar_books_button"
                        PropertyChanges {
                            clicked_rect {
                                y: sidebar_books_button.y
                                x: sidebar_books_button.x
                                height: sidebar_books_button.height
                                width: sidebar_books_button.width
                            }
                        }
                    },
                    State {
                        name: "sidebar_photos_button"
                        PropertyChanges {
                            clicked_rect {
                                y: sidebar_photos_button.y
                                x: sidebar_photos_button.x
                                height: sidebar_photos_button.height
                                width: sidebar_photos_button.width
                            }
                        }
                    },
                    State {
                        name: "sidebar_files_button"
                        PropertyChanges {
                            clicked_rect {
                                y: sidebar_files_button.y
                                x: sidebar_files_button.x
                                height: sidebar_files_button.height
                                width: sidebar_files_button.width
                            }
                        }
                    } ]
                transitions: [ Transition {
                        to: "sidebar_general_button"
                        NumberAnimation { properties: "y, x, height, width"; duration: 200; easing.type: Easing.OutCubic }
                    },
                    Transition {
                        to: "sidebar_music_button"
                        NumberAnimation { properties: "y, x, height, width"; duration: 200; easing.type: Easing.OutCubic }
                    },
                    Transition {
                        to: "sidebar_movies_button"
                        NumberAnimation { properties: "y, x, height, width"; duration: 200; easing.type: Easing.OutCubic }
                    },
                    Transition {
                        to: "sidebar_tv_button"
                        NumberAnimation { properties: "y, x, height, width"; duration: 200; easing.type: Easing.OutCubic }
                    },
                    Transition {
                        to: "sidebar_podcasts_button"
                        NumberAnimation { properties: "y, x, height, width"; duration: 200; easing.type: Easing.OutCubic }
                    },
                    Transition {
                        to: "sidebar_audiobooks_button"
                        NumberAnimation { properties: "y, x, height, width"; duration: 200; easing.type: Easing.OutCubic }
                    },
                    Transition {
                        to: "sidebar_books_button"
                        NumberAnimation { properties: "y, x, height, width"; duration: 200; easing.type: Easing.OutCubic }
                    },
                    Transition {
                        to: "sidebar_photos_button"
                        NumberAnimation { properties: "y, x, height, width"; duration: 200; easing.type: Easing.OutCubic }
                    },
                    Transition {
                        to: "sidebar_files_button"
                        NumberAnimation { properties: "y, x, height, width"; duration: 200; easing.type: Easing.OutCubic }
                    }
                ]
            }

            Rectangle {
                id: sidebar_general_button
                height: 40
                color: "#00000000"
                radius: 8
                anchors {
                    left: parent.left
                    right: parent.right
                    top: device_stroke.bottom
                    topMargin: 9
                    rightMargin: 10
                    leftMargin: 10
                }
                opacity: 1.0
                states: [ State {
                        name: "clicked"
                        PropertyChanges {
                            sidebar_general_button {
                                opacity: 1.0
                            }
                        }
                    },
                    State {
                        name: "unclicked"
                        PropertyChanges {
                            sidebar_general_button {
                                opacity: 0.5
                            }
                        }
                    }]
                transitions:  [ Transition {
                    to: "clicked"
                    NumberAnimation { properties: "opacity"; duration: 100; easing.type: Easing.OutExpo }
                },
                Transition {
                    to: "unclicked"
                    NumberAnimation { properties: "opacity"; duration: 30; easing.type: Easing.InExpo }
                } ]

                Image {
                    id: sidebar_general_button_image
                    width: 24
                    anchors {
                        left: parent.left
                        top: parent.top
                        bottom: parent.bottom
                        leftMargin: 8
                        bottomMargin: 8
                        topMargin: 8
                    }
                    source: "../images/settings.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    id: sidebar_general_button_text
                    color: "#ffffff"
                    text: qsTr("General")
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: sidebar_general_button_image.right
                        leftMargin: 8
                    }
                    font.weight: Font.DemiBold
                    font.family: "Arial"
                    font.pointSize: 10
                }
                MouseArea {
                    id: m_sidebar_general_button
                    anchors.fill: parent
                    onClicked: {
                        clicked_rect.state = "sidebar_general_button"
                        sidebar_general_button.state = "clicked"
                        sidebar_music_button.state = "unclicked"
                        sidebar_movies_button.state = "unclicked"
                        sidebar_tv_button.state = "unclicked"
                        sidebar_podcasts_button.state = "unclicked"
                        sidebar_audiobooks_button.state = "unclicked"
                        sidebar_books_button.state = "unclicked"
                        sidebar_photos_button.state = "unclicked"
                        sidebar_files_button.state = "unclicked"
                        content.source = "/general.qml"
                    }
                }
            }

            Rectangle {
                id: sidebar_music_button
                height: 40
                color: "#00000000"
                radius: 6
                anchors {
                    left: parent.left
                    right: parent.right
                    top: sidebar_general_button.bottom
                    topMargin: 4
                    leftMargin: 10
                    rightMargin: 10
                }
                opacity: 0.5
                states: [ State {
                        name: "clicked"
                        PropertyChanges {
                            sidebar_music_button {
                                opacity: 1.0
                            }
                        }
                    },
                    State {
                        name: "unclicked"
                        PropertyChanges {
                            sidebar_music_button {
                                opacity: 0.5
                            }
                        }
                    }]
                transitions:  [ Transition {
                    to: "clicked"
                    NumberAnimation { properties: "opacity"; duration: 100; easing.type: Easing.OutExpo }
                },
                Transition {
                    to: "unclicked"
                    NumberAnimation { properties: "opacity"; duration: 30; easing.type: Easing.InExpo }
                } ]
                Image {
                    id: sidebar_music_button_image
                    width: 24
                    anchors {
                        left: parent.left
                        top: parent.top
                        bottom: parent.bottom
                        topMargin: 8
                        leftMargin: 8
                        bottomMargin: 8
                    }
                    source: "../images/music.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    id: sidebar_music_button_text
                    color: "#ffffff"
                    text: qsTr("Music")
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: sidebar_music_button_image.right
                        leftMargin: 8
                    }
                    font.family: "Arial"
                    font.pointSize: 10
                    font.weight: Font.DemiBold
                }
                MouseArea {
                    id: m_sidebar_music_button
                    anchors.fill: parent
                    onClicked: {
                        clicked_rect.state = "sidebar_music_button"
                        sidebar_general_button.state = "unclicked"
                        sidebar_music_button.state = "clicked"
                        sidebar_movies_button.state = "unclicked"
                        sidebar_tv_button.state = "unclicked"
                        sidebar_podcasts_button.state = "unclicked"
                        sidebar_audiobooks_button.state = "unclicked"
                        sidebar_books_button.state = "unclicked"
                        sidebar_photos_button.state = "unclicked"
                        sidebar_files_button.state = "unclicked"
                        content.source = "/general.qml"
                    }
                }
            }

            Rectangle {
                id: sidebar_movies_button
                height: 40
                color: "#00000000"
                radius: 6
                anchors {
                    left: parent.left
                    right: parent.right
                    top: sidebar_music_button.bottom
                    topMargin: 4
                    leftMargin: 10
                }
                opacity: 0.5
                states: [ State {
                        name: "clicked"
                        PropertyChanges {
                            sidebar_movies_button {
                                opacity: 1.0
                            }
                        }
                    },
                    State {
                        name: "unclicked"
                        PropertyChanges {
                            sidebar_movies_button {
                                opacity: 0.5
                            }
                        }
                    }]
                transitions:  [ Transition {
                    to: "clicked"
                    NumberAnimation { properties: "opacity"; duration: 100; easing.type: Easing.OutExpo }
                },
                Transition {
                    to: "unclicked"
                    NumberAnimation { properties: "opacity"; duration: 30; easing.type: Easing.InExpo }
                } ]
                Image {
                    id: sidebar_movies_button_image
                    width: 24
                    anchors {
                        left: parent.left
                        top: parent.top
                        bottom: parent.bottom
                        topMargin: 8
                        leftMargin: 8
                        bottomMargin: 8
                    }
                    source: "../images/movies.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    id: sidebar_movies_button_text
                    color: "#ffffff"
                    text: qsTr("Movies")
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: sidebar_movies_button_image.right
                        leftMargin: 8
                    }
                    font.family: "Arial"
                    font.pointSize: 10
                    font.weight: Font.DemiBold
                }
                anchors.rightMargin: 10
                MouseArea {
                    id: m_sidebar_movies_button
                    anchors.fill: parent
                    onClicked: {
                        clicked_rect.state = "sidebar_movies_button"
                        sidebar_general_button.state = "unclicked"
                        sidebar_music_button.state = "unclicked"
                        sidebar_movies_button.state = "clicked"
                        sidebar_tv_button.state = "unclicked"
                        sidebar_podcasts_button.state = "unclicked"
                        sidebar_audiobooks_button.state = "unclicked"
                        sidebar_books_button.state = "unclicked"
                        sidebar_photos_button.state = "unclicked"
                        sidebar_files_button.state = "unclicked"
                        content.source = "/general.qml"
                    }
                }
            }

            Rectangle {
                id: sidebar_tv_button
                height: 40
                color: "#00000000"
                radius: 6
                anchors {
                    left: parent.left
                    right: parent.right
                    top: sidebar_movies_button.bottom
                    topMargin: 4
                    leftMargin: 10
                    rightMargin: 10
                }
                opacity: 0.5
                states: [ State {
                        name: "clicked"
                        PropertyChanges {
                            sidebar_tv_button {
                                opacity: 1.0
                            }
                        }
                    },
                    State {
                        name: "unclicked"
                        PropertyChanges {
                            sidebar_tv_button {
                                opacity: 0.5
                            }
                        }
                    }]
                transitions:  [ Transition {
                    to: "clicked"
                    NumberAnimation { properties: "opacity"; duration: 100; easing.type: Easing.OutExpo }
                },
                Transition {
                    to: "unclicked"
                    NumberAnimation { properties: "opacity"; duration: 30; easing.type: Easing.InExpo }
                } ]
                Image {
                    id: sidebar_tv_button_image
                    width: 24
                    anchors {
                        left: parent.left
                        top: parent.top
                        bottom: parent.bottom
                        topMargin: 8
                        leftMargin: 8
                        bottomMargin: 8
                    }
                    source: "../images/tv.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    id: sidebar_tv_button_text
                    color: "#ffffff"
                    text: qsTr("TV")
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: sidebar_tv_button_image.right
                        leftMargin: 8
                        rightMargin: 10
                    }
                    font.family: "Arial"
                    font.pointSize: 10
                    font.weight: Font.DemiBold
                }
                MouseArea {
                    id: m_sidebar_tv_button
                    anchors.fill: parent
                    onClicked: {
                        clicked_rect.state = "sidebar_tv_button"
                        sidebar_general_button.state = "unclicked"
                        sidebar_music_button.state = "unclicked"
                        sidebar_movies_button.state = "unclicked"
                        sidebar_tv_button.state = "clicked"
                        sidebar_podcasts_button.state = "unclicked"
                        sidebar_audiobooks_button.state = "unclicked"
                        sidebar_books_button.state = "unclicked"
                        sidebar_photos_button.state = "unclicked"
                        sidebar_files_button.state = "unclicked"
                        content.source = "/general.qml"
                    }
                }
            }

            Rectangle {
                id: sidebar_podcasts_button
                x: -11
                height: 40
                color: "#00000000"
                radius: 6
                anchors {
                    left: parent.left
                    right: parent.right
                    top: sidebar_tv_button.bottom
                    topMargin: 4
                    leftMargin: 10
                    rightMargin: 10
                }
                opacity: 0.5
                states: [ State {
                        name: "clicked"
                        PropertyChanges {
                            sidebar_podcasts_button {
                                opacity: 1.0
                            }
                        }
                    },
                    State {
                        name: "unclicked"
                        PropertyChanges {
                            sidebar_podcasts_button {
                                opacity: 0.5
                            }
                        }
                    }]
                transitions:  [ Transition {
                    to: "clicked"
                    NumberAnimation { properties: "opacity"; duration: 100; easing.type: Easing.OutExpo }
                },
                Transition {
                    to: "unclicked"
                    NumberAnimation { properties: "opacity"; duration: 30; easing.type: Easing.InExpo }
                } ]
                Image {
                    id: sidebar_podcasts_button_image
                    width: 24
                    anchors {
                        left: parent.left
                        top: parent.top
                        bottom: parent.bottom
                        topMargin: 8
                        leftMargin: 8
                        bottomMargin: 8
                    }
                    source: "../images/podcast.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    id: sidebar_podcasts_button_text
                    color: "#ffffff"
                    text: qsTr("Podcasts")
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: sidebar_podcasts_button_image.right
                        leftMargin: 8
                    }
                    font.pointSize: 10
                    font.family: "Arial"
                    font.weight: Font.DemiBold
                }
                MouseArea {
                    id: m_sidebar_podcasts_button
                    anchors.fill: parent
                    onClicked: {
                        clicked_rect.state = "sidebar_podcasts_button"
                        sidebar_general_button.state = "unclicked"
                        sidebar_music_button.state = "unclicked"
                        sidebar_movies_button.state = "unclicked"
                        sidebar_tv_button.state = "unclicked"
                        sidebar_podcasts_button.state = "clicked"
                        sidebar_audiobooks_button.state = "unclicked"
                        sidebar_books_button.state = "unclicked"
                        sidebar_photos_button.state = "unclicked"
                        sidebar_files_button.state = "unclicked"
                        content.source = "/general.qml"
                    }
                }
            }

            Rectangle {
                id: sidebar_audiobooks_button
                x: 0
                height: 40
                color: "#00000000"
                radius: 6
                anchors {
                    left: parent.left
                    right: parent.right
                    top: sidebar_podcasts_button.bottom
                    topMargin: 4
                    leftMargin: 10
                }
                opacity: 0.5
                states: [ State {
                        name: "clicked"
                        PropertyChanges {
                            sidebar_audiobooks_button {
                                opacity: 1.0
                            }
                        }
                    },
                    State {
                        name: "unclicked"
                        PropertyChanges {
                            sidebar_audiobooks_button {
                                opacity: 0.5
                            }
                        }
                    }]
                transitions:  [ Transition {
                    to: "clicked"
                    NumberAnimation { properties: "opacity"; duration: 100; easing.type: Easing.OutExpo }
                },
                Transition {
                    to: "unclicked"
                    NumberAnimation { properties: "opacity"; duration: 30; easing.type: Easing.InExpo }
                } ]
                Image {
                    id: sidebar_audiobooks_button_image
                    width: 24
                    anchors {
                        left: parent.left
                        top: parent.top
                        bottom: parent.bottom
                        topMargin: 8
                        leftMargin: 8
                        bottomMargin: 8
                    }
                    source: "../images/audiobook.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    id: sidebar_audiobooks_button_text
                    color: "#ffffff"
                    text: qsTr("Audiobooks")
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: sidebar_audiobooks_button_image.right
                        leftMargin: 8
                    }
                    font.family: "Arial"
                    font.pointSize: 10
                    font.weight: Font.DemiBold
                }
                anchors.rightMargin: 10
                MouseArea {
                    id: m_sidebar_audiobooks_button
                    anchors.fill: parent
                    onClicked: {
                        clicked_rect.state = "sidebar_audiobooks_button"
                        sidebar_general_button.state = "unclicked"
                        sidebar_music_button.state = "unclicked"
                        sidebar_movies_button.state = "unclicked"
                        sidebar_tv_button.state = "unclicked"
                        sidebar_podcasts_button.state = "unclicked"
                        sidebar_audiobooks_button.state = "clicked"
                        sidebar_books_button.state = "unclicked"
                        sidebar_photos_button.state = "unclicked"
                        sidebar_files_button.state = "unclicked"
                        content.source = "/general.qml"
                    }
                }
            }

            Rectangle {
                id: sidebar_books_button
                x: 5
                height: 40
                color: "#00000000"
                radius: 6
                anchors {
                    left: parent.left
                    right: parent.right
                    top: sidebar_audiobooks_button.bottom
                    topMargin: 4
                    leftMargin: 10
                }
                opacity: 0.5
                states: [ State {
                        name: "clicked"
                        PropertyChanges {
                            sidebar_books_button {
                                opacity: 1.0
                            }
                        }
                    },
                    State {
                        name: "unclicked"
                        PropertyChanges {
                            sidebar_books_button {
                                opacity: 0.5
                            }
                        }
                    }]
                transitions:  [ Transition {
                    to: "clicked"
                    NumberAnimation { properties: "opacity"; duration: 100; easing.type: Easing.OutExpo }
                },
                Transition {
                    to: "unclicked"
                    NumberAnimation { properties: "opacity"; duration: 30; easing.type: Easing.InExpo }
                } ]
                Image {
                    id: sidebar_books_button_image
                    width: 24
                    anchors {
                        left: parent.left
                        top: parent.top
                        bottom: parent.bottom
                        topMargin: 8
                        leftMargin: 8
                        bottomMargin: 8
                    }
                    source: "../images/book.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    id: sidebar_books_button_text
                    color: "#ffffff"
                    text: qsTr("Books")
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: sidebar_books_button_image.right
                        leftMargin: 8
                    }
                    font.family: "Arial"
                    font.pointSize: 10
                    font.weight: Font.DemiBold
                }
                anchors.rightMargin: 10
                MouseArea {
                    id: m_sidebar_books_button
                    anchors.fill: parent
                    onClicked: {
                        clicked_rect.state = "sidebar_books_button"
                        sidebar_general_button.state = "unclicked"
                        sidebar_music_button.state = "unclicked"
                        sidebar_movies_button.state = "unclicked"
                        sidebar_tv_button.state = "unclicked"
                        sidebar_podcasts_button.state = "unclicked"
                        sidebar_audiobooks_button.state = "unclicked"
                        sidebar_books_button.state = "clicked"
                        sidebar_photos_button.state = "unclicked"
                        sidebar_files_button.state = "unclicked"
                        content.source = "/general.qml"
                    }
                }
            }

            Rectangle {
                id: sidebar_photos_button
                x: -8
                height: 40
                color: "#00000000"
                radius: 6
                anchors {
                    left: parent.left
                    right: parent.right
                    top: sidebar_books_button.bottom
                    topMargin: 4
                    leftMargin: 10
                    rightMargin: 10
                }
                opacity: 0.5
                states: [ State {
                        name: "clicked"
                        PropertyChanges {
                            sidebar_photos_button {
                                opacity: 1.0
                            }
                        }
                    },
                    State {
                        name: "unclicked"
                        PropertyChanges {
                            sidebar_photos_button {
                                opacity: 0.5
                            }
                        }
                    }]
                transitions:  [ Transition {
                    to: "clicked"
                    NumberAnimation { properties: "opacity"; duration: 100; easing.type: Easing.OutExpo }
                },
                Transition {
                    to: "unclicked"
                    NumberAnimation { properties: "opacity"; duration: 30; easing.type: Easing.InExpo }
                } ]
                Image {
                    id: sidebar_photos_button_image
                    width: 24
                    anchors {
                        left: parent.left
                        top: parent.top
                        bottom: parent.bottom
                        topMargin: 8
                        leftMargin: 8
                        bottomMargin: 8
                    }
                    source: "../images/photo.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    id: sidebar_photos_button_text
                    color: "#ffffff"
                    text: qsTr("Photos")
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: sidebar_photos_button_image.right
                        leftMargin: 8
                    }
                    font.family: "Arial"
                    font.pointSize: 10
                    font.weight: Font.DemiBold
                }
                MouseArea {
                    id: m_sidebar_photos_button
                    anchors.fill: parent
                    onClicked: {
                        clicked_rect.state = "sidebar_photos_button"
                        sidebar_general_button.state = "unclicked"
                        sidebar_music_button.state = "unclicked"
                        sidebar_movies_button.state = "unclicked"
                        sidebar_tv_button.state = "unclicked"
                        sidebar_podcasts_button.state = "unclicked"
                        sidebar_audiobooks_button.state = "unclicked"
                        sidebar_books_button.state = "unclicked"
                        sidebar_photos_button.state = "clicked"
                        sidebar_files_button.state = "unclicked"
                        content.source = "/general.qml"
                    }
                }
            }

            Rectangle {
                id: sidebar_files_button
                height: 40
                color: "#00000000"
                radius: 6
                anchors {
                    left: parent.left
                    right: parent.right
                    top: sidebar_photos_button.bottom
                    topMargin: 4
                    leftMargin: 10
                }
                opacity: 0.5
                states: [ State {
                        name: "clicked"
                        PropertyChanges {
                            sidebar_files_button {
                                opacity: 1.0
                            }
                        }
                    },
                    State {
                        name: "unclicked"
                        PropertyChanges {
                            sidebar_files_button {
                                opacity: 0.5
                            }
                        }
                    }]
                transitions:  [ Transition {
                    to: "clicked"
                    NumberAnimation { properties: "opacity"; duration: 100; easing.type: Easing.OutExpo }
                },
                Transition {
                    to: "unclicked"
                    NumberAnimation { properties: "opacity"; duration: 30; easing.type: Easing.InExpo }
                } ]
                Image {
                    id: sidebar_files_button_image
                    width: 24
                    anchors {
                        left: parent.left
                        top: parent.top
                        bottom: parent.bottom
                        topMargin: 8
                        leftMargin: 8
                        bottomMargin: 8
                    }
                    source: "../images/folder.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    id: sidebar_files_button_text
                    color: "#ffffff"
                    text: qsTr("Files")
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: sidebar_files_button_image.right
                        leftMargin: 8
                    }
                    font.family: "Arial"
                    font.pointSize: 10
                    font.weight: Font.DemiBold
                }
                anchors.rightMargin: 10
                MouseArea {
                    id: m_sidebar_files_button
                    anchors.fill: parent
                    onClicked: {
                        clicked_rect.state = "sidebar_files_button"
                        sidebar_general_button.state = "unclicked"
                        sidebar_music_button.state = "unclicked"
                        sidebar_movies_button.state = "unclicked"
                        sidebar_tv_button.state = "unclicked"
                        sidebar_podcasts_button.state = "unclicked"
                        sidebar_audiobooks_button.state = "unclicked"
                        sidebar_books_button.state = "unclicked"
                        sidebar_photos_button.state = "unclicked"
                        sidebar_files_button.state = "clicked"
                        content.source = "/general.qml"
                    }
                }
            }

            Rectangle {
                id: app_settings_button
                x: 240
                y: 13
                width: 27
                height: 27
                anchors {
                    verticalCenter: app_title.verticalCenter
                    right: parent.right
                    rightMargin: 14
                }
                color: "#252525"
                radius: 27

                Image {
                    id: app_settings_button_image
                    x: 7
                    width: 21
                    height: 21
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: "../images/settings.png"
                    fillMode: Image.PreserveAspectFit
                    sourceSize.height: app_settings_button_image.height
                    sourceSize.width: app_settings_button_image.width
                }
            }
        }
    }

}
