// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import org.deepin.dtk 1.0


DialogWindow {
    id: dialog
    property var deleteHashList: []
    property string listHash: ""
    property string musicTitle: ""
    minimumWidth: 400
    modality: Qt.ApplicationModal
//    color: Qt.rgba(247,247,247,0.80);
    icon: globalVariant.appIconName
    header: DialogTitleBar {
        enableInWindowBlendBlur: false
    }
    ColumnLayout {
        width: parent.width
        Label {
            id:deleteSongsLabel
            Layout.preferredWidth: parent.width
            Layout.alignment: Qt.AlignHCenter
            font: DTK.fontManager.t5
            wrapMode: Text.WordWrap
            horizontalAlignment: Qt.AlignHCenter
            text: {
                if(deleteHashList.length <= 1){
                    return qsTr("Are you sure you want to delete %1?").arg(musicTitle);
                }else{
                    return qsTr("Are you sure you want to delete the selected %1 songs?").arg(deleteHashList.length);
                }
            }
        }


        Label {
            Layout.alignment: Qt.AlignHCenter
            font: DTK.fontManager.t7
            text: qsTr("The song files contained will also be deleted")
        }
        RowLayout {
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            Layout.bottomMargin: 10
            Layout.topMargin: 10
            Layout.fillWidth: true
            WarningButton {
                text: qsTr("Delete")
                Layout.preferredWidth: 185
                Layout.alignment: Qt.AlignRight
                onClicked: {
                    if (listHash === "musicResult")
                        listHash = "all"
                    Presenter.removeFromPlayList(deleteHashList, listHash, true);
                    globalVariant.clearSelectGroup(); //清除选中
                    dialog.close();
                }
            }
            Button {
                text: qsTr("Cancel")
                Layout.preferredWidth: 185
                onClicked: {
                    dialog.close();
                }
            }
            Item {Layout.fillWidth: true}
        }
    }
}

