import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtWebEngine 1.0

Rectangle {

    id: root

    Column{

        Row{
            Rectangle {
                id: leftSideBar
                width: 60
                height: root.height - buttonBar.height
                color: "green"
            }

            Column{

                Rectangle {
                    id: titleBar
                    width: root.width - leftSideBar.width
                    height: 25
                    color: "red"
                }

                WebEngineView {
                    id: webView
                    width: root.width - leftSideBar.width
                    height: root.height - titleBar.height - buttonBar.height
                    url: "http://10.0.0.153:8093/"
                    // url: "./test.html"
                    onLinkHovered:{

                    }

                    onJavaScriptConsoleMessage:{
                        print(level, message, lineNumber, sourceID)
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
            }
        }

        Rectangle {
            id: buttonBar
            width: root.width
            height: 100
            color: "#345678"
        }
    }
}
