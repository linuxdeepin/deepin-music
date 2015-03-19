import QtQuick 2.4

Item {

    property var rootWindow
    property var mainWindow
    property var simpleWindowLoader
    property var miniWindow
    property var constants

    function initConnect() {
        WindowManageWorker.mainWindowShowed.connect(showMainWindow);
        WindowManageWorker.simpleWindowShowed.connect(showSimpleWindow);
        WindowManageWorker.miniWindowShowed.connect(showMiniWindow);
        Qt.globalPos = WindowManageWorker.cursorPos;
        MenuWorker.miniTrigger.connect(showMiniWindow);
    }

    function showMainWindow() {
        MainWindow.show();
        rootWindow.width = constants.mainWindowWidth;
        rootWindow.height = constants.mainWindowHeight;
        mainWindow.visible = true;
        simpleWindowLoader.visible = false;
        // simpleWindowLoader.source = ''
        miniWindow.visible = false;

        WindowManageWorker.windowMode = 'Full'
        WindowManageWorker.lastWindowMode = 'Full'
    }

    function showSimpleWindow() {
        MainWindow.show();
        rootWindow.width = constants.simpleWindowWidth;
        rootWindow.height = constants.simpleWindowHeight;
        mainWindow.visible = false;
        simpleWindowLoader.visible = true;
        simpleWindowLoader.setSource('./SimpleWindow/SimpleWindow.qml', { "rootWindow": rootWindow });

        miniWindow.visible = false;
        WindowManageWorker.windowMode = 'Simple'
        WindowManageWorker.lastWindowMode = 'Simple'
    }

    function showMiniWindow() {
        miniWindow.visible = true;
        MainWindow.hide();

        WindowManageWorker.windowMode = 'Mini'
    }

    function closeAll() {
        Qt.quit();
    }

	Connections {
        target: mainWindow.titleBar

        onSimpleWindowShowed: {
            showSimpleWindow();
        }
        
        onMenuShowed: {
            MenuWorker.showSettingMenu();
        }

        onShowMinimized: {
            MainWindow.showMinimized()
        }

        onClosed: {
            closeAll();
        }
    }

    // Connections {
    //     target: simpleWindow.titleBar

    //     onMainWindowShowed: {
    //         showMainWindow();
    //     }
        
    //     onMenuShowed: {
    //         MenuWorker.showSettingMenu();
    //     }
        

    //     onShowMinimized: {
    //         MainWindow.showMinimized();
    //     }

    //     onClosed: {
    //         closeAll();
    //     }
    // }

    Connections {
        target: miniWindow

        onExpandNoraml : {
            WindowManageWorker.showNormal();
        }

        onClosed:{
            closeAll();
        }
    }

    Component.onCompleted: {
        initConnect();
    }
}
