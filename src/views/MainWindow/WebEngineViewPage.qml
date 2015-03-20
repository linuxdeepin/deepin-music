import QtQuick 2.4
import QtWebEngine 1.0

WebEngineView {
    id: webEngineView
    focus: false
    
    signal playMusicById(int musicId)
    signal playMusicByIds(string musicIds)
    signal playSonglist(string songlistName)

    onLinkHovered:{

    }

    onJavaScriptConsoleMessage:{
        if(level === 2){
            var rpc;
            try{
                rpc = JSON.parse(message)
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
                        }else if (rpc.type == 'songlist'){
                            print(rpc.rpcActionType, rpc.type, rpc.rpcId);
                            webEngineView.playSonglist(rpc.rpcId);
                        }else if (rpc.type == 'album'){
                            print(rpc.rpcActionType, rpc.type, rpc.rpcId);
                        }else if (rpc.type == 'artist'){
                            print(rpc.rpcActionType, rpc.type, rpc.rpcId);
                        }
                    }else if(rpc.rpcActionType == 'Download'){
                        print(rpc.rpcActionType, rpc.type, rpc.rpcId);
                    }
                }else if (rpc.hasOwnProperty('rpcId') && !rpc.hasOwnProperty('type')){
                    print(rpc.rpcActionType, rpc.rpcId);
                }else{
                    print(rpc)
                }
            }catch(error){
                console.log(error)
            }
        }
    }

    onNavigationRequested:{
        print(request.url)
    }

    onLoadingChanged:{
        if (loadRequest.status == WebEngineView.LoadStartedStatus){
            print('loading html')
        }
        else if (loadRequest.status == WebEngineView.LoadSucceededStatus){
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
                ")
        }
        else if (loadRequest.status == WebEngineView.LoadFailedStatus){
            print('load html faile')
        }
    }
}
