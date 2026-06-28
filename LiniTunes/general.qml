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
                font.pixelSize: 19
                font.family: AppFontFamily
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
                font.pixelSize: 14
                font.family: AppFontFamily
                lineHeight: 1.4
            }

            // ---- Backup Section ----
            Text {
                text: qsTr("Backup")
                color: root.colors.textPrimary
                font.pixelSize: 19
                font.family: AppFontFamily
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
                        font.pixelSize: 12
                        font.family: AppFontFamily
                        elide: Text.ElideMiddle
                        width: parent.width - 16
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: backupFolderDialog.open()
                    }
                }

                Rectangle {
                    width: 110
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
                        text: generalPage.backupButtonText()
                        color: "#ffffff"
                        font.pixelSize: 14
                        font.family: AppFontFamily
                        font.weight: Font.DemiBold
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: DeviceWatcher.device_connected || DeviceWatcher.backup_running
                        onClicked: {
                            if (DeviceWatcher.backup_running) {
                                generalPage.cancellationRequested = true
                                DeviceWatcher.stopBackup()
                            } else if (generalPage.backupPath !== "") {
                                generalPage.cancellationRequested = false
                                DeviceWatcher.startBackup(generalPage.backupPath)
                            }
                        }
                    }
                }
            }

            Text {
                visible: DeviceWatcher.backup_running
                text: qsTr("Keep your iPhone unlocked and connected until the backup finishes.")
                color: root.colors.textSecondary
                font.pixelSize: 12
                font.family: AppFontFamily
                wrapMode: Text.WordWrap
                width: parent.width
            }

            // Backup progress
            Column {
                visible: DeviceWatcher.backup_info !== null && DeviceWatcher.backup_info.status !== "idle"
                width: parent.width
                spacing: 6

                Text {
                    text: generalPage.backupStatusText()
                    color: generalPage.backupStatusColor()
                    font.pixelSize: 14
                    font.family: AppFontFamily
                    font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                    width: parent.width
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
                    font.pixelSize: 12
                    font.family: AppFontFamily
                }
            }
        }
    }

    property string backupPath: ""
    property bool cancellationRequested: false

    Connections {
        target: DeviceWatcher
        function onBackupChanged() {
            if (!DeviceWatcher.backup_running)
                generalPage.cancellationRequested = false
        }
    }

    FolderDialog {
        id: backupFolderDialog
        title: qsTr("Choose backup folder")
        onAccepted: {
            generalPage.backupPath = selectedFolder.toString().replace("file://", "")
        }
    }

    function backupButtonText() {
        if (generalPage.cancellationRequested && DeviceWatcher.backup_running)
            return qsTr("Cancelling...")
        if (DeviceWatcher.backup_running)
            return qsTr("Cancel")
        return qsTr("Backup")
    }

    function backupStatusText() {
        if (!DeviceWatcher.backup_info)
            return ""

        var status = DeviceWatcher.backup_info.status
        if (status === "running") {
            if (generalPage.cancellationRequested)
                return qsTr("Cancelling...")
            return qsTr("Backing up... ") + DeviceWatcher.backup_info.progress.toFixed(1) + "%"
        }
        if (status === "completed")
            return qsTr("Completed")
        if (status === "completed_with_warnings")
            return qsTr("Completed with warnings: ") + DeviceWatcher.backup_info.warning
        if (status === "failed")
            return qsTr("Failed: ") + generalPage.friendlyBackupError(DeviceWatcher.backup_info.error)
        if (status === "cancelled")
            return qsTr("Cancelled")
        return ""
    }

    function backupStatusColor() {
        if (!DeviceWatcher.backup_info)
            return root.colors.textSecondary

        var status = DeviceWatcher.backup_info.status
        if (status === "completed")
            return root.colors.green
        if (status === "completed_with_warnings" || status === "cancelled")
            return root.colors.yellow
        if (status === "failed")
            return root.colors.red
        return root.colors.textPrimary
    }

    function friendlyBackupError(error) {
        if (!error) return qsTr("Unknown error")
        var lower = error.toLowerCase()
        if (lower.indexOf("insufficient free disk space") >= 0 || lower.indexOf("not enough free disk space") >= 0 || lower.indexOf("mberrordomain/105") >= 0) {
            return qsTr("Not enough free disk space to complete this backup. Free up space on the backup drive or choose another backup folder.")
        }
        if (lower.indexOf("lock") >= 0 || lower.indexOf("passcode") >= 0 || lower.indexOf("unavailable") >= 0 || lower.indexOf("connection") >= 0) {
            return error + qsTr(" Please unlock the iPhone, keep it connected, and try again.")
        }
        if (lower.indexOf("incomplete") >= 0) {
            return error + qsTr(" Keep the iPhone unlocked and retry the backup.")
        }
        return error
    }

    function formatBytes(bytes) {
        if (bytes >= 1073741824) return (bytes / 1073741824).toFixed(1) + " GB"
        if (bytes >= 1048576) return (bytes / 1048576).toFixed(1) + " MB"
        if (bytes >= 1024) return (bytes / 1024).toFixed(1) + " KB"
        return bytes + " B"
    }
}