import QtQuick 2.4
import QtWebEngine 1.0

WebEngineView {
    id: webEngineView
    focus: false
    
    signal playMusicByID(int musicID)

    onLinkHovered:{

    }

    onJavaScriptConsoleMessage:{
        if(level === 2){
            var rpc;
            try{
                rpc = JSON.parse(message)
                if(rpc.hasOwnProperty('rpcId') && rpc.hasOwnProperty('rpcType')){
                    webEngineView.playMusicByID(rpc.rpcId)
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
                            'rpcType': type
                        })
                        console.error(message);
                    }


                    function enqueue(id, type){
                        var message = JSON.stringify({
                            'rpcVersion': '1.0', 
                            'rpcId':id, 
                            'rpcType': type
                        })
                        console.error(message);
                    }


                    function download(id, type){
                        var message = JSON.stringify({
                            'rpcVersion': '1.0', 
                            'rpcId':id, 
                            'rpcType': type
                        })
                        console.error(message);
                    }


                    function addFavorite(id){

                        var message = JSON.stringify({
                            'rpcVersion': '1.0', 
                            'rpcId':id,
                        })
                        console.error(message);

                        return { success: true };
                    }

                    function removeFavorite(id){
                         var message = JSON.stringify({
                            'rpcVersion': '1.0', 
                            'rpcId':id,
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
