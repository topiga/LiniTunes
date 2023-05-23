import QtQuick 2.0


Window {
    minimumWidth: 960
    minimumHeight: 600
    width: 960
    height: 600
    visible: true
    title: qsTr("LiniTunes")
    color: "#00000000"
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
            radius: 5
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
                    NumberAnimation { properties: "y, x, height, width"; duration: 130; easing.type: Easing.InOutExpo }
                },
                Transition {
                    to: "button_idevice"
                    NumberAnimation { properties: "y, x, height, width"; duration: 130; easing.type: Easing.InOutExpo }
                },
                Transition {
                    to: "button_files"
                    NumberAnimation { properties: "y, x, height, width"; duration: 130; easing.type: Easing.InOutExpo }
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
            }
            Text {
                text: "No device connected"
                anchors {
                    left: img_idevice.right
                    top: parent.top
                    right: parent.right
                    leftMargin: 3
                    topMargin: 10
                }
                font {
                    bold: true
                    family: "Helvetica"
                    pointSize: 13
                }
                color: color_text
                wrapMode: Text.WordWrap
            }
            MouseArea {
                id: m_idevice
                anchors.fill: parent
                onClicked: {
                    clicked_rect.state = "button_idevice"
                    main_page.source = "/idevices.qml"
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
    Loader {
        id: main_page
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
            left: sidebar.right
        }
        source: "/test.qml"
    }
}
