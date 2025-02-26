import QtQuick
import Qt5Compat.GraphicalEffects
import QtQuick.Effects

Window {
    id: root
    visible: true
    title: qsTr("LiniTunes")
    minimumWidth: 960
    minimumHeight: 630
    color: colors.background
    property bool devices_normal: true
    property bool devices_extended: false
    property bool devices_choice: false

    // Theme-related properties
    property bool isDarkTheme: ThemeManager.isDarkMode

    // Color scheme
    property QtObject colors: QtObject {
        property color background: root.isDarkTheme ?                       "#181818" : "#ffffff"
        property color contentBackground: root.isDarkTheme ?                "#303030" : "#ffffff"
        property color sidebarBackground: root.isDarkTheme ?                "#353535" : "#f6f5f4"
        property color sidebarBackgroundTransparent: root.isDarkTheme ?     "#00353535" : "#00f6f5f4"
        property color sidebarCardStroke: root.isDarkTheme ?                "#424242" : "#deddda"

        property color textPrimary: root.isDarkTheme ?                      "#ffffff" : "#241f31"
        property color textSecondary: root.isDarkTheme ?                    "#d9d9d9" : "#3d3846"
        property color accent:                                              "#3284ff"

        property color buttonHover: root.isDarkTheme ?                      "#424242" : "#e0e0e0"
        property color divider: root.isDarkTheme ?                          "#424242" : "#e0e0e0"

        property color cardBackgroundTop: root.isDarkTheme ?                "#3a3a3a" : "#ffffff"
        property color cardBackgroundBottom: root.isDarkTheme ?             "#3e3e3e" : "#f6f5f4"
        property color cardStroke: root.isDarkTheme ?                       "#424242" : "#f6f5f4"

        property color settingsButtonBg: root.isDarkTheme ?                 "#252525" : "#f0f0f0"

        property color sideTextInactive: root.isDarkTheme ?                 "#9A9A9A" : "#9a9996"  // 55% white or 55% black
        property color sideTextActive:                                      "#FFFFFF"

        property color batteryText: root.isDarkTheme ?                      "#3b3b3b" : "#F9F9F9"

    }

    // Universal font for all platforms? Doesn't work. WIP
    FontLoader {
        id: interFont
        source: "qrc:/ressources/fonts/inter_variable_font.ttf"
    }

    FontLoader {
        id: cabinFont
        source: "qrc:/ressources/fonts/inter_variable_font.ttf"
    }

    function swicthDevicesMode(mode) {
        if (mode === "devices_choice" || mode === "device_choice") {
            content_develop_arrow.state = "flipped"
            device_chosen_text.state = "devices_choice"
            current_device.state = "devices_choice"
            choose_device.state = "devices_choice"
            device_stroke.state = "devices_choice"
            serial_info_rect.state = "device_normal"
            devices_normal = false
            devices_extended = false
            devices_choice = true
        } else if (mode === "devices_extended" || mode === "device_extended") {
            content_develop_arrow.state = "normal"
            device_chosen_text.state = "devices_normal"
            current_device.state = "devices_normal"
            choose_device.state = "devices_normal"
            device_stroke.state = "device_extended"
            serial_info_rect.state = "device_extended"
            devices_normal = false
            devices_extended = true
            devices_choice = false
        } else {
            content_develop_arrow.state = "normal"
            device_chosen_text.state = "devices_normal"
            current_device.state = "devices_normal"
            choose_device.state = "devices_normal"
            device_stroke.state = "device_normal"
            serial_info_rect.state = "device_normal"
            devices_normal = true
            devices_extended = false
            devices_choice = false
        }
    }

    Rectangle {
        id: app
        color: root.colors.background
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
            property alias root: root
        }

        Rectangle {
            id: storage_info_stroke
            height: 50
            visible: true
            radius: 8
            border.width: 0
            z: 1
            anchors {
                left: content.left
                right: content.right
                bottom: content.bottom
                bottomMargin: 11
            }
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: root.colors.cardStroke
                }

                GradientStop {
                    position: 1
                    color: "#02000000"
                }
                orientation: Gradient.Vertical
            }
            anchors.leftMargin: 11
            anchors.rightMargin: 11
            Rectangle {
                id: storage_info
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
                        color: root.colors.cardBackgroundTop
                    }

                    GradientStop {
                        position: 1
                        color: root.colors.cardBackgroundBottom
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
                            color: root.colors.cardStroke
                        }

                        GradientStop {
                            position: 1
                            color: "#02000000"
                        }
                    }

                    Rectangle {
                        id: rectangle3
                        color: root.colors.batteryText
                        radius: 5
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
                    width: 100
                    radius: 5
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
                            color: root.colors.cardStroke
                        }

                        GradientStop {
                            position: 1
                            color: "#02000000"
                        }
                        orientation: Gradient.Vertical
                    }

                    Rectangle {
                        id: strorage_sync_button
                        y: 0
                        color: root.colors.batteryText
                        radius: 5
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
            id: content_bottom_shadow
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
            z: 0
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop {
                    position: 0
                    color: "#02000000"
                }

                GradientStop {
                    position: 1
                    color: "#1a000000"
                }
            }
        }

        Rectangle {
            id: sidebar_rectangle
            width: 280
            color: colors.sidebarBackground
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
                id: sidebar_bottom_fade
                visible: true
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                    rightMargin: 0
                    leftMargin: 0
                    bottomMargin: 0
                }
                z:1
                height: 50
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop {
                        position: 0
                        color: root.colors.sidebarBackgroundTransparent
                    }

                    GradientStop {
                        position: 1
                        color: root.colors.sidebarBackground
                    }
                }
            }

            Rectangle {
                id: device_stroke
                radius: 8
                border.color: "#00ffffff"
                border.width: 0
                height: 85
                width: 258
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
                        color: root.colors.cardStroke
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
                                height: 88
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
                    },
                    State {
                        name: "devices_choice"
                        PropertyChanges {
                            device_stroke {
                                height: devices_repeater.height+device_chosen_text.height+device.anchors.bottomMargin+device.anchors.topMargin+device_chosen_text.anchors.topMargin // This works. Why ? Idk, I played with it and eventually it worked lol
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
                    },
                    Transition {
                        to: "devices_choice"
                        NumberAnimation { properties: "height"; duration: 200; easing.type: Easing.OutCubic }
                    }
                ]
                state: "device_normal"

                Rectangle {
                    id: device
                    radius: 8
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
                            color: root.colors.cardBackgroundTop
                        }

                        GradientStop {
                            position: 0
                            color: root.colors.cardBackgroundBottom
                        }
                    }

                    Text {
                        id: device_chosen_text
                        opacity: 0.5
                        color: root.colors.textPrimary
                        text: qsTr("Device Chosen")
                        anchors {
                            left: parent.left
                            top: parent.top
                            topMargin: 8
                            leftMargin: 10
                        }
                        font.pointSize: 9
                        states: [ State {
                                name: "devices_normal"
                                PropertyChanges {
                                    device_chosen_text {
                                        text: qsTr("Device Chosen")
                                    }
                                }
                            },
                            State {
                                name: "devices_choice"
                                PropertyChanges {
                                    device_chosen_text {
                                        text: qsTr("Connected Devices")
                                    }
                                }
                            }
                        ]
                        state: "devices_normal"
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
                        source: "../images/glyphs/develop_arrow.png"
                        fillMode: Image.PreserveAspectFit
                        states: [ State {
                                name: "normal"
                                PropertyChanges {
                                    content_develop_arrow {
                                        rotation: 0
                                    }
                                }
                            },
                            State {
                                name: "flipped"
                                PropertyChanges {
                                    content_develop_arrow {
                                        rotation: 180
                                    }
                                }
                            } ]
                        transitions: [ Transition {
                                to: "normal"
                                NumberAnimation { properties: "rotation"; duration: 200; easing.type: Easing.OutCubic }
                            },
                            Transition {
                                to: "flipped"
                                NumberAnimation { properties: "rotation"; duration: 200; easing.type: Easing.OutCubic }
                            }
                        ]
                        state: "normal"
                    }
                    MouseArea {
                        id: m_content_develop_arrow
                        anchors.fill: content_develop_arrow
                        onPressed: content_develop_arrow.opacity=0.7
                        onReleased: {
                            content_develop_arrow.opacity=0.5
                            if (DeviceWatcher.device_connected) {
                                if (!devices_choice) {
                                    swicthDevicesMode("devices_choice")
                                } else {
                                    swicthDevicesMode("devices_normal")
                                }
                            }
                        }
                    }
                    Rectangle {
                        id: choose_device
                        color: "transparent"
                        anchors {
                            left: parent.left
                            top: device_chosen_text.bottom
                            right: parent.right
                            bottom: parent.bottom
                        }
                        opacity: 0.0
                        states: [ State {
                                name: "devices_normal"
                                PropertyChanges {
                                    choose_device {
                                        opacity: 0.0
                                    }
                                }
                            },
                            State {
                                name: "devices_choice"
                                PropertyChanges {
                                    choose_device {
                                        opacity: 1.0
                                    }
                                }
                            } ]
                        transitions: [ Transition {
                                to: "devices_normal"
                                NumberAnimation { properties: "opacity"; duration: 150; easing.type: Easing.OutCubic }
                            },
                            Transition {
                                to: "devices_choice"
                                NumberAnimation { properties: "opacity"; duration: 150; easing.type: Easing.OutCubic }
                            }
                        ]
                        state: "devices_normal"
                        Repeater {
                            id: devices_repeater // udidListChanged
                            model: []
                            delegate: Rectangle {
                                width: parent.width
                                color: "transparent"
                                height: 50
                                radius: 8
                                Component.onCompleted: {
                                    if (index === devices_repeater.count-1) {
                                        devices_repeater.height = devices_repeater.count*(height + 10)+8
                                    }
                                    if (DeviceWatcher.udid === modelData.udid) {
                                        color = "#3284ff"
                                        deviceNameTextColor = "#ffffff"
                                        deviceProductTypeTextColor = "#d9d9d9"
                                    } else {
                                        deviceNameTextColor = root.colors.textPrimary
                                        deviceProductTypeTextColor = root.colors.textSecondary
                                    }
                                }
                                property color deviceNameTextColor : "#ffffff"
                                property color deviceProductTypeTextColor : "#d9d9d9"
                                anchors {
                                    left: parent.left
                                    leftMargin: 10
                                    right: parent.right
                                    rightMargin: 10
                                    top: index === devices_repeater.count-1 ? devices_repeater.top : devices_repeater.itemAt(index+1).bottom // I have absolutely no idea why this works, but it works. I think a repeater counts backwards.
                                    topMargin: index === devices_repeater.count-1 ? 8 : 10
                                }

                                Image {
                                    id: deviceImage
                                    anchors {
                                        left: parent.left
                                        top: parent.top
                                        bottom: parent.bottom
                                        topMargin: 4
                                        bottomMargin: 4
                                    }
                                    width: 45
                                    source: encodeURIComponent(modelData.image)
                                    sourceSize.height: deviceImage.height
                                    fillMode: Image.PreserveAspectCrop
                                    smooth: true
                                }

                                Text {
                                    id: deviceNameText
                                    color: parent.deviceNameTextColor
                                    text: modelData.device_name
                                    anchors {
                                        left: deviceImage.right
                                        top: deviceImage.top
                                        topMargin: 2
                                        leftMargin: 0
                                        right: deviceBattery.left
                                        rightMargin: 4
                                    }
                                    font {
                                        weight: Font.DemiBold
                                        pointSize: 12
                                        family: interFont.name
                                    }
                                    maximumLineCount: 1
                                    wrapMode: Text.Wrap
                                    elide: Text.ElideRight
                                }
                                Text {
                                    id: deviceProductTypeText
                                    opacity: 1
                                    color: parent.deviceProductTypeTextColor
                                    text: modelData.product_type
                                    anchors {
                                        left: deviceImage.right
                                        top: deviceNameText.bottom
                                        leftMargin: 0
                                        topMargin: 1
                                    }
                                    font {
                                        family: interFont.name
                                        weight: Font.Medium
                                        pointSize: 9
                                    }
                                    wrapMode: Text.WordWrap
                                }
                                Rectangle {
                                    id: deviceBattery
                                    width: 24
                                    height: 11
                                    anchors {
                                        verticalCenter: deviceImage.verticalCenter
                                        right: parent.right
                                        rightMargin: 8
                                    }
                                    color: "transparent"
                                    visible: true

                                    Rectangle {
                                        id: deviceBatteryRect1
                                        width: 22
                                        opacity: 0.5
                                        color: root.colors.textPrimary
                                        radius: 3
                                        border.color: root.colors.textPrimary
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
                                        id: deviceBatteryRect2
                                        width: 1
                                        height: 5
                                        opacity: modelData.battery_string === "100" ? 1 : 0.5;
                                        color: modelData.battery_string === "100" ? "#6bcc43" : root.colors.textPrimary;
                                        anchors {
                                            right: parent.right
                                            rightMargin: 0
                                            verticalCenter: parent.verticalCenter
                                        }
                                    }

                                    Rectangle {
                                        id: deviceBatteryFill
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
                                        width: (modelData.battery)*22/100
                                        visible: true
                                    }

                                    Text {
                                        id: deviceBatteryText
                                        width: 20
                                        height: 11
                                        color: root.colors.batteryText
                                        text: modelData.battery_string
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.family: cabinFont.name
                                        font.weight: Font.Bold
                                        font.pointSize: 9
                                        anchors.fill: deviceBatteryRect1
                                    }
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onPressed: parent.color = "#203284ff"
                                    onReleased: {
                                        for (let i = 0; i < devices_repeater.count; i++) {
                                            devices_repeater.itemAt(i).color = "transparent"
                                            devices_repeater.itemAt(i).deviceNameTextColor = root.colors.textPrimary
                                            devices_repeater.itemAt(i).deviceProductTypeTextColor = root.colors.textSecondary
                                        }
                                        parent.color = "#3284ff"
                                        parent.deviceNameTextColor = "#ffffff"
                                        parent.deviceProductTypeTextColor = "#d9d9d9"
                                        if (DeviceWatcher.udid !== modelData.udid) {
                                            DeviceWatcher.switchCurrentDevice(modelData.udid)
                                        }
                                    }
                                    enabled: devices_choice
                                }
                            }
                        }
                        Connections {
                            target: DeviceWatcher
                            function onUdidListChanged() {
                                if (DeviceWatcher.device_connected) {
                                    devices_repeater.model = []
                                    devices_repeater.model = DeviceWatcher.getModel()
                                } else {
                                    devices_repeater.model = []
                                }
                            }
                        }
                    }
                    Rectangle {
                        id: current_device
                        color: "transparent"
                        anchors {
                            left: parent.left
                            top: device_chosen_text.bottom
                            right: parent.right
                            bottom: parent.bottom
                        }
                        opacity: 1.0
                        states: [ State {
                                name: "devices_normal"
                                PropertyChanges {
                                    current_device {
                                        opacity: 1.0
                                    }
                                }
                            },
                            State {
                                name: "devices_choice"
                                PropertyChanges {
                                    current_device {
                                        opacity: 0.0
                                    }
                                }
                            } ]
                        transitions: [ Transition {
                                to: "devices_normal"
                                NumberAnimation { properties: "opacity"; duration: 150; easing.type: Easing.OutCubic }
                            },
                            Transition {
                                to: "devices_choice"
                                NumberAnimation { properties: "opacity"; duration: 150; easing.type: Easing.OutCubic }
                            }
                        ]
                        state: "devices_normal"
                        Image {
                            id: current_device_image
                            width: 48
                            height: 53
                            anchors {
                                left: parent.left
                                top: parent.top
                                topMargin: 4
                                leftMargin: 2
                            }
                            source: "/images/iphone.png"
                            sourceSize.height: current_device_image.height
                            fillMode: Image.PreserveAspectCrop
                            smooth: true
                        }

                        Text {
                            id: device_name_text
                            color: root.colors.textPrimary
                            text: qsTr("No device connected")
                            anchors {
                                left: current_device_image.right
                                top: current_device_image.top
                                topMargin: 0
                                leftMargin: 2
                            }
                            font {
                                weight: Font.DemiBold
                                pointSize: 12
                                family: interFont.name
                            }
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
                                        if (current_device_image.source != DeviceWatcher.device_image) {
                                            current_device_image.source = DeviceWatcher.device_image
                                        }
                                    } else {
                                        device_name_text.text = "No device connected"
                                        current_device_image.source = "/images/iphone.png"
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
                            border.color: root.colors.textPrimary
                            border.width: 1
                            anchors {
                                right: parent.right
                                bottom: current_device_image.bottom
                                rightMargin: 10
                                bottomMargin: 2
                            }
                            radius: width*0.5
                            Text {
                                id: content_info_circle_text
                                text: "i"
                                height: 10
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                color: root.colors.textPrimary
                                anchors.fill: parent
                                font.family: interFont.name
                                font.pointSize: 9
                                font.bold: true
                            }
                            MouseArea {
                                anchors.fill: parent
                                onPressed: parent.opacity=0.7
                                onReleased: {
                                    parent.opacity=0.5
                                    if (DeviceWatcher.device_connected) {
                                        if (devices_normal) {
                                            swicthDevicesMode("device_extended")
                                        } else {
                                            swicthDevicesMode("device_normal")
                                        }
                                    }
                                }
                                enabled: !devices_choice
                            }
                        }

                        Text {
                            id: content_iphone_model
                            opacity: 1
                            color: colors.textSecondary
                            text: qsTr("Please connect a device to begin the sync")
                            anchors {
                                left: current_device_image.right
                                top: device_name_text.bottom
                                leftMargin: 2
                                topMargin: 0
                                right: parent.right
                                rightMargin: 40
                            }
                            font {
                                family: interFont.name
                                weight: Font.Medium
                                pointSize: 9
                            }
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
                            opacity: 1.0
                            color: "#00ffffff"
                            radius: 3
                            border.color: colors.textSecondary
                            border.width: 1
                            anchors {
                                left: current_device_image.right
                                top: content_iphone_model.bottom
                                topMargin: 2
                                leftMargin: 2
                            }
                            visible: false

                            Text {
                                id: content_storage_capacity_text
                                height: 14
                                opacity: 1
                                color: colors.textSecondary
                                text: qsTr("8GB")
                                anchors {
                                    verticalCenter: parent.verticalCenter
                                    horizontalCenter: parent.horizontalCenter
                                }
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.pointSize: 8
                                font.family: interFont.name
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
                            color: colors.textSecondary
                            text: qsTr("160.4 GB Available")
                            anchors {
                                verticalCenter: content_storage_capacity.verticalCenter
                                left: content_storage_capacity.right
                                leftMargin: 4
                            }
                            font {
                                weight: Font.Medium
                                pointSize: 9
                                family: interFont.name
                            }
                            visible: false
                            // TotalDataAvailable doesn't give the real storage left. To investigate.
                           Connections {
                               target: DeviceWatcher
                               function onCurrentDeviceChanged() {
                                   if (DeviceWatcher.device_connected) {
                                       content_storage_left.text = DeviceWatcher.storage_left + " Available"
                                       content_storage_left.visible = true
                                   } else {
                                       content_storage_left.visible = false
                                   }
                               }
                           }
                           states: [ State {
                                   name: "devices_normal"
                                   PropertyChanges {
                                       content_storage_capacity {
                                           opacity: 1.0
                                       }
                                   }
                               },
                               State {
                                   name: "devices_choice"
                                   PropertyChanges {
                                       content_storage_capacity {
                                           opacity: 0.0
                                       }
                                   }
                               } ]
                           transitions: [ Transition {
                                   to: "devices_normal"
                                   NumberAnimation { properties: "height"; duration: 200; easing.type: Easing.OutCubic }
                               },
                               Transition {
                                   to: "devices_choice"
                                   NumberAnimation { properties: "height"; duration: 200; easing.type: Easing.OutCubic }
                               }
                           ]
                           state: "devices_normal"
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
                                color: root.colors.textPrimary
                                radius: 3
                                border.color: root.colors.textPrimary
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

                            Rectangle {
                                id: device_battery_rect2
                                width: 1
                                height: 5
                                opacity: 0.5
                                color: root.colors.textPrimary
                                anchors {
                                    right: parent.right
                                    rightMargin: 0
                                    verticalCenter: parent.verticalCenter
                                }
                            }

                            Text {
                                id: device_battery_text
                                width: 20
                                height: 11
                                color: root.colors.batteryText
                                text: qsTr("82")
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.family: cabinFont.name
                                font.weight: Font.Bold
                                font.pointSize: 9
                                anchors.fill: device_battery_rect1
                            }
                            Connections {
                                target: DeviceWatcher
                                function onCurrentDeviceChanged() {
                                    if (DeviceWatcher.device_connected) {
                                        device_battery_text.text = DeviceWatcher.battery_string
                                        device_battery_fill.width = (DeviceWatcher.battery)*22/100
                                        if (DeviceWatcher.battery_string === "100") {
                                            device_battery_rect2.color = "#6bcc43"
                                            device_battery_rect2.opacity = 1
                                        } else {
                                            device_battery_rect2.color = root.colors.textPrimary
                                            device_battery_rect2.opacity = 0.5
                                        }
                                        content_battery.visible = true
                                    } else {
                                        content_battery.visible = false
                                    }
                                }
                            }
                            Connections {
                                target: ThemeManager
                                function onThemeChanged() {
                                    if (DeviceWatcher.device_connected) {
                                        device_battery_text.text = DeviceWatcher.battery_string
                                        device_battery_fill.width = (DeviceWatcher.battery)*22/100
                                        if (DeviceWatcher.battery_string === "100") {
                                            device_battery_rect2.color = "#6bcc43"
                                            device_battery_rect2.opacity = 1
                                        } else {
                                            device_battery_rect2.color = root.colors.textPrimary
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
                                top: current_device_image.bottom
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
                                color: root.colors.textPrimary
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
                                color: root.colors.textPrimary
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
                                color: root.colors.textPrimary
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
                                color: root.colors.textPrimary
                                anchors {
                                    top: imei_text.bottom
                                    left: imei_text.left
                                    topMargin: 1
                                    leftMargin: 0
                                }
                                font.pointSize: 8
                            }
                            MouseArea {
                                id: ecid_imei_mousearea
                                anchors {
                                    top: imei_text.top
                                    bottom: imei_info_text.bottom
                                    left: imei_text.left
                                    right: imei_info_text.right
                                }
                                onClicked: {
                                    if (DeviceWatcher.imei !== "") {
                                        if (imei_text.text == "IMEI") {
                                            imei_info_text.text = DeviceWatcher.ecid
                                            imei_text.text = "ECID"
                                        } else {
                                            imei_info_text.text = DeviceWatcher.imei
                                            imei_text.text = "IMEI"
                                        }
                                    }
                                }
                                enabled: devices_extended
                            }
                            Text {
                                id: udid_text
                                opacity: 0.5
                                color: root.colors.textPrimary
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
                                color: root.colors.textPrimary
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
                                        if (DeviceWatcher.imei == "") {
                                            imei_info_text.text = DeviceWatcher.ecid
                                            imei_text.text = "ECID"
                                        } else {
                                            imei_info_text.text = DeviceWatcher.imei
                                            imei_text.text = "IMEI"
                                        }
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
                        Connections {
                            target: DeviceWatcher
                            function onCurrentDeviceChanged() {
                                if (!DeviceWatcher.device_connected) {
                                    if (devices_extended) {
                                        swicthDevicesMode("device_normal")
                                    }
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
                color: root.colors.textPrimary
                text: qsTr("LiniTunes")
                anchors {
                    left: parent.left
                    top: parent.top
                    leftMargin: 14
                    topMargin: 14
                }
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignTop
                font.family: interFont.name
                font.pointSize: 18
                font.styleName: "Bold"
            }

            Rectangle {
                id: sidebar_buttons
                anchors {
                    left: parent.left
                    right: parent.right
                    top: device_stroke.bottom
                }
                color: "transparent"
                opacity: 1.0

                Rectangle {
                    id: clicked_rect
                    color: "#3284ff"
                    radius: 8
                    anchors {
                        right: sidebar_general_button.right
                        left: sidebar_general_button.left
                        top: sidebar_general_button.top
                        bottom: sidebar_general_button.bottom
                        rightMargin: 0
                        leftMargin: 0
                        topMargin: 0
                        bottomMargin: 0
                    }

                    states: [ State {
                            name: "sidebar_general_button"
                            AnchorChanges {
                                target : clicked_rect
                                anchors.right: sidebar_general_button.right
                                anchors.left: sidebar_general_button.left
                                anchors.top: sidebar_general_button.top
                                anchors.bottom: sidebar_general_button.bottom
                            }
                        },
                        State {
                            name: "sidebar_music_button"
                            AnchorChanges {
                                target : clicked_rect
                                anchors.right: sidebar_music_button.right
                                anchors.left: sidebar_music_button.left
                                anchors.top: sidebar_music_button.top
                                anchors.bottom: sidebar_music_button.bottom
                            }
                        },
                        State {
                            name: "sidebar_movies_button"
                            AnchorChanges {
                                target : clicked_rect
                                anchors.right: sidebar_movies_button.right
                                anchors.left: sidebar_movies_button.left
                                anchors.top: sidebar_movies_button.top
                                anchors.bottom: sidebar_movies_button.bottom
                            }
                        },
                        State {
                            name: "sidebar_tv_button"
                            AnchorChanges {
                                target : clicked_rect
                                anchors.right: sidebar_tv_button.right
                                anchors.left: sidebar_tv_button.left
                                anchors.top: sidebar_tv_button.top
                                anchors.bottom: sidebar_tv_button.bottom
                            }
                        },
                        State {
                            name: "sidebar_podcasts_button"
                            AnchorChanges {
                                target : clicked_rect
                                anchors.right: sidebar_podcasts_button.right
                                anchors.left: sidebar_podcasts_button.left
                                anchors.top: sidebar_podcasts_button.top
                                anchors.bottom: sidebar_podcasts_button.bottom
                            }
                        },
                        State {
                            name: "sidebar_audiobooks_button"
                            AnchorChanges {
                                target : clicked_rect
                                anchors.right: sidebar_audiobooks_button.right
                                anchors.left: sidebar_audiobooks_button.left
                                anchors.top: sidebar_audiobooks_button.top
                                anchors.bottom: sidebar_audiobooks_button.bottom
                            }
                        },
                        State {
                            name: "sidebar_books_button"
                            AnchorChanges {
                                target : clicked_rect
                                anchors.right: sidebar_books_button.right
                                anchors.left: sidebar_books_button.left
                                anchors.top: sidebar_books_button.top
                                anchors.bottom: sidebar_books_button.bottom
                            }
                        },
                        State {
                            name: "sidebar_photos_button"
                            AnchorChanges {
                                target : clicked_rect
                                anchors.right: sidebar_photos_button.right
                                anchors.left: sidebar_photos_button.left
                                anchors.top: sidebar_photos_button.top
                                anchors.bottom: sidebar_photos_button.bottom
                            }
                        },
                        State {
                            name: "sidebar_files_button"
                            AnchorChanges {
                                target : clicked_rect
                                anchors.right: sidebar_files_button.right
                                anchors.left: sidebar_files_button.left
                                anchors.top: sidebar_files_button.top
                                anchors.bottom: sidebar_files_button.bottom
                            }
                        } ]
                    transitions: [ Transition {
                            to: "sidebar_general_button"
                            AnchorAnimation { duration: 200; easing.type: Easing.OutCubic }
                        },
                        Transition {
                            to: "sidebar_music_button"
                            AnchorAnimation { duration: 200; easing.type: Easing.OutCubic }
                        },
                        Transition {
                            to: "sidebar_movies_button"
                            AnchorAnimation { duration: 200; easing.type: Easing.OutCubic }
                        },
                        Transition {
                            to: "sidebar_tv_button"
                            AnchorAnimation { duration: 200; easing.type: Easing.OutCubic }
                        },
                        Transition {
                            to: "sidebar_podcasts_button"
                            AnchorAnimation { duration: 200; easing.type: Easing.OutCubic }
                        },
                        Transition {
                            to: "sidebar_audiobooks_button"
                            AnchorAnimation { duration: 200; easing.type: Easing.OutCubic }
                        },
                        Transition {
                            to: "sidebar_books_button"
                            AnchorAnimation { duration: 200; easing.type: Easing.OutCubic }
                        },
                        Transition {
                            to: "sidebar_photos_button"
                            AnchorAnimation { duration: 200; easing.type: Easing.OutCubic }
                        },
                        Transition {
                            to: "sidebar_files_button"
                            AnchorAnimation { duration: 200; easing.type: Easing.OutCubic }
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
                        top: parent.top
                        topMargin: 9
                        rightMargin: 10
                        leftMargin: 10
                    }
                    opacity: 1.0
                    states: [
                        State { name: "clicked" },
                        State { name: "unclicked" }
                    ]

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
                        source: "../images/glyphs/settings.png"
                        fillMode: Image.PreserveAspectFit
                        visible: false
                    }

                    Text {
                        id: sidebar_general_button_text
                        // color: "#ffffff"
                        text: qsTr("General")
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: sidebar_general_button_image.right
                            leftMargin: 8
                        }
                        font.weight: Font.DemiBold
                        font.family: interFont.name
                        font.pointSize: 10
                        color: root.colors.sideTextActive
                        state: parent.state
                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_general_button_text; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_general_button_text; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_general_button_text; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_general_button_text; property: "color"; duration: 80 }
                            }
                        ]
                    }

                    ColorOverlay {
                        id: sidebar_general_button_image_overlay
                        anchors.fill: sidebar_general_button_image
                        source: sidebar_general_button_image
                        state: parent.state
                        color: root.colors.sideTextActive

                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_general_button_image_overlay; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_general_button_image_overlay; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_general_button_image_overlay; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_general_button_image_overlay; property: "color"; duration: 80 }
                            }
                        ]
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
                    opacity: 1.0
                    states: [
                        State { name: "clicked" },
                        State { name: "unclicked" }
                    ]
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
                        source: "../images/glyphs/music.png"
                        fillMode: Image.PreserveAspectFit
                        visible: false
                    }

                    Text {
                        id: sidebar_music_button_text
                        // color: "#ffffff"
                        text: qsTr("Music")
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: sidebar_music_button_image.right
                            leftMargin: 8
                        }
                        font.family: interFont.name
                        font.pointSize: 10
                        font.weight: Font.DemiBold
                        color: root.colors.sideTextInactive
                        state: parent.state
                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_music_button_text; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_music_button_text; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_music_button_text; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_music_button_text; property: "color"; duration: 80 }
                            }
                        ]
                    }

                    ColorOverlay {
                        id: sidebar_music_button_image_overlay
                        anchors.fill: sidebar_music_button_image
                        source: sidebar_music_button_image
                        state: parent.state
                        color: root.colors.sideTextInactive

                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_music_button_image_overlay; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_music_button_image_overlay; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_music_button_image_overlay; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_music_button_image_overlay; property: "color"; duration: 80 }
                            }
                        ]
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
                        rightMargin: 10
                    }
                    opacity: 1.0
                    states: [
                        State { name: "clicked" },
                        State { name: "unclicked" }
                    ]
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
                        source: "../images/glyphs/movies.png"
                        fillMode: Image.PreserveAspectFit
                        visible: false
                    }

                    Text {
                        id: sidebar_movies_button_text
                        // color: "#ffffff"
                        text: qsTr("Movies")
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: sidebar_movies_button_image.right
                            leftMargin: 8
                        }
                        font.family: interFont.name
                        font.pointSize: 10
                        font.weight: Font.DemiBold
                        color: root.colors.sideTextInactive
                        state: parent.state
                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_movies_button_text; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_movies_button_text; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_movies_button_text; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_movies_button_text; property: "color"; duration: 80 }
                            }
                        ]
                    }

                    ColorOverlay {
                        id: sidebar_movies_button_image_overlay
                        anchors.fill: sidebar_movies_button_image
                        source: sidebar_movies_button_image
                        state: parent.state
                        color: root.colors.sideTextInactive

                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_movies_button_image_overlay; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_movies_button_image_overlay; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_movies_button_image_overlay; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_movies_button_image_overlay; property: "color"; duration: 80 }
                            }
                        ]
                    }
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
                    opacity: 1.0
                    states: [
                        State { name: "clicked" },
                        State { name: "unclicked" }
                    ]
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
                        source: "../images/glyphs/tv.png"
                        fillMode: Image.PreserveAspectFit
                        visible: false
                    }

                    Text {
                        id: sidebar_tv_button_text
                        // color: "#ffffff"
                        text: qsTr("TV")
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: sidebar_tv_button_image.right
                            leftMargin: 8
                            rightMargin: 10
                        }
                        font.family: interFont.name
                        font.pointSize: 10
                        font.weight: Font.DemiBold
                        color: root.colors.sideTextInactive
                        state: parent.state
                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_tv_button_text; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_tv_button_text; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_tv_button_text; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_tv_button_text; property: "color"; duration: 80 }
                            }
                        ]
                    }

                    ColorOverlay {
                        id: sidebar_tv_button_image_overlay
                        anchors.fill: sidebar_tv_button_image
                        source: sidebar_tv_button_image
                        state: parent.state
                        color: root.colors.sideTextInactive

                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_tv_button_image_overlay; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_tv_button_image_overlay; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_tv_button_image_overlay; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_tv_button_image_overlay; property: "color"; duration: 80 }
                            }
                        ]
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
                    opacity: 1.0
                    states: [
                        State { name: "clicked" },
                        State { name: "unclicked" }
                    ]
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
                        source: "../images/glyphs/podcast.png"
                        fillMode: Image.PreserveAspectFit
                        visible: false
                    }

                    Text {
                        id: sidebar_podcasts_button_text
                        // color: "#ffffff"
                        text: qsTr("Podcasts")
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: sidebar_podcasts_button_image.right
                            leftMargin: 8
                        }
                        font.pointSize: 10
                        font.family: interFont.name
                        font.weight: Font.DemiBold
                        color: root.colors.sideTextInactive
                        state: parent.state
                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_podcasts_button_text; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_podcasts_button_text; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_podcasts_button_text; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_podcasts_button_text; property: "color"; duration: 80 }
                            }
                        ]
                    }

                    ColorOverlay {
                        id: sidebar_podcasts_button_image_overlay
                        anchors.fill: sidebar_podcasts_button_image
                        source: sidebar_podcasts_button_image
                        state: parent.state
                        color: root.colors.sideTextInactive

                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_podcasts_button_image_overlay; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_podcasts_button_image_overlay; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_podcasts_button_image_overlay; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_podcasts_button_image_overlay; property: "color"; duration: 80 }
                            }
                        ]
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
                    height: 40
                    color: "#00000000"
                    radius: 6
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: sidebar_podcasts_button.bottom
                        topMargin: 4
                        leftMargin: 10
                        rightMargin: 10
                    }
                    opacity: 1.0
                    states: [
                        State { name: "clicked" },
                        State { name: "unclicked" }
                    ]
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
                        source: "../images/glyphs/audiobook.png"
                        fillMode: Image.PreserveAspectFit
                        visible: false
                    }

                    Text {
                        id: sidebar_audiobooks_button_text
                        // color: "#ffffff"
                        text: qsTr("Audiobooks")
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: sidebar_audiobooks_button_image.right
                            leftMargin: 8
                        }
                        font.family: interFont.name
                        font.pointSize: 10
                        font.weight: Font.DemiBold
                        color: root.colors.sideTextInactive
                        state: parent.state
                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_audiobooks_button_text; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_audiobooks_button_text; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_audiobooks_button_text; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_audiobooks_button_text; property: "color"; duration: 80 }
                            }
                        ]
                    }

                    ColorOverlay {
                        id: sidebar_audiobooks_button_image_overlay
                        anchors.fill: sidebar_audiobooks_button_image
                        source: sidebar_audiobooks_button_image
                        state: parent.state
                        color: root.colors.sideTextInactive

                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_audiobooks_button_image_overlay; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_audiobooks_button_image_overlay; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_audiobooks_button_image_overlay; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_audiobooks_button_image_overlay; property: "color"; duration: 80 }
                            }
                        ]
                    }
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
                    opacity: 1.0
                    states: [
                        State { name: "clicked" },
                        State { name: "unclicked" }
                    ]
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
                            rightMargin: 10
                        }
                        source: "../images/glyphs/book.png"
                        fillMode: Image.PreserveAspectFit
                        visible: false
                    }

                    Text {
                        id: sidebar_books_button_text
                        // color: "#ffffff"
                        text: qsTr("Books")
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: sidebar_books_button_image.right
                            leftMargin: 8
                        }
                        font.family: interFont.name
                        font.pointSize: 10
                        font.weight: Font.DemiBold
                        color: root.colors.sideTextInactive
                        state: parent.state
                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_books_button_text; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_books_button_text; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_books_button_text; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_books_button_text; property: "color"; duration: 80 }
                            }
                        ]
                    }

                    ColorOverlay {
                        id: sidebar_books_button_image_overlay
                        anchors.fill: sidebar_books_button_image
                        source: sidebar_books_button_image
                        state: parent.state
                        color: root.colors.sideTextInactive

                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_books_button_image_overlay; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_books_button_image_overlay; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_books_button_image_overlay; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_books_button_image_overlay; property: "color"; duration: 80 }
                            }
                        ]
                    }
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
                    opacity: 1.0
                    states: [
                        State { name: "clicked" },
                        State { name: "unclicked" }
                    ]
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
                        source: "../images/glyphs/photo.png"
                        fillMode: Image.PreserveAspectFit
                        visible: false
                    }

                    Text {
                        id: sidebar_photos_button_text
                        // color: "#ffffff"
                        text: qsTr("Photos")
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: sidebar_photos_button_image.right
                            leftMargin: 8
                        }
                        font.family: interFont.name
                        font.pointSize: 10
                        font.weight: Font.DemiBold
                        color: root.colors.sideTextInactive
                        state: parent.state
                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_photos_button_text; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_photos_button_text; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_photos_button_text; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_photos_button_text; property: "color"; duration: 80 }
                            }
                        ]
                    }

                    ColorOverlay {
                        id: sidebar_photos_button_image_overlay
                        anchors.fill: sidebar_photos_button_image
                        source: sidebar_photos_button_image
                        state: parent.state
                        color: root.colors.sideTextInactive

                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_photos_button_image_overlay; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_photos_button_image_overlay; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_photos_button_image_overlay; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_photos_button_image_overlay; property: "color"; duration: 80 }
                            }
                        ]
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
                        rightMargin: 10
                    }
                    opacity: 1.0
                    states: [
                        State { name: "clicked" },
                        State { name: "unclicked" }
                    ]
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
                        source: "../images/glyphs/folder.png"
                        fillMode: Image.PreserveAspectFit
                        visible: false
                    }

                    Text {
                        id: sidebar_files_button_text
                        // color: "#ffffff"
                        text: qsTr("Files")
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: sidebar_files_button_image.right
                            leftMargin: 8
                        }
                        font.family: interFont.name
                        font.pointSize: 10
                        font.weight: Font.DemiBold
                        color: root.colors.sideTextInactive
                        state: parent.state
                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_files_button_text; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_files_button_text; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_files_button_text; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_files_button_text; property: "color"; duration: 80 }
                            }
                        ]
                    }

                    ColorOverlay {
                        id: sidebar_files_button_image_overlay
                        anchors.fill: sidebar_files_button_image
                        source: sidebar_files_button_image
                        state: parent.state
                        color: root.colors.sideTextInactive

                        states: [
                            State {
                                name: "clicked"
                                PropertyChanges { target: sidebar_files_button_image_overlay; color: root.colors.sideTextActive }
                            },
                            State {
                                name: "unclicked"
                                PropertyChanges { target: sidebar_files_button_image_overlay; color: root.colors.sideTextInactive }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "clicked"
                                ColorAnimation { target: sidebar_files_button_image_overlay; property: "color"; duration: 80 }
                            },
                            Transition {
                                to: "unclicked"
                                ColorAnimation { target: sidebar_files_button_image_overlay; property: "color"; duration: 80 }
                            }
                        ]
                    }
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
            }

            Rectangle {
                id: app_settings_button
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
                    width: 21
                    height: 21
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: "../images/glyphs/settings.png"
                    fillMode: Image.PreserveAspectFit
                    sourceSize.height: app_settings_button_image.height
                    sourceSize.width: app_settings_button_image.width
                }
                MouseArea {
                    id: m_app_settings_button
                    anchors.fill: parent
                    onPressed: parent.opacity=0.7
                    onReleased: {
                        parent.opacity=1.0
                        console.log("App settings not implemented yet")
                    }
                }
            }
        }
    }

}
