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
            } ]
            transitions: [ Transition {
                to: "button_general"
                NumberAnimation { properties: "y, x, height, width"; duration: 100; easing.type: Easing.InOutExpo }
            },
            Transition {
                to: "button_idevice"
                NumberAnimation { properties: "y, x, height, width"; duration: 100; easing.type: Easing.InOutExpo }
            }]
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
                source: "/images/iDevice_90x90.png"
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
            Text {
                text: "General"
                anchors {
                    left: parent. left
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
