import QtQuick 2.3

Item {

    property var rootWindow
    property var mainWindow
    property var simpleWindowLoader
    property var miniWindow
    property var lrcWindow
    property var constants

    function initConnect() {
        WindowManageWorker.mainWindowShowed.connect(showMainWindow);
        WindowManageWorker.simpleWindowShowed.connect(showSimpleWindow);
        WindowManageWorker.miniWindowShowed.connect(showMiniWindow);
        Qt.globalPos = WindowManageWorker.cursorPos;
        Qt.createItems = []
        Qt.destoryItems = []
        MenuWorker.miniTrigger.connect(showMiniWindow);
        MenuWorker.settingTrigger.connect(loadSettingPage);
    }

    function showMainWindow() {
        MainWindow.show();
        rootWindow.width = constants.mainWindowWidth;
        rootWindow.height = constants.mainWindowHeight;
        mainWindow.visible = true;
        simpleWindowLoader.source = ''
        simpleWindowLoader.focus = false;

        miniWindow.visible = false;

        WindowManageWorker.windowMode = 'Full'
        WindowManageWorker.lastWindowMode = 'Full'
    }

    function showSimpleWindow() {
        MainWindow.show();
        rootWindow.width = constants.simpleWindowWidth;
        rootWindow.height = constants.simpleWindowHeight;
        mainWindow.visible = false;
        simpleWindowLoader.source = './SimpleWindow/SimpleWindow.qml';
        simpleWindowLoader.focus = true;

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

    function destoryTermporyWindow(){
        mainWindow.temporaryLoader.source = ''
        mainWindow.focus = true;
    }

    function loadSettingPage(){
        WindowManageWorker.switchPageByID('SettingPage')
    }

	Connections {
        target: mainWindow.titleBar

        onSimpleWindowShowed: {
            showSimpleWindow();
        }
        
        onMenuShowed: {
            MenuWorker.settingMenuShow();
        }

        onShowMinimized: {
            MainWindow.showMinimized()
        }

        onClosed: {
            closeAll();
        }
    }

    Connections {
        target: mainWindow.playBottomBar.playlistButton
        onClicked:{
            if(mainWindow.temporaryLoader.source == ''){
                mainWindow.temporaryLoader.source = './TemporaryWindow/TemporaryWindow.qml'
                mainWindow.temporaryLoader.focus = true;
                mainWindow.focus = false;
            }else{
                destoryTermporyWindow();
            }
        } 
    }

    Connections {
        target: mainWindow.playBottomBar.lrcButton
        onClicked:{
            lrcWindow.visible = !lrcWindow.visible;
        } 
    }

    Connections {
        target: mainWindow.temporaryLoader
        onLoaded:{
            var temporyWindow = mainWindow.temporaryLoader.item;
            if (temporyWindow){
                var closeButton = temporyWindow.closeButton;
                closeButton.clicked.connect(destoryTermporyWindow);
            }
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
