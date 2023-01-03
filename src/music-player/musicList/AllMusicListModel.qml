// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.0
import org.deepin.dtk 1.0

ListModel {
    property string dataHash: ""
    id: mediaModels
    property string m_activateMetaHash: "undefine"
    function loadMediaDatas() {
        //console.log("loadMediaDatas..........." + dataHash)
        mediaModels.clear();
        var tempModel = Presenter.getPlaylistMetas(dataHash);
        for(var i = 0; i < tempModel.length; i++){
            tempModel[i].inMulitSelect = false;
            tempModel[i].dragFlag = false
            mediaModels.append(tempModel[i]);
        }
    }
    function updateModelData(){
        var media = Presenter.getActivateMeta();
        //console.log("====updateModelData=====" + media.hash)
        m_activateMetaHash = media.hash;
    }

    function onDeleteOneMeta(playlistHashs, hash){
        for (var i = 0; i < playlistHashs.length; i++){
            if (playlistHashs[i] === dataHash) {
                for(var j = mediaModels.count - 1; j >= 0; j--){
                    if(hash === mediaModels.get(j).hash){
                        mediaModels.remove(j);
                        break;
                    }
                }
            }else if(playlistHashs[i] === "fav"){
                for(var k = 0; k < mediaModels.count; k++){
                    if(hash == mediaModels.get(k).hash){
                       mediaModels.setProperty(k,"favourite",false)
                       break;
                    }
                }
            }
        }
    }
    function onAddOneMeta(playlistHashs, meta) {
        for (var i = 0; i < playlistHashs.length; i++) {
            if (playlistHashs[i] === dataHash) {
                meta.inMulitSelect = false
                meta.dragFlag = false
                mediaModels.append(meta);
            }
            if (playlistHashs[i] === "fav") {
                for (var j = 0; j < mediaModels.count; j++) {
                    if (mediaModels.get(j)["hash"] === meta.hash) {
                        mediaModels.setProperty(j,"favourite",true)
                        break
                    }
                }
            }
        }
    }
    function playlistSortChanged(playlistHash) {
        //console.log("playlistSortChanged........", playlistHash)
        mediaModels.loadMediaDatas()
    }
    function onUpdatedMetaCodec(meta, preAlbum, preArtist) {
        for (var j = 0; j < mediaModels.count; j++) {
            if (mediaModels.get(j)["hash"] === meta.hash) {
                mediaModels.set(j, meta)
                break
            }
        }
    }

    Component.onCompleted: {
        mediaModels.loadMediaDatas();
        Presenter.importFinished.connect(loadMediaDatas);
        Presenter.deleteOneMeta.connect(onDeleteOneMeta);
        Presenter.addOneMeta.connect(onAddOneMeta);
        Presenter.playlistSortChanged.connect(playlistSortChanged)
        Presenter.updatedMetaCodec.connect(onUpdatedMetaCodec)
    }
}
