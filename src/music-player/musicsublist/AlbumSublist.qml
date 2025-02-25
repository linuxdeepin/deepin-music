// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0
import "../musicmousemenu"
Rectangle {
    property double scalingRatio: 168 / 810   //计算宽度占比
    property var m_albumData //AlbumInfo 对象
    property ListModel mediaListModels: MusicSublistModel{ meidaDataMap: m_albumData.musicinfos} //从ArtistInfo中解析musicinfos
    property Menu importMenu: ImportMenu{}
    property Menu musicMoreMenu: MusicMoreMenu{pageHash: "album"}
    id: rootrectangle
    objectName: "albumSublist"
    MusicSublistTitle {
        id: musicSublistTitle
        anchors.left: rootrectangle.left; anchors.top: rootrectangle.top
        titleWidth: rootrectangle.width; titleHeight: 244
        artistData: rootrectangle.m_albumData  //AlbumInfo 对象
    }
    Component {
        id: headerV
        Row {
            id: headerView
            width: musicSublistTitle.width - 40; height: 36
            Label {
                width: 56; height: 36
            }
            Label {
                width: headerView.width - 398; height: 36
                text: qsTr("Title")
                verticalAlignment: Qt.AlignVCenter
            }
            Label {
                width: 200; height: 36
                text: qsTr("Artist")
                verticalAlignment: Qt.AlignVCenter
            }
            Label {
                width: 142; height: 36
                text: qsTr("Duration")
                verticalAlignment: Qt.AlignVCenter
            }
        }
    }
    ListView{
        id: listview
        width: musicSublistTitle.width - 40; height: rootrectangle.height - musicSublistTitle.height
        anchors.left: musicSublistTitle.left; anchors.leftMargin: 20
        anchors.top: musicSublistTitle.bottom;
        ScrollBar.vertical: ScrollBar {}
        header: headerV
        model: mediaListModels
        clip: true
        focus: true
        delegate: AlbumSublistDelegate{
            width: listview.width; height: 56
            m_musicIndex: index + 1
            m_mediaMeta: mediaListModels.get(index).musicData //meida数据
            backgroundVisible: true
            normalBackgroundVisible: index % 2 === 0
            onDoubleClicked: {
                Presenter.playAlbum(m_albumData.name, mediaListModels.get(index).musicData.hash);
            }
        }
        onContentYChanged: {
            if(listview.contentY <= 164){
                musicSublistTitle.titleHeight = 244;
            }else{
                musicSublistTitle.titleHeight = 80;
            }
        }
    }

}
