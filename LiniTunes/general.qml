import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Item {
    id: generalPage

    Rectangle {
        anchors.fill: parent
        color: root.colors.contentBackground

        Column {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: 20
            }
            spacing: 12

            // ---- Device Info Section ----
            Text {
                text: qsTr("Device Info")
                color: root.colors.textPrimary
                font.pointSize: 14
                font.family: interFont.name
                font.weight: Font.Bold
            }

            Rectangle {
                width: parent.width
                height: 1
                color: root.colors.divider
            }

            Text {
                text: DeviceWatcher.device_connected
                    ? qsTr("Name: ") + DeviceWatcher.device_name + "\\n"
                      + qsTr("Model: ") + DeviceWatcher.marketing_name + "\\n"
                      + qsTr("Serial: ") + DeviceWatcher.serial + "\\n"
                      + qsTr("Battery: ") + DeviceWatcher.battery_string + "%"
                    : qsTr("No device connected")
                color: root.colors.textSecondary
                font.pointSize: 10
                font.family: interFont.name
                lineHeight: 1.4
            }

            // ---- Backup Section ----
            Text {
                text: qsTr("Backup")
                color: root.colors.textPrimary
                font.pointSize: 14
                font.family: interFont.name
                font.weight: Font.Bold
            }

            Rectangle {
                width: parent.width
                height: 1
                color: root.colors.divider
            }

            Row {
                spacing: 10

                Rectangle {
                    width: backupPathLabel.implicitWidth + 20
                    height: 32
                    radius: 6
                    color: root.colors.settingsButtonBg
                    border.color: root.colors.cardStroke
                    border.width: 1

                    Text {
                        id: backupPathLabel
                        anchors.centerIn: parent
                        text: generalPage.backupPath === "" ? qsTr("Choose backup folder...") : generalPage.backupPath
                        color: generalPage.backupPath === "" ? root.colors.sideTextInactive : root.colors.textPrimary
                        font.pointSize: 9
                        font.family: interFont.name
                        elide: Text.ElideMiddle
                        width: parent.width - 16
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: backupFolderDialog.open()
                    }
                }

                Rectangle {
                    width: 80
                    height: 32
                    radius: 6
                    color: {
                        if (DeviceWatcher.backup_running) return root.colors.red
                        if (!DeviceWatcher.device_connected || generalPage.backupPath === "") return root.colors.buttonHover
                        return root.colors.accent
                    }
                    opacity: (!DeviceWatcher.device_connected || generalPage.backupPath === "") && !DeviceWatcher.backup_running ? 0.5 : 1.0

                    Text {
                        anchors.centerIn: parent
                        text: DeviceWatcher.backup_running ? qsTr("Cancel") : qsTr("Backup")
                        color: "#ffffff"
                        font.pointSize: 10
                        font.family: interFont.name
                        font.weight: Font.DemiBold
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: DeviceWatcher.device_connected || DeviceWatcher.backup_running
                        onClicked: {
                            if (DeviceWatcher.backup_running) {
                                DeviceWatcher.stopBackup()
                            } else if (generalPage.backupPath !== "") {
                                DeviceWatcher.startBackup(generalPage.backupPath)
                            }
                        }
                    }
                }
            }

            // Backup progress
            Column {
                visible: DeviceWatcher.backup_info !== null && DeviceWatcher.backup_info.status !== "idle"
                width: parent.width
                spacing: 6

                Text {
                    text: {
                        if (!DeviceWatcher.backup_info) return ""
                        var s = DeviceWatcher.backup_info.status
                        if (s === "running") return qsTr("Backing up... ") + DeviceWatcher.backup_info.progress + "%"
                        if (s === "completed") return qsTr("Backup completed!")
                        if (s === "failed") return qsTr("Backup failed: ") + DeviceWatcher.backup_info.error
                        if (s === "cancelled") return qsTr("Backup cancelled")
                        return ""
                    }
                    color: {
                        if (!DeviceWatcher.backup_info) return root.colors.textSecondary
                        var s = DeviceWatcher.backup_info.status
                        if (s === "completed") return root.colors.green
                        if (s === "failed") return root.colors.red
                        if (s === "cancelled") return root.colors.yellow
                        return root.colors.textPrimary
                    }
                    font.pointSize: 10
                    font.family: interFont.name
                    font.weight: Font.DemiBold
                }

                Rectangle {
                    width: parent.width
                    height: 6
                    radius: 3
                    color: root.colors.cardStroke

                    Rectangle {
                        width: parent.width * (DeviceWatcher.backup_info ? DeviceWatcher.backup_info.progress / 100 : 0)
                        height: parent.height
                        radius: 3
                        color: root.colors.accent

                        Behavior on width {
                            NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                        }
                    }
                }

                Text {
                    text: {
                        if (!DeviceWatcher.backup_info || DeviceWatcher.backup_info.bytesTotal === 0) return ""
                        return formatBytes(DeviceWatcher.backup_info.bytesDone) + " / " + formatBytes(DeviceWatcher.backup_info.bytesTotal)
                    }
                    color: root.colors.textSecondary
                    font.pointSize: 9
                    font.family: interFont.name
                }
            }
        }
    }

    property string backupPath: ""

    FolderDialog {
        id: backupFolderDialog
        title: qsTr("Choose backup folder")
        onAccepted: {
            generalPage.backupPath = selectedFolder.toString().replace("file://", "")
        }
    }

    function formatBytes(bytes) {
        if (bytes >= 1073741824) return (bytes / 1073741824).toFixed(1) + " GB"
        if (bytes >= 1048576) return (bytes / 1048576).toFixed(1) + " MB"
        if (bytes >= 1024) return (bytes / 1024).toFixed(1) + " KB"
        return bytes + " B"
    }
}