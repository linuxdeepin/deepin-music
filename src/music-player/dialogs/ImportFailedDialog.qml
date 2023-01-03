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
    maximumWidth: 400
    maximumHeight: 160
    minimumWidth: 400
    minimumHeight: 160
    modality: Qt.ApplicationModal
    icon: globalVariant.appIconName

    ColumnLayout {
        anchors.fill: parent
        anchors.centerIn: parent
        spacing: 40

        Label {
            id:deleteSongsLabel
            Layout.alignment: Qt.AlignHCenter
            font: DTK.fontManager.t6
            elide: Text.ElideMiddle
            text: qsTr("Import failed, no valid music file found")
        }

        Button {
            text: qsTr("OK")
            Layout.preferredWidth: parent.width
            Layout.alignment: Qt.AlignHCenter

            onClicked: {
                dialog.close();
            }
        }
    }
}

