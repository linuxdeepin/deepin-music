// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0

ListModel {
    property var meidaDataMap
    id: mediaListModels
    property int modelCount: mediaListModels.count
    function loadMediaDatas()
    {
        var tmplist = meidaDataMap;
        mediaListModels.clear();
        for(var key in tmplist){
            tmplist[key].inMulitSelect = false;
            mediaListModels.append(tmplist[key]);
        }

    }
    function onDeleteOneMeta(playlistHashs, hash){
        for (var i = 0; i < playlistHashs.length; i++){
            if (playlistHashs[i] === "all") {
                for(var j = mediaListModels.count - 1; j >= 0; j--){
                    if(hash === mediaListModels.get(j).hash){
                        mediaListModels.remove(j);
                        break;
                    }
                }
            }else if(playlistHashs[i] === "fav"){
                for(var k = 0; k < mediaListModels.count; k++){
                    if(hash === mediaListModels.get(k).hash){
                        mediaListModels.setProperty(k, "favourite", false);
                       break;
                    }
                }
            }
        }
    }
    function onAddOneMetaFinished(playlistHashs, mediaData) {
        for (var i = 0; i < playlistHashs.length; i++){
            if (playlistHashs[i] === "fav"){
                for (var j = 0; j < mediaListModels.count; j++) {
                    if (mediaListModels.get(j).hash === mediaData.hash) {
                        mediaListModels.setProperty(j, "favourite", true);
                        break
                    }
                }
            }
        }
    }
    function onUpdatedMetaCodec(meta, preAlbum, preArtist) {
        for (var j = 0; j < mediaListModels.count; j++) {
            if (mediaListModels.get(j)["hash"] === meta.hash) {
                mediaListModels.set(j, meta)
                break
            }
        }
    }
    Component.onCompleted: {
        mediaListModels.loadMediaDatas();
        Presenter.deleteOneMeta.connect(onDeleteOneMeta);
        Presenter.addOneMeta.connect(onAddOneMetaFinished);
        Presenter.updatedMetaCodec.connect(onUpdatedMetaCodec)
    }
    onModelCountChanged: {
        if(modelCount <= 0)
            globalVariant.returnUpperlevelView();
    }
}
