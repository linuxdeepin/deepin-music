// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import QtQml.Models 2.11
import org.deepin.dtk 1.0
import org.deepin.dtk.settings 1.0 as Settings

Settings.SettingsDialog {
    property string shortcutPlayPause: Presenter.valueFromSettings("shortcuts.all.play_pause")
    property string shortcutPrevious: Presenter.valueFromSettings("shortcuts.all.previous")
    property string shortcutNext: Presenter.valueFromSettings("shortcuts.all.next")
    property string shortcutVolumeUp: Presenter.valueFromSettings("shortcuts.all.volume_up")
    property string shortcutVolumeDown: Presenter.valueFromSettings("shortcuts.all.volume_down")
    property string shortcutFavourite: Presenter.valueFromSettings("shortcuts.all.favorite_song")
    property var shortcuts: [shortcutPlayPause, shortcutPrevious, shortcutNext, shortcutVolumeUp, shortcutVolumeDown, shortcutFavourite]

    id: control
    height: 548
    width: 664
    config: SettingsConfig {}
    flags: Qt.WindowCloseButtonHint | Qt.WindowStaysOnTopHint

    groups: [  // 创建配置组，管理配置子组和Options
        Settings.SettingsGroup {
            key: "Basic"
            name: qsTr("Basic")
            children: [
                Settings.SettingsGroup {
                    key: "Play"
                    name: qsTr("Play")
                    background: Settings.ContentBackground {
                        //border.color: "red"
                        color: palette.base
                        //radius: 8
                    }
                    Settings.SettingsOption {
                        key: "autoPlay"
                        name: qsTr("Autoplay")
                        Settings.CheckBox {}
                        onValueChanged: {
                            Presenter.setValueToSettings("base.play.auto_play", value)
                        }
                        Component.onCompleted: {
                            value = Presenter.valueFromSettings("base.play.auto_play")
                        }
                    }
                    Settings.SettingsOption {
                        key: "rememberProgress"
                        name: qsTr("Remember playback position on exit")
                        Settings.CheckBox {}
                        onValueChanged: {
//                            console.log("onValueChanged rememberProgress.........." + value)
                            Presenter.setValueToSettings("base.play.remember_progress", value)
                        }
                        Component.onCompleted: {
//                            console.log("rememberProgress onCompleted.........." + Presenter.valueFromSettings("base.play.remember_progress"))
                            value = Presenter.valueFromSettings("base.play.remember_progress")
                        }
                    }
                    Settings.SettingsOption {
                        key: "fadeInOut"
                        name: qsTr("Enable fade in/out")
                        Settings.CheckBox {}
                        onValueChanged: {
                            Presenter.setValueToSettings("base.play.fade_in_out", value)
                        }
                        Component.onCompleted: {
                            value = Presenter.valueFromSettings("base.play.fade_in_out")
                        }
                    }
                },
                Settings.SettingsGroup {
                    key: "Close Main Window"
                    name: qsTr("Close Main Window")
                    background: Settings.ContentBackground {
                        color: palette.base
                    }
                    Settings.SettingsOption {
                        id: closeAct
                        key: "closeAction"
                        ButtonGroup {
                            buttons: column.children
                        }
                        Column {
                            id: column
                            RadioButton {
                                text: qsTr("Minimize to system tray")
                                font: DTK.fontManager.t8
                                height: 30
                                checked: Settings.SettingsOption.value === 0 || Presenter.valueFromSettings("base.close.close_action") == 0 ? true : false
                                onClicked: Settings.SettingsOption.value = 0
                            }
                            RadioButton {
                                text: qsTr("Exit")
                                font: DTK.fontManager.t8
                                height: 30
                                checked: Settings.SettingsOption.value === 1 || Presenter.valueFromSettings("base.close.close_action") == 1 ? true : false
                                onClicked: Settings.SettingsOption.value = 1
                            }
                            RadioButton {
                                text: qsTr("Ask me always")
                                font: DTK.fontManager.t8
                                height: 30
                                checked: Settings.SettingsOption.value === 2 || Presenter.valueFromSettings("base.close.close_action") == 2 ? true : false
                                onClicked: Settings.SettingsOption.value = 2
                            }
                        }
                        onValueChanged: {
//                            console.log("onValueChanged closeAction.........." + value)
                            Presenter.setValueToSettings("base.close.close_action", value)
                            if (value === 2) {
                                Presenter.setValueToSettings("base.close.is_close", false)
                            } else {
                                Presenter.setValueToSettings("base.close.is_close", true)
                            }
                        }
                        Component.onCompleted: {
//                            console.log("closeAction Component.onCompleted.........." + Presenter.valueFromSettings("base.close.close_action") + value)
                            value = Presenter.valueFromSettings("base.close.close_action")
//                            console.log("Settings.SettingsOption.value:", value)
                            if (value === 2) {
                                Presenter.setValueToSettings("base.close.is_close", false)
                            } else {
                                Presenter.setValueToSettings("base.close.is_close", true)
                            }
                        }
                    }
                }
            ]
        },
        Settings.SettingsGroup {
            key: "Shortcuts"
            name: qsTr("Shortcuts")
            background: Settings.ContentBackground {
                color: palette.base
            }
            Settings.SettingsOption {
                property int idd: 0
                property string oldValue: Presenter.valueFromSettings("shortcuts.all.play_pause")
                property string tmpOldValue: oldValue

                id: shortcut0
                key: "shortcutPlayPause"
                onValueChanged: {
                    if (value === null) {
                        oldValue = tmpOldValue
                    }
                    Presenter.setValueToSettings("shortcuts.all.play_pause", value)
                }
                KeySequenceEdit {
                    id: sequenceEdit
                    width: 430
                    height: 36
                    text: qsTr("Play/Pause")
                    placeholderText: ""
                    keys: string2List(Settings.SettingsOption.value)
                    onKeysChanged: {
                        Settings.SettingsOption.value = list2String(keys)
                        var i;
                        for (i = 0; i < shortcuts.length; i++) {
                            if (i === Settings.SettingsOption.idd)
                                continue
                            if (list2String(keys) === shortcuts[i] && list2String(keys) !== " ") {
                                duplicatDlg.curId = Settings.SettingsOption.idd
                                duplicatDlg.duplicatId = i
                                duplicatDlg.newValue = list2String(keys)
                                duplicatDlg.oldValue = Settings.SettingsOption.oldValue
                                duplicatDlg.show()
                                break
                            }
                        }
                        if (i >= shortcuts.length) {
                            Settings.SettingsOption.tmpOldValue = list2String(keys)
                        }
                    }
                }
                Component.onCompleted: {
                    value = Presenter.valueFromSettings("shortcuts.all.play_pause")
                }
            }
            Settings.SettingsOption {
                property int idd: 1
                property string oldValue: Presenter.valueFromSettings("shortcuts.all.previous")
                property string tmpOldValue: oldValue

                id: shortcut1
                key: "shortcutPrevious"
                onValueChanged: {
                    if (value === "") {
                        oldValue = tmpOldValue
                    }
                    Presenter.setValueToSettings("shortcuts.all.previous", value)
                }
                KeySequenceEdit {
                    width: 430
                    height: 36
                    text: qsTr("Previous")
                    placeholderText: ""
                    keys: string2List(Settings.SettingsOption.value)
                    onKeysChanged: {
                        Settings.SettingsOption.value = list2String(keys)
                        var i;
                        for (i = 0; i < shortcuts.length; i++) {
                            if (i === Settings.SettingsOption.idd)
                                continue
                            if (list2String(keys) === shortcuts[i] && list2String(keys) !== " ") {
                                duplicatDlg.curId = Settings.SettingsOption.idd
                                duplicatDlg.duplicatId = i
                                duplicatDlg.newValue = list2String(keys)
                                duplicatDlg.oldValue = Settings.SettingsOption.oldValue
                                duplicatDlg.show()
                                break
                            }
                        }
                        if (i >= shortcuts.length) {
                            Settings.SettingsOption.tmpOldValue = list2String(keys)
                        }
                    }
                }

                Component.onCompleted: {
                    value = Presenter.valueFromSettings("shortcuts.all.previous")
                }
            }
            Settings.SettingsOption {
                property int idd: 2
                property string oldValue: Presenter.valueFromSettings("shortcuts.all.next")
                property string tmpOldValue: oldValue

                id: shortcut2
                key: "shortcutNext"
                onValueChanged: {
                    if (value === "") {
                        oldValue = tmpOldValue
                    }
                    Presenter.setValueToSettings("shortcuts.all.next", value)
                }
                KeySequenceEdit {
                    width: 430
                    height: 36
                    text: qsTr("Next")
                    placeholderText: ""
                    keys: string2List(Settings.SettingsOption.value)
                    onKeysChanged: {
                        Settings.SettingsOption.value = list2String(keys)
                        var i;
                        for (i = 0; i < shortcuts.length; i++) {
                            if (i === Settings.SettingsOption.idd)
                                continue
                            if (list2String(keys) === shortcuts[i] && list2String(keys) !== " ") {
                                duplicatDlg.curId = Settings.SettingsOption.idd
                                duplicatDlg.duplicatId = i
                                duplicatDlg.newValue = list2String(keys)
                                duplicatDlg.oldValue = Settings.SettingsOption.oldValue
                                duplicatDlg.show()
                                break
                            }
                        }
                        if (i >= shortcuts.length) {
                            Settings.SettingsOption.tmpOldValue = list2String(keys)
                        }
                    }

                }
                Component.onCompleted: {
                    value = Presenter.valueFromSettings("shortcuts.all.next")
                }

            }
            Settings.SettingsOption {
                property int idd: 3
                property string oldValue: Presenter.valueFromSettings("shortcuts.all.volume_up")
                property string tmpOldValue: oldValue

                id: shortcut3
                key: "shortcutVolumeUp"
                onValueChanged: {
                    if (value === "") {
                        oldValue = tmpOldValue
                    }
                    Presenter.setValueToSettings("shortcuts.all.volume_up", value)
                }
                KeySequenceEdit {
                    width: 430
                    height: 36
                    text: qsTr("Volume up")
                    placeholderText: ""
                    keys: string2List(Settings.SettingsOption.value)
                    onKeysChanged: {
                        Settings.SettingsOption.value = list2String(keys)
                        var i;
                        for (i = 0; i < shortcuts.length; i++) {
                            if (i === Settings.SettingsOption.idd)
                                continue
                            if (list2String(keys) === shortcuts[i] && list2String(keys) !== " ") {
                                duplicatDlg.curId = Settings.SettingsOption.idd
                                duplicatDlg.duplicatId = i
                                duplicatDlg.newValue = list2String(keys)
                                duplicatDlg.oldValue = Settings.SettingsOption.oldValue
                                duplicatDlg.show()
                                break
                            }
                        }
                        if (i >= shortcuts.length) {
                            Settings.SettingsOption.tmpOldValue = list2String(keys)
                        }
                    }

                }

                Component.onCompleted: {
                    value = Presenter.valueFromSettings("shortcuts.all.volume_up")
                }
            }
            Settings.SettingsOption {
                property int idd: 4
                property string oldValue: Presenter.valueFromSettings("shortcuts.all.volume_down")
                property string tmpOldValue: oldValue

                id: shortcut4
                key: "shortcutVolumeDown"
                onValueChanged: {
                    if (value === "") {
                        oldValue = tmpOldValue
                    }
                    Presenter.setValueToSettings("shortcuts.all.volume_down", value)
                }
                KeySequenceEdit {
                    width: 430
                    height: 36
                    text: qsTr("Volume down")
                    placeholderText: ""
                    keys: string2List(Settings.SettingsOption.value)
                    onKeysChanged: {
                        Settings.SettingsOption.value = list2String(keys)
                        var i;
                        for (i = 0; i < shortcuts.length; i++) {
                            if (i === Settings.SettingsOption.idd)
                                continue
                            if (list2String(keys) === shortcuts[i] && list2String(keys) !== " ") {
                                duplicatDlg.curId = Settings.SettingsOption.idd
                                duplicatDlg.duplicatId = i
                                duplicatDlg.newValue = list2String(keys)
                                duplicatDlg.oldValue = Settings.SettingsOption.oldValue
                                duplicatDlg.show()
                                break
                            }
                        }
                        if (i >= shortcuts.length) {
                            Settings.SettingsOption.tmpOldValue = list2String(keys)
                        }
                    }
                }

                Component.onCompleted: {
                    value = Presenter.valueFromSettings("shortcuts.all.volume_down")
                }
            }
            Settings.SettingsOption {
                property int idd: 5
                property string oldValue: Presenter.valueFromSettings("shortcuts.all.favorite_song")
                property string tmpOldValue: oldValue

                id: shortcut5
                key: "shortcutFavoriteSong"
                onValueChanged: {
                    if (value === "") {
                        oldValue = tmpOldValue
                    }
                    Presenter.setValueToSettings("shortcuts.all.favorite_song", value)
                }
                KeySequenceEdit {
                    width: 430
                    height: 36
                    text: qsTr("Favorite")
                    placeholderText: ""
                    keys: string2List(Settings.SettingsOption.value)
                    onKeysChanged: {
                        Settings.SettingsOption.value = list2String(keys)
                        var i;
                        for (i = 0; i < shortcuts.length; i++) {
                            if (i === Settings.SettingsOption.idd)
                                continue
                            if (list2String(keys) === shortcuts[i] && list2String(keys) !== " ") {
                                duplicatDlg.curId = Settings.SettingsOption.idd
                                duplicatDlg.duplicatId = i
                                duplicatDlg.newValue = list2String(keys)
                                duplicatDlg.oldValue = Settings.SettingsOption.oldValue
                                duplicatDlg.show()
                                break
                            }
                        }
                        if (i >= shortcuts.length) {
                            Settings.SettingsOption.tmpOldValue = list2String(keys)
                        }
                    }
                }
                Component.onCompleted: {
                    value = Presenter.valueFromSettings("shortcuts.all.favorite_song")
                }
            }
        }
    ]

    function string2List(str) {
        if (str === undefined)
            return []

        var list = str.split("+")
        //console.log("Settings dialog, string2List:" + list)
        return list
    }
    function list2String(list) {
        var str = list.join("+")
        //console.log("Settings dialog, list2String:" + str)
        return str
    }
    function updateInfos(){
        shortcutPlayPause = Presenter.valueFromSettings("shortcuts.all.play_pause")
        shortcutPrevious = Presenter.valueFromSettings("shortcuts.all.previous")
        shortcutNext = Presenter.valueFromSettings("shortcuts.all.next")
        shortcutVolumeUp = Presenter.valueFromSettings("shortcuts.all.volume_up")
        shortcutVolumeDown = Presenter.valueFromSettings("shortcuts.all.volume_down")
        shortcutFavourite = Presenter.valueFromSettings("shortcuts.all.favorite_song")
    }
    function settingsValueChanged(key, value) {
        if (key === "base.close.close_action") {
            closeAct.value = value
        }
        updateInfos()
    }

    property var duplicatDlg :DialogWindow {
        property int curId: 0
        property int duplicatId: 0
        property string newValue: ""
        property string oldValue: ""
        property bool replaceFlag: false

        id: dialog
        minimumWidth: 380
        modality: Qt.ApplicationModal
//        color: Qt.rgba(247,247,247,0.80);
        icon: globalVariant.appIconName

        onClosing: {
            if (!replaceFlag) {
                switch(curId) {
                case 0:
                    shortcut0.value = oldValue
                    break
                case 1:
                    shortcut1.value = oldValue
                    break
                case 2:
                    shortcut2.value = oldValue
                    break
                case 3:
                    shortcut3.value = oldValue
                    break
                case 4:
                    shortcut4.value = oldValue
                    break
                case 5:
                    shortcut5.value = oldValue
                    break
                default:
                    break
                }
            } else {
                replaceFlag = false
            }
            cancelBtn.focus = false
            replaceBtn.focus = false
        }

        ColumnLayout {
            width: parent.width
            Label {
                id:deleteSongsLabel
                Layout.preferredWidth: parent.width
                Layout.alignment: Qt.AlignHCenter
                font: DTK.fontManager.t6
                wrapMode: Text.WordWrap
                horizontalAlignment: Qt.AlignHCenter
                text: qsTr("This shortcut key conflicts with [%1], click Replace to make this shortcut key take effect immediately").arg(dialog.newValue)
            }

            RowLayout {
                Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                Layout.bottomMargin: 10
                Layout.topMargin: 10
                Layout.fillWidth: true
                Button {
                    id: cancelBtn
                    Layout.preferredWidth: 175
                    text: qsTr("Cancel")
                    onClicked: {
                        //console.log("Cancel  clicked curId:", dialog.curId, "  duplicatId:", dialog.duplicatId, " newValue:", dialog.newValue, "  oldValue:", dialog.oldValue)

                        switch(dialog.curId) {
                        case 0:
                            shortcut0.value = dialog.oldValue
                            break
                        case 1:
                            shortcut1.value = dialog.oldValue
                            break
                        case 2:
                            shortcut2.value = dialog.oldValue
                            break
                        case 3:
                            shortcut3.value = dialog.oldValue
                            break
                        case 4:
                            shortcut4.value = dialog.oldValue
                            break
                        case 5:
                            shortcut5.value = dialog.oldValue
                            break
                        default:
                            break
                        }

                        dialog.close()
                    }
                }
                Button {
                    id: replaceBtn
                    Layout.preferredWidth: 175
                    text: qsTr("Replace")
                    onClicked: {
                        dialog.replaceFlag = true
                        switch(dialog.duplicatId) {
                        case 0:
                            shortcut0.value = " "
                            break
                        case 1:
                            shortcut1.value = " "
                            break
                        case 2:
                            shortcut2.value = " "
                            break
                        case 3:
                            shortcut3.value = " "
                            break
                        case 4:
                            shortcut4.value = " "
                            break
                        case 5:
                            shortcut5.value = " "
                            break
                        default:
                            break

                        }
                        dialog.close();
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        Presenter.valueChangedFromSettings.connect(settingsValueChanged)
    }
}
