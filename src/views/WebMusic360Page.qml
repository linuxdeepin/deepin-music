import QtQuick 2.0
import QtWebEngine 1.0

WebEngineView {
    id: webEngineView
    property var viewID: 'WebMusic360Page'
    url: "http://10.0.0.153:8093/"
    focus: false
    
    signal playMusicByID(int musicID)

    onLinkHovered:{

    }

    onJavaScriptConsoleMessage:{
    	if(level === 2){
    		var rpc = JSON.parse(message)
            if(rpc.hasOwnProperty('rpcId') && rpc.hasOwnProperty('rpcType')){
                webEngineView.playMusicByID(rpc.rpcId)
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
