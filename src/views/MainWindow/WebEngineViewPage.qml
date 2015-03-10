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
                print(message)
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
                Dmusic.play = function(id, type) {
                        var message = JSON.stringify({
                            'rpcVersion': '1.0', 
                            'rpcId':id, 
                            'rpcType': type
                        })
                    console.error(message);
                };")
        }
        else if (loadRequest.status == WebEngineView.LoadFailedStatus){
            print('load html faile')
        }
    }
}
