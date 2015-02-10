import QtQuick 2.0
import QtWebEngine 1.0

WebEngineView {
    id: root
    property var viewID: 'WebMusic360Page'
    url: "http://10.0.0.153:8093/"
    focus: false
    onLinkHovered:{

    }

    onJavaScriptConsoleMessage:{
    	if(level > 0){
    		print(level, message, lineNumber, sourceID)
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
            runJavaScript("console.log(Dmusic)")
        }
        else if (loadRequest.status == WebEngineView.LoadFailedStatus){
            print('load html faile')
        }
    }
}
