// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0

ListModel {
    signal metaCodecChanged(string name)

    id: albumModel
    property int listCount: 0
    property int musicCount: 0
    function loadArtistDatas() {
        var tempModel = Presenter.allAlbumInfos();
        albumModel.clear();
        listCount = 0
        musicCount = 0
        for(var i = 0; i < tempModel.length; i++){
            listCount++;
            var albumData = tempModel[i];
            var curCount = 0;
            for(var key in albumData.musicinfos){
                curCount++;
                musicCount++;
                albumData.coverUrl = albumData.musicinfos[key].coverUrl
                albumData.musicCount = curCount;
            }
            albumModel.append(albumData);
        }
    }

    function onDeleteOneMetaFromModel(playlistHashs, hash) {
        for (var i = 0; i < playlistHashs.length; i++){
            if (playlistHashs[i] === "all") {
                for(var j = albumModel.listCount -1; j >=0; j--){
                    for (var key in albumModel.get(j).musicinfos){
                        var curMusicInfos = albumModel.get(j).musicinfos;
                        var albumMusicCount = albumModel.get(j).musicCount;
                        if(key === hash){
                            delete curMusicInfos[hash];
                            albumMusicCount--;
                            musicCount--;
                            if(albumMusicCount <= 0){
                                albumModel.remove(j);
                                albumModel.listCount--
                            }else{
                                albumModel.setProperty(j, "musicCount", albumMusicCount);
                                albumModel.setProperty(j, "musicinfos", curMusicInfos);
                            }
                            return;
                        }
                    }
                }

            }else if(playlistHashs[i] === "fav"){
                for(var f = albumModel.listCount -1; f >=0; f--){
                    for(var key2 in albumModel.get(f).musicinfos){
                        if(hash === key2){
                            var curMusicInfos2 = albumModel.get(f).musicinfos;
                            curMusicInfos2[hash].favourite = false;
                            albumModel.setProperty(f, "musicinfos", curMusicInfos2);
                            return;
                        }
                    }
                }
            }
        }
    }

    function onAddOneMetaFinished(playlistHashs, mediaData){
        for (var i = 0; i < playlistHashs.length; i++){
            if (playlistHashs[i] === "fav"){
                for(var j = albumModel.listCount -1; j >=0; j--){
                    for(var key in albumModel.get(j).musicinfos){
                        if(mediaData.hash === key){
                            var curAlbum = albumModel.get(j).musicinfos;
                            curAlbum[key].favourite = true;
                            albumModel.setProperty(j, "musicinfos", curAlbum);
                            return;
                        }
                    }
                }
            }
        }
    }

    function playlistSortChanged(playlistHash) {
        if (playlistHash === "album")
            loadArtistDatas()
    }
    function onUpdatedMetaCodec(meta, preAlbum, preArtist) {
        for (var i = 0; i < albumModel.listCount; i++) {
            if (albumModel.get(i).name === preAlbum) {
                var album = albumModel.get(i)
                var tmpModel = album.musicinfos
                for (var key in tmpModel) {
                    if (tmpModel[key].hash === meta.hash) {
                        tmpModel[key] = meta
                        break
                    }
                }
                album.musicinfos = tmpModel
                album.name = meta.album
                album.artist = meta.artist
                albumModel.set(i, album)
                metaCodecChanged(album.name)
            }
        }
    }
    Component.onCompleted: {
        albumModel.loadArtistDatas();
        Presenter.importFinished.connect(loadArtistDatas)
        Presenter.deleteOneMeta.connect(onDeleteOneMetaFromModel);
        Presenter.addOneMeta.connect(onAddOneMetaFinished);
        Presenter.playlistSortChanged.connect(playlistSortChanged)
        Presenter.updatedMetaCodec.connect(onUpdatedMetaCodec)
    }
}
