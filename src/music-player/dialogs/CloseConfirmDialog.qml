// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import QtQml.Models 2.11
import org.deepin.dtk 1.0

DialogWindow {
    property bool isClose: Presenter.valueFromSettings("base.close.is_close")
    property int closeAction: minimizeBtn.checked ? 0 : 1
    property bool isMinimize: false
    signal minimizeToSystemTray()

    id: closeConfirmDialog
    width: 400
    height: 230
    modality: Qt.ApplicationModal
    icon: globalVariant.appIconName

    header: DialogTitleBar {
        enableInWindowBlendBlur: false
    }

    Column {
        width: parent.width
        height: parent.height
        anchors.centerIn: parent
        spacing: 5
        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            font: DTK.fontManager.t5
            text: qsTr("Please choose your action")
        }
        ButtonGroup {
            buttons: column.children
        }
        Column {
            id: column
            width: parent.width
            spacing: 5
            RadioButton {
                id: minimizeBtn
                width: parent.width
                height: 30
                padding: 5
                text: qsTr("Minimize to system tray")
                font: DTK.fontManager.t6
                checked: true
                onClicked: {
                    closeAction = 0
                }
            }
            RadioButton {
                id: closeBtn
                width: parent.width
                height: 30
                padding: 5
                text: qsTr("Exit")
                font: DTK.fontManager.t6
                onClicked: {
                    closeAction = 1
                }
            }
        }
        CheckBox {
            width: parent.width
            height: 30
            padding: 5
            text: qsTr("Do not ask again")
            font: DTK.fontManager.t6
            onClicked: {
                if (checked) {
                    isClose = true
                } else {
                    isClose = false
                }
            }
        }
        Row {
            width: parent.width
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            Button {
                width: 180
                height: 30
                text: qsTr("Cancel")
                onClicked: {
                    closeConfirmDialog.close()
                }
            }
            Button {
                width: 180
                height: 30
                text: qsTr("Confirm")
                onClicked: {
                    //console.log("onClicked: closeAction:" + closeAction)
                    Presenter.setValueToSettings("base.close.is_close", isClose)
                    if (closeAction == 0) {
                        //console.log("set base.close.close_action:" + closeAction)
                        if (isClose) {
                            Presenter.setValueToSettings("base.close.close_action", closeAction)
                        }
                        isMinimize = true
                        close()
                        minimizeToSystemTray()
                    } else if (closeAction == 1) {
                        //console.log("set base.close.close_action:" + closeAction)
                        if (isClose) {
                            Presenter.setValueToSettings("base.close.close_action", closeAction)
                        }
                        Presenter.syncToSettings();
                        Presenter.forceExit();
                    }
                }
            }
        }
    }
}
