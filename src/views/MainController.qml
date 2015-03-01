import QtQuick 2.0

Item {

    property var rootWindow
    property var mainWindow
    property var simpleWindow

    function showMainWindow() {
        rootWindow.width = 960;
        rootWindow.height = 660;
        mainWindow.visible = true
        simpleWindow.visible = false

        WindowManageWorker.windowMode = 'Full'
    }

    function showSimpleWindow() {
        rootWindow.width = 300;
        rootWindow.height = 660;
        mainWindow.visible = false
        simpleWindow.visible = true

        WindowManageWorker.windowMode = 'Simple'
    }

	Connections {
        target: mainWindow.titleBar
        onShowMinimized: {
            MainWindow.showMinimized()
        }
        onMenuShowed: {
            MenuWorker.showSettingMenu();
        }
        onSimpleWindowShowed: {
            showSimpleWindow();
        }
    }

    Connections {
        target: simpleWindow.titleBar
        onShowMinimized: {
            MainWindow.showMinimized();
        }
        onMenuShowed: {
            MenuWorker.showSettingMenu();
        }
        onMainWindowShowed: {
            showMainWindow();
        }
    }
}
