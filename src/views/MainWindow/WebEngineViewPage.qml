import QtQuick 2.3
// import QtWebEngine 1.0
import DMusic 1.0

WebEngineView {
    id: webEngineView
    focus: false
    oxideUserJs: '../../views/MainWindow/oxide-user.js'

    signal playMusicById(int musicId)
    signal playMusicByIds(string musicIds)
    signal playSonglistByName(string songlistName)
    signal playSonglistById(int songlistId)
    signal playAlbumById(int albumId)

    signal addFavorite(int musicId)
    signal removeFavorite(int musicId)
    signal downloadSong(int musicId)
    signal downloadSongs(string musicIds)

    onJavaScriptConsoleMessage:{
        if(level === 2){
            var rpc;
            try{
                rpc = JSON.parse(message);
                if(rpc.hasOwnProperty('rpcId') && rpc.hasOwnProperty('type')){
                    if (rpc.rpcActionType == 'Play'){
                        if (rpc.type == 'music'){
                            print(rpc.rpcActionType, rpc.type, rpc.rpcId, typeof rpc.rpcId);
                            if (typeof rpc.rpcId === 'number'){
                                webEngineView.playMusicById(rpc.rpcId)
                            }else if(typeof rpc.rpcId === 'string'){
                                webEngineView.playMusicByIds(rpc.rpcId)
                            }
                        }else if (rpc.type == 'album'){
                            print(rpc.rpcActionType, rpc.type, rpc.rpcId);
                            webEngineView.playAlbumById(rpc.rpcId);
                        }else if (rpc.type == 'songlist'){
                            print(rpc.rpcActionType, rpc.type, rpc.rpcId);
                            if (typeof rpc.rpcId == 'number'){
                                webEngineView.playSonglistById(rpc.rpcId);
                            }else if(typeof rpc.rpcId === 'string'){
                                webEngineView.playSonglistByName(rpc.rpcId);
                            }
                        }else if (rpc.type == 'artist'){
                            print(rpc.rpcActionType, rpc.type, rpc.rpcId);
                        }
                    }else if(rpc.rpcActionType == 'Download'){
                        print(rpc.rpcActionType, rpc.type, rpc.rpcId);

                        if (typeof rpc.rpcId === 'number'){
                            webEngineView.downloadSong(rpc.rpcId);
                        }else if(typeof rpc.rpcId === 'string'){
                            webEngineView.downloadSongs(rpc.rpcId);
                        }
                    }
                }else if (rpc.hasOwnProperty('rpcId') && !rpc.hasOwnProperty('type')){
                    print(rpc.rpcActionType, rpc.rpcId, typeof rpc.rpcId);
                    if (rpc.rpcActionType == "AddFavorite") {
                        webEngineView.addFavorite(rpc.rpcId);
                    }else if (rpc.rpcActionType == "RemoveFavorite")
                    {
                        webEngineView.removeFavorite(rpc.rpcId);
                    }
                }else{
                    print(rpc)
                }
            }catch(error){
                console.log(error, message)
            }
        }
    }

    onLoadingChanged:{
        if (loadEvent.type == 0){
            print('loading html')
        }
        else if (loadEvent.type == 2){
            print('load html successs')
            runJavaScript("
                (function(global, undefined){
                    var Dmusic = {};

                    function play(id, type){
                        var message = JSON.stringify({
                            'rpcVersion': '1.0', 
                            'rpcId':id, 
                            'rpcActionType': 'Play',
                            'type': type
                        })
                        console.error(message);
                    }


                    function enqueue(id, type){
                        var message = JSON.stringify({
                            'rpcVersion': '1.0', 
                            'rpcId':id, 
                            'rpcActionType': 'Enqueue',
                            'type': type
                        })
                        console.error(message);
                    }


                    function download(id, type){
                        var message = JSON.stringify({
                            'rpcVersion': '1.0', 
                            'rpcId':id, 
                            'rpcActionType': 'Download',
                            'type': type
                        })
                        console.error(message);
                    }


                    function addFavorite(id){

                        var message = JSON.stringify({
                            'rpcVersion': '1.0', 
                            'rpcId':id,
                            'rpcActionType': 'AddFavorite'
                        })
                        console.error(message);

                        return { success: true };
                    }

                    function removeFavorite(id){
                         var message = JSON.stringify({
                            'rpcVersion': '1.0', 
                            'rpcId':id,
                            'rpcActionType': 'RemoveFavorite'
                        })
                        console.error(message);

                        return { success: true };
                    }


                    Dmusic.play = play;

                    Dmusic.enqueue = enqueue;

                    Dmusic.download = download;

                    Dmusic.addFavorite = addFavorite;

                    Dmusic.removeFavorite = removeFavorite;

                    global.Dmusic = Dmusic;
                    })(window);
                ");
        }
        else{
            print('load html faile')
        }
    }
}
