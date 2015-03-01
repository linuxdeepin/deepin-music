import QtQuick 2.0

Item {

    property var rootWindow
    property var mainWindow
    property var simpleWindow
    property var miniWindow

    function initConnect() {
        WindowManageWorker.mainWindowShowed.connect(showMainWindow);
        WindowManageWorker.simpleWindowShowed.connect(showSimpleWindow);
        WindowManageWorker.miniWindowShowed.connect(showMiniWindow);

        MenuWorker.miniTrigger.connect(showMiniWindow);

        Web360ApiWorker.playUrl.connect(playMusic)
    }

    function playMusic(url){
        MediaPlayer.stop()
        MediaPlayer.setMediaUrl(url);
        MediaPlayer.playToggle(true)
    }

    function showMainWindow() {
        MainWindow.show();
        rootWindow.width = 960;
        rootWindow.height = 660;
        mainWindow.visible = true;
        simpleWindow.visible = false;
        miniWindow.visible = false;
        
        WindowManageWorker.windowMode = 'Full'
        WindowManageWorker.lastWindowMode = 'Full'
    }

    function showSimpleWindow() {
        MainWindow.show();
        rootWindow.width = 300;
        rootWindow.height = 660;
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

    Component.onCompleted: {
        initConnect();
    }
}
