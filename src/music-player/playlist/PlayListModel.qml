// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0

ListModel {
    id: playlistModel

    function loadplaylistData() {
        playlistModel.clear()
        var tempModel = Presenter.getPlaylistMetas("play");
        for(var i = 0; i < tempModel.length; i++) {
            tempModel[i].inMulitSelect = false;
            tempModel[i].dragFlag = false
            playlistModel.append(tempModel[i]);
        }
    }
    function onDeleteOneMeta(playlistHashs, hash) {
        for (var i = 0; i < playlistHashs.length; i++){
            if (playlistHashs[i] == "play") {
                for (var j = 0; j < playlistModel.count; j++) {
                    if (playlistModel.get(j)["hash"] === hash) {
                        playlistModel.remove(j)
                        break
                    }
                }
            } else if (playlistHashs[i] == "fav") {
                for (var k = 0; k < playlistModel.count; k++) {
                    if (playlistModel.get(k)["hash"] === hash) {
                        playlistModel.setProperty(k,"favourite",false)
                        break
                    }
                }
            }
        }
    }
    function onAddMetaFinished(playlistHashs) {
        for (var i = 0; i < playlistHashs.length; i++){
            if (playlistHashs[i] == "play")
                loadplaylistData()
        }
    }
    function onAddOneMeta(playlistHashs, meta) {
        for (var i = 0; i < playlistHashs.length; i++){
            if (playlistHashs[i] == "fav") {
                for (var j = 0; j < playlistModel.count; j++) {
                    if (playlistModel.get(j)["hash"] === meta.hash) {
                        playlistModel.setProperty(j,"favourite",true)
                        break
                    }
                }
            }
        }
    }
    function onUpdatedMetaCodec(meta, preAlbum, preArtist) {
        for (var j = 0; j < playlistModel.count; j++) {
            if (playlistModel.get(j)["hash"] === meta.hash) {
                playlistModel.set(j, meta)
                break
            }
        }
    }

    Component.onCompleted: {
        loadplaylistData();
        Presenter.deleteOneMeta.connect(onDeleteOneMeta)
        Presenter.addMetaFinished.connect(onAddMetaFinished)
        Presenter.importFinished.connect(onAddMetaFinished)
        Presenter.addOneMeta.connect(onAddOneMeta)
        Presenter.updatedMetaCodec.connect(onUpdatedMetaCodec)

        Presenter.currentPlaylistSChanged.connect(function(playlistHash){
            if (playlistHash === "") {
                playlistModel.clear()
                Presenter.clearPlayList("play")
            }
        })

        globalVariant.playingCount = Qt.binding(function(){return playlistModel.count})
    }
}
