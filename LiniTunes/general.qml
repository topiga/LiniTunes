import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects
import "qml/components"

Item {
    id: generalPage

    property string backupPath: DeviceWatcher.backup_folder
    property bool cancellationRequested: false
    property int backupMode: DeviceWatcher.backup_encryption_status === "enabled" ? 1 : 0 // 0 = standard local backup, 1 = encrypted local backup
    property string backupPasswordForRun: ""
    property bool pendingBackupAfterPassword: false
    property string backupPasswordError: ""
    property string disablePasswordError: ""
    property string changePasswordError: ""
    property var backupDevices: []
    property string expandedBackupUdid: ""
    property string pendingDeleteBackupPath: ""
    property string pendingDeleteBackupDate: ""
    property string pendingFolderAction: ""

    component BackupOption: Rectangle {
        id: optionRoot
        property string title: ""
        property string details: ""
        property bool checked: false
        signal selected()

        width: parent ? parent.width : 0
        implicitHeight: optionTextColumn.implicitHeight + 4
        radius: 0
        color: "transparent"
        border.width: 0
        opacity: enabled ? 1.0 : 0.45

        Rectangle {
            id: radioOuter
            width: 16
            height: 16
            radius: 8
            color: "transparent"
            border.width: 1
            border.color: optionRoot.checked ? root.colors.accent : root.colors.textSecondary
            anchors { left: parent.left; top: parent.top; leftMargin: 0; topMargin: 2 }

            Rectangle {
                width: 8; height: 8; radius: 4
                visible: optionRoot.checked
                color: root.colors.accent
                anchors.centerIn: parent
            }
        }

        Column {
            id: optionTextColumn
            anchors { left: radioOuter.right; right: parent.right; top: parent.top; leftMargin: 10 }
            spacing: 4

            Text {
                text: optionRoot.title
                color: root.colors.textPrimary
                font.pixelSize: 13; font.family: AppFontFamily; font.weight: Font.DemiBold
                wrapMode: Text.WordWrap; width: parent.width
            }
            Text {
                text: optionRoot.details
                color: root.colors.textSecondary
                font.pixelSize: 12; font.family: AppFontFamily
                wrapMode: Text.WordWrap; width: parent.width
            }
        }

        MouseArea {
            anchors { left: radioOuter.left; right: optionTextColumn.right; top: radioOuter.top; bottom: optionTextColumn.bottom }
            enabled: optionRoot.enabled
            onClicked: optionRoot.selected()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: root.colors.contentBackground

        Flickable {
            anchors.fill: parent
            contentWidth: width
            contentHeight: pageColumn.implicitHeight + 40
            clip: true

            Column {
                id: pageColumn
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    margins: 20
                }
                spacing: 12

                Card {
    colors: root.colors
                    visible: DeviceWatcher.device_connected
                    title: qsTr("Software")

                    Row {
                        width: parent.width
                        spacing: 18
                        topPadding: 0
                        bottomPadding: 0

                        Item {
                            id: softwareImage
                            width: 72
                            height: 72

                            Image {
                                id: softwareImageSource
                                anchors.fill: parent
                                source: DeviceWatcher.software_image !== "" ? DeviceWatcher.software_image : "/images/software/Generic_AppleOS.png"
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                                mipmap: true
                                sourceSize.width: 216
                                sourceSize.height: 216
                            }

                            Text {
                                anchors.centerIn: parent
                                visible: DeviceWatcher.software_image.indexOf("Background_AppleOS") >= 0
                                text: generalPage.softwareVersionBadgeText()
                                color: "#ffffff"
                                font.family: AppFontFamily
                                font.weight: Font.DemiBold
                                font.pixelSize: 15
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        Column {
                            width: parent.width - softwareImage.width - softwareButtonColumn.width - (parent.spacing * 2)
                            spacing: 4

                            Text {
                                text: DeviceWatcher.device_connected
                                      ? generalPage.softwareVersionLine()
                                      : qsTr("No device connected")
                                color: root.colors.textPrimary
                                font.pixelSize: 18
                                font.family: AppFontFamily
                                font.weight: Font.Bold
                            }

                            Text {
                                text: DeviceWatcher.device_connected
                                      ? qsTr("Your %1 is up to date. LiniTunes will automatically check for updates again on %2.").arg(generalPage.deviceTypeLabel()).arg(generalPage.nextUpdateCheckDate())
                                      : qsTr("Connect a device to check for software updates.")
                                color: root.colors.textSecondary
                                font.pixelSize: 12
                                font.family: AppFontFamily
                                wrapMode: Text.WordWrap
                                width: parent.width
                            }

                            Text {
                                visible: DeviceWatcher.device_connected && DeviceWatcher.build_version !== ""
                                text: qsTr("Build ") + DeviceWatcher.build_version
                                color: root.colors.textSecondary
                                opacity: 0.75
                                font.pixelSize: 12
                                font.family: AppFontFamily
                            }
                        }

                        Column {
                            id: softwareButtonColumn
                            width: 150
                            spacing: 6

                            AppButton {
    colors: root.colors
                                width: parent.width
                                label: qsTr("Check for Updates")
                                enabled: false
                            }
                            AppButton {
    colors: root.colors
                                width: parent.width
                                label: qsTr("Restore...")
                                enabled: false
                            }
                        }
                    }
                }

                Card {
    colors: root.colors
                    title: qsTr("Backups")

                    Text {
                        text: generalPage.backupPath === ""
                              ? qsTr("LiniTunes will ask for a backup folder when needed. You can change it later in Settings.")
                              : qsTr("Backup folder: ") + generalPage.backupPath + qsTr("  ·  Change it later in Settings.")
                        color: root.colors.textSecondary
                        font.pixelSize: 12
                        font.family: AppFontFamily
                        elide: Text.ElideMiddle
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }

                    Column {
                        width: parent.width
                        spacing: 8

                        BackupOption {
                            title: qsTr("Back up your most important data of this device to this computer")
                            details: DeviceWatcher.backup_encryption_status === "enabled"
                                     ? qsTr("Uses a local unencrypted backup behavior.")
                                     : qsTr("Uses the current local unencrypted backup behavior.")
                            checked: generalPage.backupMode === 0
                            enabled: DeviceWatcher.device_connected
                            onSelected: generalPage.requestStandardBackupMode()
                        }

                        BackupOption {
                            title: qsTr("Back up all of the data of this iPhone to this computer (Encrypted)")
                            details: DeviceWatcher.backup_encryption_status === "enabled"
                                     ? qsTr("Encrypted local backup is enabled for this device.")
                                     : qsTr("Encrypt local backup. A password is required and cannot be recovered if forgotten.")
                            checked: generalPage.backupMode === 1
                            enabled: DeviceWatcher.device_connected
                            onSelected: generalPage.requestEncryptedBackupMode()
                        }
                    }

                    Text {
                        visible: DeviceWatcher.backup_running
                        text: DeviceWatcher.backup_encryption_status === "enabled" || generalPage.backupMode === 1
                              ? qsTr("Keep your iPhone unlocked and connected. If prompted, confirm backup encryption on the iPhone.")
                              : qsTr("Keep your iPhone unlocked and connected until the backup finishes.")
                        color: root.colors.textSecondary
                        font.pixelSize: 12
                        font.family: AppFontFamily
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }

                    Text {
                        visible: DeviceWatcher.backup_info !== null && DeviceWatcher.backup_info.status !== "idle"
                        text: generalPage.backupStatusText()
                        color: generalPage.backupStatusColor()
                        font.pixelSize: 13
                        font.family: AppFontFamily
                        font.weight: Font.DemiBold
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }

                    Text {
                        visible: generalPage.backupPasswordError !== ""
                        text: generalPage.backupPasswordError
                        color: root.colors.red
                        font.pixelSize: 12
                        font.family: AppFontFamily
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }

                    Text {
                        visible: DeviceWatcher.backup_encryption_busy || DeviceWatcher.backup_encryption_error !== ""
                        text: DeviceWatcher.backup_encryption_busy
                              ? qsTr("Updating backup encryption…")
                              : generalPage.friendlyBackupError(DeviceWatcher.backup_encryption_error)
                        color: DeviceWatcher.backup_encryption_busy ? root.colors.textSecondary : root.colors.red
                        font.pixelSize: 12
                        font.family: AppFontFamily
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }

                    Row {
                        spacing: 10
                        AppButton {
    colors: root.colors
                            label: qsTr("Manage Backups")
                            onClicked: generalPage.ensureBackupFolder("manage")
                        }
                        AppButton {
    colors: root.colors
                            label: DeviceWatcher.backup_running ? qsTr("Cancel") : qsTr("Back Up Now")
                            primary: !DeviceWatcher.backup_running
                            destructive: DeviceWatcher.backup_running
                            enabled: DeviceWatcher.device_connected && !DeviceWatcher.backup_encryption_busy
                            onClicked: {
                                if (DeviceWatcher.backup_running) {
                                    generalPage.cancellationRequested = true
                                    DeviceWatcher.stopBackup()
                                } else {
                                    generalPage.ensureBackupFolder("backup")
                                }
                            }
                        }
                        AppButton {
    colors: root.colors
                            label: qsTr("Restore Backup")
                            enabled: false
                        }
                        AppButton {
    colors: root.colors
                            label: qsTr("Change Password")
                            enabled: DeviceWatcher.device_connected && DeviceWatcher.backup_encryption_status === "enabled" && !DeviceWatcher.backup_encryption_busy
                            onClicked: changePasswordPopup.open()
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: DeviceWatcher
        function onBackupChanged() {
            if (DeviceWatcher.backup_encryption_status === "enabled")
                generalPage.backupMode = 1
            if (!DeviceWatcher.backup_running) {
                generalPage.cancellationRequested = false
                generalPage.backupPasswordForRun = ""
                generalPage.pendingBackupAfterPassword = false
            }
            if (DeviceWatcher.backup_encryption_status === "disabled" && generalPage.backupMode === 1 && !generalPage.pendingBackupAfterPassword)
                generalPage.backupMode = 0
            if (!DeviceWatcher.backup_encryption_busy && DeviceWatcher.backup_encryption_error === "") {
                if (disablePasswordPopup.visible) {
                    disablePasswordPopup.close()
                }
                if (changePasswordPopup.visible) {
                    changePasswordPopup.close()
                }
            }
            if (DeviceWatcher.backup_encryption_error !== "") {
                if (disablePasswordPopup.visible) {
                    generalPage.disablePasswordError = generalPage.friendlyBackupError(DeviceWatcher.backup_encryption_error)
                }
                if (changePasswordPopup.visible) {
                    generalPage.changePasswordError = generalPage.friendlyBackupError(DeviceWatcher.backup_encryption_error)
                }
            }
        }

        function onCurrentDeviceChanged() {
            generalPage.backupMode = DeviceWatcher.backup_encryption_status === "enabled" ? 1 : 0
            generalPage.backupPasswordError = ""
            generalPage.disablePasswordError = ""
            generalPage.changePasswordError = ""
            generalPage.backupPasswordForRun = ""
            generalPage.pendingBackupAfterPassword = false
        }
    }

    FolderDialog {
        id: backupFolderDialog
        title: qsTr("Choose backup folder")
        onAccepted: {
            DeviceWatcher.backup_folder = selectedFolder.toLocalFile()
            generalPage.resumePendingFolderAction()
        }
    }

    ModalPanel {
    colors: root.colors
        id: chooseBackupFolderPopup
        title: qsTr("Choose Backup Folder")

        Text {
            text: qsTr("Choose where LiniTunes should store and look for local backups. The path will be remembered and can be changed later in Settings.")
            color: root.colors.textSecondary
            font.pixelSize: 12
            font.family: AppFontFamily
            wrapMode: Text.WordWrap
            width: parent.width
        }

        Text {
            visible: generalPage.backupPath !== ""
            text: qsTr("Current folder: ") + generalPage.backupPath
            color: root.colors.textSecondary
            font.pixelSize: 12
            font.family: AppFontFamily
            elide: Text.ElideMiddle
            width: parent.width
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            AppButton {
    colors: root.colors
                label: qsTr("Cancel")
                onClicked: {
                    generalPage.pendingFolderAction = ""
                    chooseBackupFolderPopup.close()
                }
            }
            AppButton {
    colors: root.colors
                label: qsTr("Choose Folder")
                primary: true
                onClicked: {
                    chooseBackupFolderPopup.close()
                    backupFolderDialog.open()
                }
            }
        }
    }

    ModalPanel {
    colors: root.colors
        id: passwordSetupPopup
        title: qsTr("Set Backup Password")
        onClosed: {
            setupPasswordField.text = ""
            setupPasswordConfirmField.text = ""
        }

        Text {
            text: qsTr("This password protects encrypted local backups. LiniTunes cannot recover it if it is forgotten.")
            color: root.colors.textSecondary
            font.pixelSize: 12
            font.family: AppFontFamily
            wrapMode: Text.WordWrap
            width: parent.width
        }

        PasswordField {
    colors: root.colors
            id: setupPasswordField
            width: parent.width
            placeholderText: qsTr("Backup password")
        }

        PasswordField {
    colors: root.colors
            id: setupPasswordConfirmField
            width: parent.width
            placeholderText: qsTr("Confirm password")
        }

        Text {
            visible: generalPage.backupPasswordError !== ""
            text: generalPage.backupPasswordError
            color: root.colors.red
            font.pixelSize: 12
            font.family: AppFontFamily
            wrapMode: Text.WordWrap
            width: parent.width
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            AppButton {
    colors: root.colors
                label: qsTr("Cancel")
                onClicked: {
                    generalPage.pendingBackupAfterPassword = false
                    generalPage.backupMode = DeviceWatcher.backup_encryption_status === "enabled" ? 1 : 0
                    passwordSetupPopup.close()
                }
            }
            AppButton {
    colors: root.colors
                label: qsTr("Use Password")
                primary: true
                onClicked: generalPage.acceptBackupPasswordSetup()
            }
        }
    }

    ModalPanel {
    colors: root.colors
        id: disableEncryptionConfirmPopup
        title: qsTr("Disable Encrypted Backups?")

        Text {
            text: qsTr("Important data backups do not include all encrypted-backup data. To switch back, encrypted local backups must be disabled for this iPhone.")
            color: root.colors.textSecondary
            font.pixelSize: 12
            font.family: AppFontFamily
            wrapMode: Text.WordWrap
            width: parent.width
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            AppButton {
    colors: root.colors
                label: qsTr("Go Back")
                onClicked: disableEncryptionConfirmPopup.close()
            }
            AppButton {
    colors: root.colors
                label: qsTr("Disable Encryption")
                destructive: true
                onClicked: {
                    disableEncryptionConfirmPopup.close()
                    disablePasswordPopup.open()
                }
            }
        }
    }

    ModalPanel {
    colors: root.colors
        id: disablePasswordPopup
        title: qsTr("Enter Current Backup Password")
        onClosed: disablePasswordField.text = ""

        Text {
            text: qsTr("Enter the current encrypted backup password to disable encrypted local backups. The password will not be logged or stored.")
            color: root.colors.textSecondary
            font.pixelSize: 12
            font.family: AppFontFamily
            wrapMode: Text.WordWrap
            width: parent.width
        }

        PasswordField {
    colors: root.colors
            id: disablePasswordField
            width: parent.width
            placeholderText: qsTr("Current backup password")
        }

        Text {
            visible: generalPage.disablePasswordError !== ""
            text: generalPage.disablePasswordError
            color: root.colors.red
            font.pixelSize: 12
            font.family: AppFontFamily
            wrapMode: Text.WordWrap
            width: parent.width
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            AppButton {
    colors: root.colors
                label: qsTr("Cancel")
                onClicked: disablePasswordPopup.close()
            }
            AppButton {
    colors: root.colors
                label: DeviceWatcher.backup_encryption_busy ? qsTr("Disabling…") : qsTr("Disable")
                destructive: true
                enabled: !DeviceWatcher.backup_encryption_busy
                onClicked: generalPage.disableEncryptionWithPassword()
            }
        }
    }

    ModalPanel {
    colors: root.colors
        id: changePasswordPopup
        title: qsTr("Change Backup Password")
        onClosed: {
            changeOldPasswordField.text = ""
            changeNewPasswordField.text = ""
            changeConfirmPasswordField.text = ""
        }

        Text {
            text: qsTr("Change the encrypted local backup password for this iPhone.")
            color: root.colors.textSecondary
            font.pixelSize: 12
            font.family: AppFontFamily
            wrapMode: Text.WordWrap
            width: parent.width
        }

        PasswordField {
    colors: root.colors
            id: changeOldPasswordField
            width: parent.width
            placeholderText: qsTr("Current password")
        }
        PasswordField {
    colors: root.colors
            id: changeNewPasswordField
            width: parent.width
            placeholderText: qsTr("New password")
        }
        PasswordField {
    colors: root.colors
            id: changeConfirmPasswordField
            width: parent.width
            placeholderText: qsTr("Confirm new password")
        }

        Text {
            visible: generalPage.changePasswordError !== ""
            text: generalPage.changePasswordError
            color: root.colors.red
            font.pixelSize: 12
            font.family: AppFontFamily
            wrapMode: Text.WordWrap
            width: parent.width
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            AppButton {
    colors: root.colors
                label: qsTr("Cancel")
                onClicked: changePasswordPopup.close()
            }
            AppButton {
    colors: root.colors
                label: DeviceWatcher.backup_encryption_busy ? qsTr("Changing…") : qsTr("Change Password")
                primary: true
                enabled: !DeviceWatcher.backup_encryption_busy
                onClicked: generalPage.changeBackupPassword()
            }
        }
    }

    ModalPanel {
    colors: root.colors
        id: manageBackupsPopup
        title: qsTr("Manage Backups")
        width: Math.min(620, generalPage.width - 40)

        Text {
            visible: generalPage.backupPath === ""
            text: qsTr("Choose a backup folder first.")
            color: root.colors.textSecondary
            font.pixelSize: 12
            font.family: AppFontFamily
            wrapMode: Text.WordWrap
            width: parent.width
        }

        Text {
            visible: generalPage.backupPath !== "" && generalPage.backupDevices.length === 0
            text: qsTr("No backups found in this folder.")
            color: root.colors.textSecondary
            font.pixelSize: 12
            font.family: AppFontFamily
            wrapMode: Text.WordWrap
            width: parent.width
        }

        ListView {
            visible: generalPage.backupPath !== "" && generalPage.backupDevices.length > 0
            width: parent.width
            height: Math.min(contentHeight, 320)
            clip: true
            spacing: 8
            model: generalPage.backupDevices

            delegate: Rectangle {
                width: ListView.view.width
                height: backupCardContent.implicitHeight + 18
                radius: 8
                color: "transparent"
                border.width: 1
                border.color: root.colors.cardStroke

                Column {
                    id: backupCardContent
                    anchors { left: parent.left; right: parent.right; top: parent.top; margins: 9 }
                    spacing: 8

                    Rectangle {
                        width: parent.width
                        height: deviceLabel.implicitHeight + 8
                        radius: 5
                        color: deviceMouse.containsMouse ? root.colors.buttonHover : "transparent"

                        Text {
                            id: deviceLabel
                            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; leftMargin: 8; rightMargin: 8 }
                            text: (generalPage.expandedBackupUdid === modelData.udid ? "▾ " : "▸ ") + (modelData.label || modelData.name)
                            color: root.colors.textPrimary
                            font.pixelSize: 13
                            font.family: AppFontFamily
                            font.weight: Font.DemiBold
                            elide: Text.ElideMiddle
                        }

                        MouseArea {
                            id: deviceMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: generalPage.expandedBackupUdid = generalPage.expandedBackupUdid === modelData.udid ? "" : modelData.udid
                        }
                    }

                    Column {
                        visible: generalPage.expandedBackupUdid === modelData.udid
                        width: parent.width
                        spacing: 6

                        Repeater {
                            model: modelData.saves
                            delegate: Rectangle {
                                width: parent.width
                                height: 34
                                radius: 5
                                color: "transparent"

                                Row {
                                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; leftMargin: 8; rightMargin: 6 }
                                    spacing: 8

                                    Item {
                                        id: lockIconSlot
                                        width: 14
                                        height: 14
                                        anchors.verticalCenter: parent.verticalCenter

                                        Image {
                                            id: lockIcon
                                            anchors.fill: parent
                                            source: modelData.encrypted ? "/images/glyphs/lock.svg" : "/images/glyphs/no_lock.svg"
                                            fillMode: Image.PreserveAspectFit
                                            smooth: true
                                            visible: false
                                        }

                                        ColorOverlay {
                                            anchors.fill: lockIcon
                                            source: lockIcon
                                            color: root.colors.textSecondary
                                        }

                                        MouseArea {
                                            id: lockMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                        }

                                        AppTooltip {
                                            tipText: modelData.encrypted ? qsTr("Encrypted backup") : qsTr("Unencrypted backup")
                                            textColor: root.colors.textPrimary
                                            backgroundStroke: root.colors.cardStroke
                                            backgroundFill: root.colors.settingsButtonBg
                                            hovered: lockMouse.containsMouse
                                            verticalMargin: 35
                                        }
                                    }

                                    Text {
                                        width: parent.width - lockIconSlot.width - showButton.width - deleteButton.width - parent.spacing * 3
                                        text: modelData.date + (modelData.size > 0 ? " · " + generalPage.formatBytes(modelData.size) : "")
                                        color: root.colors.textSecondary
                                        font.pixelSize: 12
                                        font.family: AppFontFamily
                                        elide: Text.ElideMiddle
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    AppButton {
                                        id: showButton
                                        colors: root.colors
                                        width: 150
                                        label: qsTr("Show in File Manager")
                                        onClicked: DeviceWatcher.openBackup(modelData.path)
                                    }

                                    AppButton {
                                        id: deleteButton
                                        colors: root.colors
                                        width: 78
                                        label: qsTr("Delete")
                                        destructive: true
                                        onClicked: generalPage.requestDeleteBackup(modelData.path, modelData.date)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            AppButton {
    colors: root.colors
                label: qsTr("Close")
                onClicked: manageBackupsPopup.close()
            }
        }
    }

    ModalPanel {
        colors: root.colors
        id: deleteBackupConfirmPopup
        title: qsTr("Delete Backup?")
        width: Math.min(460, generalPage.width - 40)
        onClosed: {
            generalPage.pendingDeleteBackupPath = ""
            generalPage.pendingDeleteBackupDate = ""
        }

        Text {
            text: generalPage.pendingDeleteBackupDate === ""
                  ? qsTr("This backup will be permanently deleted from disk.")
                  : qsTr("The backup from %1 will be permanently deleted from disk.").arg(generalPage.pendingDeleteBackupDate)
            color: root.colors.textSecondary
            font.pixelSize: 12
            font.family: AppFontFamily
            wrapMode: Text.WordWrap
            width: parent.width
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            AppButton {
                colors: root.colors
                label: qsTr("Cancel")
                onClicked: deleteBackupConfirmPopup.close()
            }
            AppButton {
                colors: root.colors
                label: qsTr("Delete")
                destructive: true
                onClicked: generalPage.confirmDeleteBackup()
            }
        }
    }

    function ensureBackupFolder(action) {
        generalPage.pendingFolderAction = action
        if (generalPage.backupPath === "") {
            chooseBackupFolderPopup.open()
            return
        }
        generalPage.resumePendingFolderAction()
    }

    function resumePendingFolderAction() {
        var action = generalPage.pendingFolderAction
        generalPage.pendingFolderAction = ""

        switch (action) {
        case "backup":
            generalPage.startBackupFromButton()
            break
        case "manage":
            generalPage.openManageBackupsPopup()
            break
        case "restore":
            generalPage.backupPasswordError = qsTr("Restore Backup is not available yet.")
            break
        }
    }

    function unknownEncryptionStatusMessage() {
        return qsTr("LiniTunes could not confirm the current backup encryption state. Unlock the iPhone and reconnect it before changing encryption.")
    }

    function requestStandardBackupMode() {
        generalPage.backupPasswordError = ""
        if (DeviceWatcher.backup_encryption_status === "enabled") {
            disableEncryptionConfirmPopup.open()
            return
        }
        generalPage.backupMode = 0
        generalPage.backupPasswordForRun = ""
    }

    function requestEncryptedBackupMode() {
        generalPage.backupPasswordError = ""
        generalPage.backupMode = 1
        if (DeviceWatcher.backup_encryption_status === "unknown") {
            generalPage.backupPasswordError = generalPage.unknownEncryptionStatusMessage()
            generalPage.backupMode = 0
            return
        }
        if (DeviceWatcher.backup_encryption_status !== "enabled" && generalPage.backupPasswordForRun === "")
            passwordSetupPopup.open()
    }

    function acceptBackupPasswordSetup() {
        generalPage.backupPasswordError = ""
        if (setupPasswordField.text.length === 0) {
            generalPage.backupPasswordError = qsTr("Enter a backup password.")
            return
        }
        if (setupPasswordField.text !== setupPasswordConfirmField.text) {
            generalPage.backupPasswordError = qsTr("Backup passwords do not match.")
            return
        }

        generalPage.backupPasswordForRun = setupPasswordField.text
        passwordSetupPopup.close()
        if (generalPage.pendingBackupAfterPassword) {
            generalPage.pendingBackupAfterPassword = false
            generalPage.startBackupFromButton()
        }
    }

    function startBackupFromButton() {
        generalPage.backupPasswordError = ""
        if (generalPage.backupPath === "") {
            generalPage.ensureBackupFolder("backup")
            return
        }

        var encryptionStatus = DeviceWatcher.backup_encryption_status
        if (generalPage.backupMode === 0) {
            if (encryptionStatus === "enabled") {
                disableEncryptionConfirmPopup.open()
                return
            }
            DeviceWatcher.startBackup(generalPage.backupPath, false, "")
            return
        }

        if (encryptionStatus === "enabled") {
            DeviceWatcher.startBackup(generalPage.backupPath, false, "")
            return
        }

        if (encryptionStatus === "unknown") {
            generalPage.backupPasswordError = generalPage.unknownEncryptionStatusMessage()
            generalPage.backupMode = 0
            return
        }

        if (generalPage.backupPasswordForRun === "") {
            generalPage.pendingBackupAfterPassword = true
            passwordSetupPopup.open()
            return
        }

        DeviceWatcher.startBackup(generalPage.backupPath, true, generalPage.backupPasswordForRun)
    }

    function disableEncryptionWithPassword() {
        generalPage.disablePasswordError = ""
        if (disablePasswordField.text.length === 0) {
            generalPage.disablePasswordError = qsTr("Enter the current backup password.")
            return
        }
        DeviceWatcher.disableBackupEncryption(generalPage.backupPath, disablePasswordField.text)
    }

    function changeBackupPassword() {
        generalPage.changePasswordError = ""
        if (changeOldPasswordField.text.length === 0 || changeNewPasswordField.text.length === 0) {
            generalPage.changePasswordError = qsTr("Enter the current and new passwords.")
            return
        }
        if (changeNewPasswordField.text !== changeConfirmPasswordField.text) {
            generalPage.changePasswordError = qsTr("New passwords do not match.")
            return
        }
        DeviceWatcher.changeBackupPassword(generalPage.backupPath, changeOldPasswordField.text, changeNewPasswordField.text)
    }

    function openManageBackupsPopup() {
        generalPage.refreshBackupList()
        manageBackupsPopup.open()
    }

    function refreshBackupList() {
        generalPage.backupDevices = DeviceWatcher.listBackups(generalPage.backupPath)
        generalPage.expandedBackupUdid = generalPage.preferredBackupUdid()
    }

    function preferredBackupUdid() {
        var currentUdid = DeviceWatcher.udid
        var latestUdid = ""
        var latestModified = -1

        for (var i = 0; i < generalPage.backupDevices.length; ++i) {
            var device = generalPage.backupDevices[i]
            if (device.udid === currentUdid)
                return device.udid

            if (!device.saves || device.saves.length === 0)
                continue

            var modified = Number(device.saves[0].modified || 0)
            if (modified > latestModified) {
                latestModified = modified
                latestUdid = device.udid
            }
        }

        return latestUdid
    }

    function requestDeleteBackup(path, date) {
        generalPage.pendingDeleteBackupPath = path
        generalPage.pendingDeleteBackupDate = date
        deleteBackupConfirmPopup.open()
    }

    function confirmDeleteBackup() {
        var path = generalPage.pendingDeleteBackupPath
        deleteBackupConfirmPopup.close()
        if (path !== "")
            generalPage.deleteBackup(path)
    }

    function deleteBackup(path) {
        if (DeviceWatcher.deleteBackup(generalPage.backupPath, path))
            generalPage.refreshBackupList()
    }

    function softwareVersionLine() {
        var version = DeviceWatcher.product_version || qsTr("Unknown")
        var label = generalPage.platformLabel()
        return label ? label + " " + version : version
    }

    function softwareVersionBadgeText() {
        if (!DeviceWatcher.product_version)
            return ""
        var major = generalPage.productMajorVersion()
        var label = generalPage.platformLabel()
        return label ? label + " " + major : major
    }

    function productMajorVersion() {
        if (!DeviceWatcher.product_version)
            return ""
        return DeviceWatcher.product_version.split(".")[0]
    }

    function platformLabel() {
        var type = DeviceWatcher.product_type || ""
        var major = parseInt(generalPage.productMajorVersion())
        if (type.indexOf("iPad") === 0)
            return major >= 17 ? "iPadOS" : "iOS"
        if (type.indexOf("AppleTV") === 0)
            return "tvOS"
        if (type.indexOf("Mac") === 0)
            return "macOS"
        if (type.indexOf("iPhone") === 0 || type.indexOf("iPod") === 0)
            return "iOS"
        return ""
    }

    function deviceTypeLabel() {
        var type = DeviceWatcher.product_type || ""
        if (type.indexOf("iPad") === 0)
            return "iPad"
        if (type.indexOf("iPod") === 0)
            return "iPod"
        if (type.indexOf("AppleTV") === 0)
            return "Apple TV"
        if (type.indexOf("Mac") === 0)
            return "Mac"
        return "iPhone"
    }

    function nextUpdateCheckDate() {
        var date = new Date()
        date.setDate(date.getDate() + 7)
        return Qt.formatDate(date, Qt.locale().dateFormat(Locale.ShortFormat))
    }

    function backupStatusText() {
        if (!DeviceWatcher.backup_info)
            return ""

        switch (DeviceWatcher.backup_info.status) {
        case "running":
            return generalPage.cancellationRequested ? qsTr("Cancelling...") : qsTr("Backing up…")
        case "completed":
            return qsTr("Backup completed")
        case "completed_with_warnings":
            return qsTr("Completed with warnings: ") + DeviceWatcher.backup_info.warning
        case "failed":
            return qsTr("Failed: ") + generalPage.friendlyBackupError(DeviceWatcher.backup_info.error)
        case "cancelled":
            return qsTr("Backup cancelled")
        default:
            return ""
        }
    }

    function backupStatusColor() {
        if (!DeviceWatcher.backup_info)
            return root.colors.textSecondary

        switch (DeviceWatcher.backup_info.status) {
        case "completed":
            return root.colors.green
        case "completed_with_warnings":
        case "cancelled":
            return root.colors.yellow
        case "failed":
            return root.colors.red
        default:
            return root.colors.textPrimary
        }
    }

    function friendlyBackupError(error) {
        if (!error) return qsTr("Unknown error")
        var lower = error.toLowerCase()
        if (lower.indexOf("insufficient free disk space") >= 0 || lower.indexOf("not enough free disk space") >= 0 || lower.indexOf("mberrordomain/105") >= 0)
            return qsTr("Not enough free disk space to complete this backup. Free up space on the backup drive or choose another backup folder.")
        if (lower.indexOf("lock") >= 0 || lower.indexOf("passcode") >= 0 || lower.indexOf("unavailable") >= 0 || lower.indexOf("connection") >= 0)
            return error + qsTr(" Please unlock the iPhone, keep it connected, and try again.")
        if (lower.indexOf("incomplete") >= 0)
            return error + qsTr(" Keep the iPhone unlocked and retry the backup.")
        return error
    }

    function formatBytes(bytes) {
        if (bytes >= 1073741824) return (bytes / 1073741824).toFixed(1) + " GB"
        if (bytes >= 1048576) return (bytes / 1048576).toFixed(1) + " MB"
        if (bytes >= 1024) return (bytes / 1024).toFixed(1) + " KB"
        return bytes + " B"
    }
}
