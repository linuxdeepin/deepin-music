import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtWebEngine 1.0

Rectangle {
    id: root

    Rectangle {
        id: titleBar
        width: root.width - leftSideBar.width
        height: 25
        anchors.left: leftSideBar.left
        anchors.top: root.top
        anchors.right: root.right
        anchors.bottom: webView.top
        color: "red"
    }

    Rectangle {
        id: leftSideBar
        width: 60
        height: root.height - buttonBar.height
        anchors.top: root.top
        anchors.left: root.left
        color: "green"
    }

    Rectangle {
        id: buttonBar
        width: root.width
        height: 100
        anchors.left: root.left
        anchors.top: webView.bottom
        anchors.right: root.right
        anchors.bottom: root.buttom
        color: "#345678"
    }

    WebEngineView {
        id: webView
        width: root.width - leftSideBar.width
        height: root.height - titleBar.height - buttonBar.height
        anchors.left: leftSideBar.right
        anchors.top: titleBar.bottom
        anchors.right: root.right
        // anchors.bottom: buttonBar.top
        url: "http://10.0.0.153:8093/"
        // url: "./test.html"

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
