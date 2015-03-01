import QtQuick 2.0
import QtQuick.Window 2.1
import DMusic 1.0

Item {
    id: root
    objectName: "root"

    width: 960
    height: 660

    MainWindow{
        id: mainWindow
        objectName: "MainWindow"

        anchors.fill: parent
        
        width: root.width
        height: root.heigh
        visible : true
    }

    SimpleWindow{
        id: simpleWindow
        width: root.width
        height: root.height
        anchors.fill: parent
        visible : false
    }

    MainController {
        id: mainController
        rootWindow: root
        mainWindow: mainWindow
        simpleWindow: simpleWindow
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_F1) {
            mainWindow.mainController.resetSkin();
            simpleWindow.simpleController.resetSkin();
        }else if (event.key == Qt.Key_F2) {
            mainWindow.mainController.setSkinByImage();
            simpleWindow.simpleController.setSkinByImage();
        }
    }
}
