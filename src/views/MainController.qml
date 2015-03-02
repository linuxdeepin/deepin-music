import QtQuick 2.0

Item {

    property var rootWindow
    property var mainWindow
    property var simpleWindow
    property var miniWindow
    property var constants

    function initConnect() {
        WindowManageWorker.mainWindowShowed.connect(showMainWindow);
        WindowManageWorker.simpleWindowShowed.connect(showSimpleWindow);
        WindowManageWorker.miniWindowShowed.connect(showMiniWindow);

        MenuWorker.miniTrigger.connect(showMiniWindow);

        Web360ApiWorker.playUrl.connect(playMusic);

    }

    function playMusic(url){
        MediaPlayer.stop()
        MediaPlayer.setMediaUrl(url);
        MediaPlayer.playToggle(true)
    }

    function showMainWindow() {
        MainWindow.show();
        rootWindow.width = constants.mainWindowWidth;
        rootWindow.height = constants.mainWindowHeight;
        mainWindow.visible = true;
        simpleWindow.visible = false;
        miniWindow.visible = false;
        
        WindowManageWorker.windowMode = 'Full'
        WindowManageWorker.lastWindowMode = 'Full'
    }

    function showSimpleWindow() {
        MainWindow.show();
        rootWindow.width = constants.simpleWindowWidth;
        rootWindow.height = constants.simpleWindowHeight;
        mainWindow.visible = false;
        simpleWindow.visible = true;
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

    Connections {
        target: simpleWindow.titleBar

        onMainWindowShowed: {
            showMainWindow();
        }
        
        onMenuShowed: {
            MenuWorker.showSettingMenu();
        }
        

        onShowMinimized: {
            MainWindow.showMinimized();
        }

        onClosed: {
            closeAll();
        }
    }

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
