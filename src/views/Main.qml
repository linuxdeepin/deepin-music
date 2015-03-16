import QtQuick 2.4
import QtQuick.Window 2.1
import DMusic 1.0
import "./MainWindow"
import "./SimpleWindow"
import "./MiniWindow"

Item {
    id: root
    objectName: "root"

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
        
        width: root.width
        height: root.heigh
        visible : true
    }

    SimpleWindow {
        id: simpleWindow
        constants: constants
        width: root.width
        height: root.height
        anchors.fill: parent
        visible : false
    }

    MiniWindow {
        id: miniWindow
        constants: constants
        width: constants.miniWindowWidth
        height: constants.miniWindowHeight
    }

    MainController {
        id: mainController
        rootWindow: root
        mainWindow: mainWindow
        simpleWindow: simpleWindow
        miniWindow: miniWindow
        constants: constants
    }

    MediaController {
        id: mediaController
        mainWindow: mainWindow
        simpleWindow: simpleWindow
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
            mainWindow.webEngineViewPage.url = "http://music.haosou.com/_linuxdeepin/";
            root.focus = true;
        }
    }

    Timer {
        id: positionTimer
        repeat: false
        interval: 100
        running: false
    }
}
