// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0

ListModel {
    property int listCount: 0
    property int musicCount: 0
    signal metaCodecChanged(string name)

    id: artistModels
    function loadArtistDatas() {
        var tempModel = Presenter.allArtistInfos();
        artistModels.clear();
        listCount = 0
        musicCount = 0
        for(var i = 0; i < tempModel.length; i++){
            var artistData = tempModel[i];
            listCount++;
            var curCount = 0;
            for(var key in artistData.musicinfos){
                curCount++;
                musicCount++;
                artistData.coverUrl = artistData.musicinfos[key].coverUrl
                artistData.musicCount = curCount;
            }
            artistModels.append(artistData);
        }
    }

    function onDeleteOneMetaFromModel(playlistHashs, hash) {
        for (var i = 0; i < playlistHashs.length; i++){
            if (playlistHashs[i] === "all") {
                for(var j = artistModels.listCount -1; j >=0; j--){
                    for (var key in artistModels.get(j).musicinfos){
                        var curMusicInfos = artistModels.get(j).musicinfos;
                        var albumMusicCount = artistModels.get(j).musicCount;
                        if(key === hash){
                            delete curMusicInfos[hash];
                            albumMusicCount--;
                            musicCount--;
                            if(albumMusicCount <= 0){
                                artistModels.remove(j);
                                artistModels.listCount--
                            }else{
                                artistModels.setProperty(j, "musicCount", albumMusicCount);
                                artistModels.setProperty(j, "musicinfos", curMusicInfos);
                            }
                            return;
                        }
                    }
                }

            }else if(playlistHashs[i] === "fav"){
                for(var f = artistModels.listCount -1; f >=0; f--){
                    for(var key2 in artistModels.get(f).musicinfos){
                        if(hash === key2){
                            var curMusicInfos2 = artistModels.get(f).musicinfos;
                            curMusicInfos2[hash].favourite = false;
                            artistModels.setProperty(f, "musicinfos", curMusicInfos2);
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
                for(var j = artistModels.listCount -1; j >=0; j--){
                    for(var key in artistModels.get(j).musicinfos){
                        if(mediaData.hash === key){
                            var curAlbum = artistModels.get(j).musicinfos;
                            curAlbum[key].favourite = true;
                            artistModels.setProperty(j, "musicinfos", curAlbum);
                            return;
                        }
                    }
                }
            }
        }
    }

    function playlistSortChanged(playlistHash) {
        if (playlistHash === "artist"){
            artistModels.loadArtistDatas();
        }
    }

    function onUpdatedMetaCodec(meta, preAlbum, preArtist) {
        for (var i = 0; i < artistModels.listCount; i++) {
            if (artistModels.get(i).name === preArtist) {

                var artist = artistModels.get(i)
                var artistModel = artist.musicinfos
                for (var key in artistModel) {
                    if (artistModel[key].hash === meta.hash) {
                        artistModel[key] = meta
                        break
                    }
                }
                artist.musicinfos = artistModel
                artist.name = meta.artist
                artistModels.set(i, artist)
                metaCodecChanged(artist.name)
            }
        }
    }

    Component.onCompleted: {
        artistModels.loadArtistDatas();
        Presenter.importFinished.connect(loadArtistDatas);
        Presenter.deleteOneMeta.connect(onDeleteOneMetaFromModel);
        Presenter.addOneMeta.connect(onAddOneMetaFinished);
        Presenter.playlistSortChanged.connect(playlistSortChanged)
        Presenter.updatedMetaCodec.connect(onUpdatedMetaCodec)
    }
}
