import QtQuick 2.3
import QtQuick.Window 2.1
import DMusic 1.0
import "./MainWindow"
import "./SimpleWindow"
import "./MiniWindow"
import "./TemporaryWindow"
import "./LrcWindow"

Rectangle {
    id: root
    objectName: "root"

    property var mainWindow: mainWindow
    property var mainController: mainController
    property var positionTimer: positionTimer

    width: constants.mainWindowWidth
    height: constants.mainWindowHeight


    focus: true

    Constants{
        id: constants
    }

    MainWindow {
        id: mainWindow
        objectName: "MainWindow"

        constants: constants
        anchors.fill: parent
        width: constants.mainWindowWidth
        height: constants.mainWindowHeight
        visible : true
    }

    MiniWindow {
        id: miniWindow
        constants: constants
        width: constants.miniWindowWidth
        height: constants.miniWindowHeight
    }

    Loader {
        id: simpleWindowLoader
        objectName: 'simpleWindowLoader'
        anchors.fill: parent
        width: root.width
        height: root.height
    }
    Loader {
        id: temporaryLoader
        y: 150
        anchors.right: parent.right
        asynchronous: true
    }

    MainController {
        id: mainController
        rootWindow: root
        mainWindow: mainWindow
        simpleWindowLoader: simpleWindowLoader
        miniWindow: miniWindow
        temporaryLoader: temporaryLoader
        constants: constants
    }

    MediaController {
        id: mediaController
        mainWindow: mainWindow
        miniWindow: miniWindow
        positionTimer: positionTimer
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_F1) {
            MediaPlayer.setPlaylistByName('temporary');
        }else if (event.key == Qt.Key_F2) {
            MediaPlayer.setPlaylistByName('favorite');
        }
    }

    Timer {
        repeat: false
        interval: 100
        running: true
        onTriggered:{
            mainWindow.webEngineViewPage.url = Qt.resolvedUrl("http://music.haosou.com/_linuxdeepin/");
            root.focus = true;
        }
    }

    Timer {
        id: positionTimer
        repeat: false
        interval: 100
        running: false
    }

    Component.onCompleted: {
        Qt.constants = constants
        Qt.mainWindow = mainWindow
        Qt.mainController= mainController
        Qt.positionTimer = positionTimer
        Qt.globalObject = MainWindow.globalObject

        function get(arg) {
            return eval(arg)
        }
        Qt.get = get
    }
}
