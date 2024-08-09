// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
import QtQml.Models 2.11
import org.deepin.dtk 1.0
import audio.global 1.0

Rectangle{
    property string title
    property string pageHash
    property string musicinfo
    property bool gridAndlistViewModel: false
    property bool isDefault: true
    property int m_viewType: 0 // 0为icon、1为list
    property int sortType: DmGlobal.SortByNull
    signal viewChanged(var type)

    anchors.horizontalCenter: parent.horizontalCenter
    color: "transparent"
    Row {
        anchors.verticalCenter: parent.verticalCenter
        leftPadding: 10
        spacing: 10
//        DciIcon {
//            id: playall
//            name: "headline_play_bottom"
//            sourceSize: Qt.size(28, 28)
//            ActionButton {
//                id: palyBtn
//                anchors.fill: playall
//                icon.name: "list_play";
//                width: 20; height: 20
//                hoverEnabled: true
//                ColorSelector.hovered: false
//                ToolTip {
//                    visible: palyBtn.hovered
//                    text: qsTr("Play All")
//                }
//                onClicked: {
//                    Presenter.playPlaylist(pageHash);
//                }
//            }
//        }
        FloatingButton {
            id: palyBtn
            width: 28; height: 28
            icon.name: "headline_play_all"
            icon.width: 28
            icon.height: 28
            ToolTip {
                visible: palyBtn.hovered
                text: qsTr("Play All")
            }
            onClicked: {
                globalVariant.clickPlayAllBtn();
                Presenter.playPlaylist(pageHash);
            }
        }
        Label {
            id: buttonLable
            text: title
            textFormat: Text.PlainText
            font: DTK.fontManager.t4
        }
        Label {
            topPadding: 4
            font: DTK.fontManager.t8
            text: musicinfo
            anchors.verticalCenter: buttonLable.verticalCenter
            color: "#7C7C7C"
        }
    }
    SortMenu{
        id: dataSort
        sortPageHash: pageHash
        pageSortType: sortType
        pageTitle: title
    }
    ButtonGroup{ buttons: row.children }
    Row {
        id: row
        anchors.right: dataSort.left; anchors.rightMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        ToolButton {
            id: gridViewButton
            width: 36; height: 36
            icon.name: "view_Grid"
            visible: gridAndlistViewModel
            checkable: true;
            onClicked: {
                m_viewType = 0
                console.log("gridViewButton is clicked");
            }
        }
        ToolButton {
            id: listViewButton
            width: 36; height: 36
            icon.name: "view_list"
            visible: gridAndlistViewModel
            checkable: true;
            onClicked: {
                m_viewType = 1;
                //console.log("listViewButton is clicked");
            }
        }
    }
    onM_viewTypeChanged: {
        viewChanged(m_viewType);
    }
}
