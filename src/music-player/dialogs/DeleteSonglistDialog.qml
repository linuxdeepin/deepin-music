// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import Qt5Compat.GraphicalEffects
import org.deepin.dtk 1.0


DialogWindow {
    id: dialog
    property var deleteHashList: []
    property string listHash: ""
    property string musicTitle: ""
    property bool removeMusic: true
    width: 400
    height: deleteSongsLabel.height + 110
    modality: Qt.ApplicationModal
    icon: globalVariant.appIconName

    header: DialogTitleBar {
        enableInWindowBlendBlur: false
    }

    Column {
        width: parent.width
        height: parent.height
        spacing: 10
        bottomPadding: 10
        Label {
            id:deleteSongsLabel
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            font: DTK.fontManager.t5
            wrapMode: Text.WordWrap
            horizontalAlignment: Qt.AlignHCenter
            text: {
                if(deleteHashList.length <= 1 && removeMusic){
                    return qsTr("Are you sure you want to remove %1?").arg(dialog.musicTitle);
                }else if(deleteHashList.length > 1 && removeMusic){
                    return qsTr("Are you sure you want to remove the selected %1 songs?").arg(deleteHashList.length);
                }else {
                    return qsTr("Are you sure you want to delete this playlist?");
                }
            }
        }

        Row {
            width: parent.width
            spacing: 10
            WarningButton {
                width: 185; height: 36
                text: removeMusic ? qsTr("Remove") : qsTr("Delete")
                onClicked: {
                    if(removeMusic){
                        if (listHash === "musicResult")
                            listHash = "all"
                        Presenter.removeFromPlayList(deleteHashList, listHash);
                        globalVariant.clearSelectGroup(); //清除选中
                    }else{
                        Presenter.deletePlaylist(listHash);
                    }
                    dialog.close();
                }
            }
            Button {
                width: 185; height: 36
                text: qsTr("Cancel")
                onClicked: {
                    dialog.close();
                }
            }
        }
    }
}

