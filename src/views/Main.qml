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

    Constants{
        id: constants
    }

    MainWindow {
        id: mainWindow
        objectName: "MainWindow"

        anchors.fill: parent
        
        width: root.width
        height: root.heigh
        visible : true
    }

    SimpleWindow {
        id: simpleWindow
        width: root.width
        height: root.height
        anchors.fill: parent
        visible : false
    }

    MiniWindow {
        id: miniWindow
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
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_F1) {
            mainWindow.mainWindowController.resetSkin();
            simpleWindow.simpleWindowController.resetSkin();
            miniWindow.miniWindowController.resetSkin();
        }else if (event.key == Qt.Key_F2) {
            mainWindow.mainWindowController.setSkinByImage();
            simpleWindow.simpleWindowController.setSkinByImage();
            miniWindow.miniWindowController.setSkinByImage();
        }
    }
}
