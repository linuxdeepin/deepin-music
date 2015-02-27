import QtQuick 2.0

Item {
    property var bgImage
	property var titleBar

	Connections {
        target: titleBar
        onShowMinimized: SimpleWindow.showMinimized()
        onMenuShowed: MenuWorker.showSettingMenu()
        onMainWindowShowed: WindowManageWorker.mainWindowShowed()
    }
}
