import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Item {
    id: generalPage

    property string backupPath: ""
    property bool cancellationRequested: false
    property int backupMode: DeviceWatcher.backup_encrypted ? 1 : 0 // 0 = important data, 1 = all data/encrypted
    property string backupPasswordForRun: ""
    property bool pendingBackupAfterPassword: false
    property string backupPasswordError: ""
    property string disablePasswordError: ""
    property string changePasswordError: ""
    property var backupDevices: []
    property string pendingFolderAction: ""

    component Card: Rectangle {
        id: cardRoot
        property string title: ""
        default property alias content: cardContent.data

        width: parent ? parent.width : 0
        implicitHeight: cardTitle.implicitHeight + cardContent.implicitHeight + 30
        radius: 8
        border.width: 0
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0; color: root.colors.cardStroke }
            GradientStop { position: 1.5; color: "#02000000" }
        }

        Rectangle {
            id: cardInner
            anchors.fill: parent
            anchors.margins: 1
            radius: 8
            border.width: 0
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop { position: 1.2; color: root.colors.cardBackgroundTop }
                GradientStop { position: 0; color: root.colors.cardBackgroundBottom }
            }
        }

        Text {
            id: cardTitle
            opacity: 0.5
            color: root.colors.textPrimary
            text: cardRoot.title
            font.pixelSize: 12
            font.family: AppFontFamily
            anchors {
                left: cardInner.left
                top: cardInner.top
                leftMargin: 10
                topMargin: 8
            }
        }

        Column {
            id: cardContent
            anchors {
                left: cardInner.left
                right: cardInner.right
                top: cardTitle.bottom
                margins: 14
                topMargin: 8
            }
            spacing: 10
        }
    }

    component SyncStyleButton: Rectangle {
        id: buttonRoot
        property string label: "Button"
        property bool destructive: false
        property bool primary: false
        signal clicked()

        width: Math.max(118, buttonText.implicitWidth + 28)
        height: 32
        radius: 5
        opacity: enabled ? 1.0 : 0.45
        border.width: destructive || primary ? 0 : 1
        border.color: root.colors.cardStroke
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 1; color: buttonRoot.topColor() }
            GradientStop { position: 0; color: buttonRoot.bottomColor() }
        }

        function topColor() {
            if (destructive)
                return root.colors.darkRed
            if (primary)
                return root.colors.accent
            return root.colors.cardBackgroundTop
        }

        function bottomColor() {
            if (destructive)
                return root.colors.red
            if (primary)
                return root.colors.accent
            return root.colors.cardBackgroundBottom
        }

        function labelColor() {
            if (destructive || primary)
                return "#ffffff"
            return root.colors.textPrimary
        }

        Text {
            id: buttonText
            anchors.centerIn: parent
            text: buttonRoot.label
            color: buttonRoot.labelColor()
            font.weight: Font.DemiBold
            font.family: AppFontFamily
            font.pixelSize: 13
        }

        MouseArea {
            anchors.fill: parent
            enabled: buttonRoot.enabled
            onPressed: buttonRoot.opacity = 0.7
            onCanceled: buttonRoot.opacity = buttonRoot.enabled ? 1.0 : 0.45
            onReleased: {
                buttonRoot.opacity = buttonRoot.enabled ? 1.0 : 0.45
                buttonRoot.clicked()
            }
        }
    }

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

        Rectangle {
            id: radioOuter
            width: 16
            height: 16
            radius: 8
            color: "transparent"
            border.width: 1
            border.color: optionRoot.checked ? root.colors.accent : root.colors.textSecondary
            anchors {
                left: parent.left
                top: parent.top
                leftMargin: 0
                topMargin: 2
            }

            Rectangle {
                width: 8
                height: 8
                radius: 4
                visible: optionRoot.checked
                color: root.colors.accent
                anchors.centerIn: parent
            }
        }

        Column {
            id: optionTextColumn
            anchors {
                left: radioOuter.right
                right: parent.right
                top: parent.top
                leftMargin: 10
            }
            spacing: 4

            Text {
                text: optionRoot.title
                color: root.colors.textPrimary
                font.pixelSize: 13
                font.family: AppFontFamily
                font.weight: Font.DemiBold
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Text {
                text: optionRoot.details
                color: root.colors.textSecondary
                font.pixelSize: 12
                font.family: AppFontFamily
                wrapMode: Text.WordWrap
                width: parent.width
            }
        }

        MouseArea {
            anchors {
                left: radioOuter.left
                right: optionTextColumn.right
                top: radioOuter.top
                bottom: optionTextColumn.bottom
            }
            onClicked: optionRoot.selected()
        }
    }

    component ModalPanel: Popup {
        id: popupRoot
        property string title: ""
        default property alias content: popupContent.data

        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        width: Math.min(520, generalPage.width - 40)
        x: Math.round((generalPage.width - width) / 2)
        y: Math.max(24, Math.round((generalPage.height - height) / 2))
        padding: 0
        scale: 1.0
        opacity: 1.0
        Overlay.modal: Rectangle { color: "#88000000" }
        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 160; easing.type: Easing.OutCubic }
            NumberAnimation { property: "scale"; from: 0.96; to: 1.0; duration: 160; easing.type: Easing.OutCubic }
        }
        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 120; easing.type: Easing.InCubic }
            NumberAnimation { property: "scale"; from: 1.0; to: 0.97; duration: 120; easing.type: Easing.InCubic }
        }
        background: Rectangle {
            radius: 10
            border.width: 0
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop { position: 0; color: root.colors.cardStroke }
                GradientStop { position: 1; color: "#02000000" }
            }
            Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                radius: 10
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop { position: 0; color: root.colors.cardBackgroundTop }
                    GradientStop { position: 1; color: root.colors.cardBackgroundBottom }
                }
            }
        }

        contentItem: Column {
            width: popupRoot.width
            spacing: 14
            padding: 18

            Text {
                text: popupRoot.title
                color: root.colors.textPrimary
                font.pixelSize: 16
                font.family: AppFontFamily
                font.weight: Font.Bold
                wrapMode: Text.WordWrap
                width: parent.width - parent.leftPadding - parent.rightPadding
            }

            Rectangle {
                width: parent.width - parent.leftPadding - parent.rightPadding
                height: 1
                color: root.colors.divider
            }

            Column {
                id: popupContent
                width: parent.width - parent.leftPadding - parent.rightPadding
                spacing: 12
            }
        }
    }

    component PasswordField: TextField {
        height: 34
        echoMode: TextInput.Password
        color: root.colors.textPrimary
        placeholderTextColor: root.colors.sideTextInactive
        font.pixelSize: 12
        font.family: AppFontFamily
        background: Rectangle {
            radius: 6
            color: root.colors.cardBackgroundTop
            border.color: root.colors.cardStroke
            border.width: 1
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
                                      ? qsTr("iOS ") + (DeviceWatcher.product_version || qsTr("Unknown"))
                                      : qsTr("No device connected")
                                color: root.colors.textPrimary
                                font.pixelSize: 18
                                font.family: AppFontFamily
                                font.weight: Font.Bold
                            }

                            Text {
                                text: DeviceWatcher.device_connected
                                      ? qsTr("Your iPhone is up to date. LiniTunes will automatically check for updates again on %1.").arg(generalPage.nextUpdateCheckDate())
                                      : qsTr("Connect an iPhone to check for software updates.")
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

                            SyncStyleButton {
                                width: parent.width
                                label: qsTr("Check for Updates")
                                enabled: DeviceWatcher.device_connected
                                onClicked: console.log("Check for Updates not implemented yet")
                            }
                            SyncStyleButton {
                                width: parent.width
                                label: qsTr("Restore...")
                                enabled: DeviceWatcher.device_connected
                                onClicked: console.log("Software Restore not implemented yet")
                            }
                        }
                    }
                }

                Card {
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
                            details: DeviceWatcher.backup_encrypted
                                     ? qsTr("Uses the current local unencrypted backup behavior.")
                                     : qsTr("Uses a local unencrypted backup behavior.")
                            checked: generalPage.backupMode === 0
                            onSelected: generalPage.requestImportantBackupMode()
                        }

                        BackupOption {
                            title: qsTr("Back up all of the data of this iPhone to this computer (Encrypted)")
                            details: DeviceWatcher.backup_encrypted
                                     ? qsTr("Encrypted local backup is enabled for this device.")
                                     : qsTr("Encrypt local backup. A password is required and cannot be recovered if forgotten.")
                            checked: generalPage.backupMode === 1
                            onSelected: generalPage.requestEncryptedBackupMode()
                        }
                    }

                    Text {
                        visible: DeviceWatcher.backup_running
                        text: DeviceWatcher.backup_encrypted || generalPage.backupMode === 1
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

                    Row {
                        spacing: 10
                        SyncStyleButton {
                            label: qsTr("Manage Backups")
                            onClicked: generalPage.ensureBackupFolder("manage")
                        }
                        SyncStyleButton {
                            label: DeviceWatcher.backup_running ? qsTr("Cancel") : qsTr("Back Up Now")
                            primary: !DeviceWatcher.backup_running
                            destructive: DeviceWatcher.backup_running
                            enabled: DeviceWatcher.device_connected && (generalPage.backupPath !== "" || DeviceWatcher.backup_running)
                            onClicked: {
                                if (DeviceWatcher.backup_running) {
                                    generalPage.cancellationRequested = true
                                    DeviceWatcher.stopBackup()
                                } else {
                                    generalPage.ensureBackupFolder("backup")
                                }
                            }
                        }
                        SyncStyleButton {
                            label: qsTr("Restore Backup")
                            enabled: DeviceWatcher.device_connected
                            onClicked: generalPage.ensureBackupFolder("restore")
                        }
                        SyncStyleButton {
                            label: qsTr("Change Password")
                            enabled: DeviceWatcher.device_connected && DeviceWatcher.backup_encrypted
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
            if (DeviceWatcher.backup_encrypted)
                generalPage.backupMode = 1
            if (!DeviceWatcher.backup_running) {
                generalPage.cancellationRequested = false
                generalPage.backupPasswordForRun = ""
                generalPage.pendingBackupAfterPassword = false
            }
            if (!DeviceWatcher.backup_encrypted && generalPage.backupMode === 1 && !generalPage.pendingBackupAfterPassword)
                generalPage.backupMode = 0
        }

        function onCurrentDeviceChanged() {
            generalPage.backupMode = DeviceWatcher.backup_encrypted ? 1 : 0
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
            generalPage.backupPath = selectedFolder.toString().replace("file://", "")
            generalPage.resumePendingFolderAction()
        }
    }

    ModalPanel {
        id: chooseBackupFolderPopup
        title: qsTr("Choose Backup Folder")

        Text {
            text: qsTr("Choose where LiniTunes should store and look for local backups. This is remembered for this session and can be changed later in Settings.")
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
            SyncStyleButton {
                label: qsTr("Cancel")
                onClicked: {
                    generalPage.pendingFolderAction = ""
                    chooseBackupFolderPopup.close()
                }
            }
            SyncStyleButton {
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
            id: setupPasswordField
            width: parent.width
            placeholderText: qsTr("Backup password")
        }

        PasswordField {
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
            SyncStyleButton {
                label: qsTr("Cancel")
                onClicked: {
                    generalPage.pendingBackupAfterPassword = false
                    passwordSetupPopup.close()
                }
            }
            SyncStyleButton {
                label: qsTr("Use Password")
                primary: true
                onClicked: generalPage.acceptBackupPasswordSetup()
            }
        }
    }

    ModalPanel {
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
            SyncStyleButton {
                label: qsTr("Go Back")
                onClicked: disableEncryptionConfirmPopup.close()
            }
            SyncStyleButton {
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
            SyncStyleButton {
                label: qsTr("Cancel")
                onClicked: disablePasswordPopup.close()
            }
            SyncStyleButton {
                label: qsTr("Disable")
                destructive: true
                onClicked: generalPage.disableEncryptionWithPassword()
            }
        }
    }

    ModalPanel {
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
            id: changeOldPasswordField
            width: parent.width
            placeholderText: qsTr("Current password")
        }
        PasswordField {
            id: changeNewPasswordField
            width: parent.width
            placeholderText: qsTr("New password")
        }
        PasswordField {
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
            SyncStyleButton {
                label: qsTr("Cancel")
                onClicked: changePasswordPopup.close()
            }
            SyncStyleButton {
                label: qsTr("Change Password")
                primary: true
                onClicked: generalPage.changeBackupPassword()
            }
        }
    }

    ModalPanel {
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

        Repeater {
            model: generalPage.backupDevices
            delegate: Column {
                width: parent.width
                spacing: 6

                Text {
                    text: modelData.name + " — " + modelData.udid
                    color: root.colors.textPrimary
                    font.pixelSize: 13
                    font.family: AppFontFamily
                    font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                    width: parent.width
                }

                Repeater {
                    model: modelData.saves
                    delegate: Text {
                        text: "  • " + modelData.name + (modelData.size > 0 ? " (" + generalPage.formatBytes(modelData.size) + ")" : "")
                        color: root.colors.textSecondary
                        font.pixelSize: 12
                        font.family: AppFontFamily
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }
                }
            }
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            SyncStyleButton {
                label: qsTr("Close")
                onClicked: manageBackupsPopup.close()
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
            console.log("Restore Backup not implemented yet")
            break
        }
    }

    function requestImportantBackupMode() {
        generalPage.backupPasswordError = ""
        if (DeviceWatcher.backup_encrypted) {
            disableEncryptionConfirmPopup.open()
            return
        }
        generalPage.backupMode = 0
        generalPage.backupPasswordForRun = ""
    }

    function requestEncryptedBackupMode() {
        generalPage.backupPasswordError = ""
        generalPage.backupMode = 1
        if (!DeviceWatcher.backup_encrypted && generalPage.backupPasswordForRun === "")
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

        if (generalPage.backupMode === 0) {
            if (DeviceWatcher.backup_encrypted) {
                disableEncryptionConfirmPopup.open()
                return
            }
            DeviceWatcher.startBackup(generalPage.backupPath, false, "")
            return
        }

        if (DeviceWatcher.backup_encrypted) {
            DeviceWatcher.startBackup(generalPage.backupPath, false, "")
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
        disablePasswordPopup.close()
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
        changePasswordPopup.close()
    }

    function openManageBackupsPopup() {
        generalPage.backupDevices = DeviceWatcher.listBackups(generalPage.backupPath)
        manageBackupsPopup.open()
    }

    function softwareVersionBadgeText() {
        if (!DeviceWatcher.product_version)
            return ""
        var major = DeviceWatcher.product_version.split(".")[0]
        if (DeviceWatcher.product_type && DeviceWatcher.product_type.indexOf("iPad") === 0)
            return "iPadOS " + major
        if (DeviceWatcher.product_type && (DeviceWatcher.product_type.indexOf("iPhone") === 0 || DeviceWatcher.product_type.indexOf("iPod") === 0))
            return "iOS " + major
        return major
    }

    function nextUpdateCheckDate() {
        var date = new Date()
        date.setDate(date.getDate() + 7)
        return Qt.formatDate(date, Qt.locale().dateFormat(Locale.ShortFormat))
    }

    function backupStatusText() {
        if (!DeviceWatcher.backup_info)
            return ""

        var status = DeviceWatcher.backup_info.status
        if (status === "running") {
            if (generalPage.cancellationRequested)
                return qsTr("Cancelling...")
            return qsTr("Backing up…")
        }
        if (status === "completed")
            return qsTr("Backup completed")
        if (status === "completed_with_warnings")
            return qsTr("Completed with warnings: ") + DeviceWatcher.backup_info.warning
        if (status === "failed")
            return qsTr("Failed: ") + generalPage.friendlyBackupError(DeviceWatcher.backup_info.error)
        if (status === "cancelled")
            return qsTr("Backup cancelled")
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
