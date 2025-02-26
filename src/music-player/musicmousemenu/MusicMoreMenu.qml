// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0
import audio.global 1.0
import "../dialogs"

Menu{
    property string pageHash: ""
    property var mediaData
    property var encodings: []
    property bool isPlaying: (globalVariant.curPlayingStatus === DmGlobal.Playing) ? true : false
    property bool activeMeta: (globalVariant.curPlayingHash === mediaData.hash) ? true : false;
    property int itemIndex: -1  //判断当前菜单属于哪个列表项，用于控制菜单按钮的显隐


    id: moreMenu
    width: 200
    MenuItem {
        text: (isPlaying && activeMeta) ? qsTr("Pause") : qsTr("Play")
        onTriggered: {
            if(isPlaying && activeMeta){
                Presenter.pause();
            }else{
                if (pageHash === "album") {
                    Presenter.playAlbum(mediaData.album, mediaData.hash);
                } else if(pageHash === "artist") {
                    Presenter.playArtist(mediaData.artist, mediaData.hash);
                } else if (pageHash ==="play") {
                    Presenter.setActivateMeta(mediaData.hash)
                    Presenter.play()
                } else {
                    Presenter.playPlaylist(moreMenu.pageHash, mediaData.hash);
                }
            }
        }
    }
    ImportMenu {
        id: musicMoreImportMenu;
        title: qsTr("Add to")
        pageHash: moreMenu.pageHash
        onImportMenuClosed: {
            moreMenu.close();
        }
    }
    MenuSeparator {}
    MenuItem {
        text: qsTr("Open in file manager")
        onTriggered: {
            Presenter.showMetaFile(mediaData.hash)
        }
    }
    DeleteSonglistDialog{
        id: removeSong;
        listHash: pageHash
    }
    MenuItem {
        text: pageHash === "play" ? qsTr("Remove from play queue") : qsTr("Remove from playlist")
        onTriggered: {
            removeSong.show();
        }
    }
    DeleteLocalDialog{
        id: deleteLocal;
        listHash: pageHash;
    }
    MenuItem {
        text: qsTr("Delete from local disk")
        onTriggered: {
            deleteLocal.show();
        }
    }
    MenuSeparator {}
    Menu {
        title: qsTr("Encoding")
        width: 200
        Repeater {
            model: encodings
            delegate: MenuItem {
                text: encodings[index]
                checked: {
                    if (mediaData !== null && mediaData.codec === encodings[index])
                        return true
                    else
                        return false
                }

                onTriggered: {
                    Presenter.updateMetaCodec(mediaData.hash, encodings[index])
                }
            }
        }
    }
    MenuSeparator {}
    MusicInfoDialog {
        id: infoDialog
        musicData: moreMenu.mediaData;
    }
    MenuItem {
        text: qsTr("Song info")
        onTriggered: {
            infoDialog.show()
        }
    }

    onMediaDataChanged: {
        if("hash" in mediaData){
            var list = [];
            list.push(mediaData.hash);
            removeSong.deleteHashList = list
            deleteLocal.deleteHashList = list
            removeSong.musicTitle = mediaData.title
            deleteLocal.musicTitle = mediaData.title
            musicMoreImportMenu.mediaHashList = list
        }
    }

    onAboutToShow: {
        encodings = Presenter.detectEncodings(mediaData.hash)
    }

    function onUpdatedMetaCodec(meta, preAlbum, preArtist) {
        //console.log("onUpdatedMetaCodec>>>>>>>>" + meta.codec + "    " + mediaData.hash)
        mediaData.codec = meta.codec
    }

    Component.onCompleted: {
        Presenter.updatedMetaCodec.connect(onUpdatedMetaCodec)
    }
}
