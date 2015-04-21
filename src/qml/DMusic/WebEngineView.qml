import QtQuick 2.3
import com.canonical.Oxide 1.0

WebView {
	id: root
	property string usContext: "messaging://"
    property string oxideUserJs: '../../qml/DMusic/oxide-user.js'
    // oxideUserJs: '../../views/MainWindow/oxide-user.js'
	property var devtoolsEnabled: true
    property var devtoolsPort: 8080

    context: webcontext

    function runJavaScript(code){
        rootFrame.sendMessage(root.usContext, 'runJavaScript', {"detail": code});
    }

    WebContext {
        id: webcontext
        cachePath: "file:///tmp/oxide/"
        dataPath: "file:///tmp/oxide/"
        devtoolsEnabled: root.devtoolsEnabled
        devtoolsPort: root.devtoolsPort

        userScripts: [
            UserScript {
                context: root.usContext
                matchAllFrames: true
                url: {
                    print(Qt.resolvedUrl(oxideUserJs))
                    return Qt.resolvedUrl(oxideUserJs)
                }
            }
        ]
    }
}