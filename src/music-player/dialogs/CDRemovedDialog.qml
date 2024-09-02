// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import org.deepin.dtk 1.0


DialogWindow {
    id: dialog
    minimumWidth: 400
    height: 140
    modality: Qt.ApplicationModal
    color: Qt.rgba(247,247,247,0.80);
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
            text: qsTr("The CD has been removed")
        }

        Button {
            text: qsTr("Cancel")
            Layout.preferredWidth: parent.width - 20
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                dialog.close();
            }
        }
    }
}

