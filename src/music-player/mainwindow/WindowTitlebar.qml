// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQml 2.15
import QtQuick.Window
import QtQuick.Layouts
import org.deepin.dtk 1.0
import org.deepin.dtk 1.0 as D
import "../dialogs"

TitleBar {
    signal lrcHideBtnClicked()
    signal searchItemTriggered(string text, int type)

    property bool preHover: false
    property bool nextHover: false
    property bool preMaskVisible: true
    property bool nextMaskVisible: true
    property double opat: 1.0
    property double disableOpat: 0.4
    property var searchEditRef: null
    property string pendingSearchText: ""

    Loader { id: equalizerDlgLoader }
    Loader { id: settingDlgLoader }

    id: titleBar
    hoverEnabled: true

    // App icon - use Image on Windows since DCI icon theme may not have app icon
    Loader {
        id: iconLoader
        anchors.verticalCenter: parent.verticalCenter
        x: 10
        sourceComponent: Qt.platform.os === "windows" ? fallbackIcon : dciIcon
        Component {
            id: dciIcon
            D.DciIcon {
                name: globalVariant.appIconName
                sourceSize { width: 24; height: 24 }
                opacity: opat
                palette: D.DTK.makeIconPalette(titleBar.palette)
                mode: titleBar.D.ColorSelector.controlState
                theme: titleBar.D.ColorSelector.controlTheme
            }
        }
        Component {
            id: fallbackIcon
            Image {
                width: 24; height: 24
                source: "qrc:/dsg/img/deepin-music.svg"
                opacity: opat
            }
        }
    }
    ActionButton {
        icon.name: "go_down"
        icon.width: 12
        icon.height: 12
        hoverEnabled: true;
        visible: isLyricShow
        width: 36
        height: 36
        x: 8
        y: 10
        onClicked: lrcHideBtnClicked()
    }

    menu: Menu {
        x: 0; y: 0
        width: 200
        MenuItem {
            text: qsTr("Add playlist")
            onTriggered: {
                var tmpPlaylist = Presenter.addPlayList(qsTr("New playlist"));
                globalVariant.globalCustomPlaylistModel.onAddPlaylist(tmpPlaylist);
                globalVariant.renameNewItem();
            }}
        MenuItem {
            text: qsTr("Add music")
            onTriggered: {
                globalVariant.globalFileDlgOpen()
            }
        }
        MenuSeparator {}
        Action {
            id: equalizerControl
            text: qsTr("Equalizer")

            onTriggered: {
                if (equalizerDlgLoader.status === Loader.Null )
                    equalizerDlgLoader.setSource("../dialogs/EqualizerDialog.qml")
                if (equalizerDlgLoader.status === Loader.Ready )
                    equalizerDlgLoader.item.show()
            }
        }
        MenuItem {
            id: settingsControl
            text: qsTr("Settings")
            onTriggered: {
                if (settingDlgLoader.status === Loader.Null )
                    settingDlgLoader.setSource("../dialogs/SettingsDialog.qml")
                if (settingDlgLoader.status === Loader.Ready )
                    settingDlgLoader.item.show()
            }
        }
        MenuSeparator { }
        ThemeMenu { width: 200 }
        MenuSeparator { }
        HelpAction { }
        AboutAction {
            aboutDialog: AboutDialog {
                modality: Qt.WindowModal
                productName: qsTr("Music")
                productIcon: Qt.platform.os === "windows" ? "qrc:/dsg/img/deepin-music.svg" : globalVariant.appIconName
                description: qsTr("Music is a local music player with beautiful design and simple functions.")
                version: qsTr("Version:") + "%1".arg(Qt.application.version)
                companyLogo: globalVariant.appIconName
                websiteName: DTK.deepinWebsiteName
                websiteLink: DTK.deepinWebsiteLink
                Binding on flags {
        when: Qt.platform.os === "windows"
        value: Qt.Dialog | Qt.WindowCloseButtonHint | Qt.FramelessWindowHint
    }
                header: DialogTitleBar {
                    enableInWindowBlendBlur: false
                }
                onVisibleChanged: {
                    if (visible && Qt.platform.os === "windows") {
                        x = (Screen.width - width) / 2
                        y = (Screen.height - height) / 2
                    }
                }
            }
        }
        QuitAction {}
        /*MenuItem {
            id: quitControl
            text: qsTr("Quit")
            onTriggered: {
                var closeAction = Presenter.valueFromSettings("base.close.close_action")
//                console.log("Quit, closeAction: " + closeAction)

                if (globalVariant.closeConfirmDlgLoader.status === Loader.Null)
                    globalVariant.closeConfirmDlgLoader.setSource("../dialogs/CloseConfirmDialog.qml")

                if (closeAction == 0) {
                    //最小化
                    globalVariant.closeConfirmDlgLoader.item.isMinimize = false
                    rootWindow.close()
                } else if (closeAction == 1) {
                    //退出
                    Qt.quit()
                } else {
                    //询问
                    globalVariant.closeConfirmDlgLoader.item.isClose = Presenter.valueFromSettings("base.close.is_close")
                    if (closeAction != 2)
                        globalVariant.closeConfirmDlgLoader.item.closeAction = closeAction
                    if (!globalVariant.closeConfirmDlgLoader.item.isClose) {
                        close.accepted = false
                        globalVariant.closeConfirmDlgLoader.item.show()
                    }
                }
            }
        }*/
    }
    content: titleBarContent

    Rectangle {
        id: preMask
        width: 36
        height: width
        x: 240
        y: 7
        visible: preMaskVisible && !isLyricShow
        color: "transparent"

        MouseArea {
            anchors.fill: preMask
            hoverEnabled: true
            onEntered: {
                preHover = true
            }
            onExited: {
                preHover = false
            }
        }
    }

    Rectangle {
        id: nextMask
        width: preMask.width
        height: preMask.width
        x: 286
        y: 7
        visible: nextMaskVisible && !isLyricShow
        color: "transparent"

        MouseArea {
            anchors.fill: nextMask
            hoverEnabled: true
            onEntered: {
                nextHover = true
            }
            onExited: {
                nextHover = false
            }
        }
    }

    Component {
        id: titleBarContent

        RowLayout {
            id: titleRowLayout
            width: parent.width - 20
            anchors {
                left: parent.left
                leftMargin: 30
            }

            // 响应式显示控制 - 使用内容区域自身宽度判断
            property bool showSearchEdit: titleRowLayout.width > 400
            property bool showNavButtons: titleRowLayout.width > 300

            RowLayout {
                spacing: 10
                visible: titleRowLayout.showNavButtons  // 窗口很窄时隐藏导航按钮
                IconButton {
                    id: preBtn
                    icon.name: "arrow_ordinary_left"
                    visible: !isLyricShow
                    opacity: (globalVariant.globalSwitchButtonStatus === 2) ? opat: disableOpat
                    enabled: (globalVariant.globalSwitchButtonStatus === 2) ? true: false
                    hoverEnabled: true
                    onClicked: {
                        globalVariant.globalSwitchButtonStatus = 1; //切换到上一级页面
                    }
                    onEnabledChanged: {
                        preMaskVisible = !enabled
                    }

                    ToolTip {
                        visible: preHover || preBtn.hovered
                        text: qsTr("Previous page")
                    }
                }
                IconButton {
                    id: nextBtn
                    icon.name: "arrow_ordinary_right"
                    visible: !isLyricShow
                    opacity: (globalVariant.globalSwitchButtonStatus === 1) ? opat: disableOpat
                    enabled: (globalVariant.globalSwitchButtonStatus === 1) ? true: false
                    hoverEnabled: true
                    onClicked: {globalVariant.globalSwitchButtonStatus = 2;} //切换到下一级页面
                    onEnabledChanged: {
                        nextMaskVisible = !enabled
                    }
                    ToolTip {
                        visible: nextHover || nextBtn.hovered
                        text: qsTr("Next page")
                    }
                }
            }
            SearchEdit {
                id: searchEdit
                visible: !isLyricShow && titleRowLayout.showSearchEdit  // 窗口窄时隐藏
                opacity: opat
                Layout.preferredWidth: titleRowLayout.showSearchEdit ? 300 : 0
                Layout.alignment: Qt.AlignCenter
                placeholder: qsTr("Search")
                enabled: Presenter.isExistMeta()
                Keys.onReturnPressed: {
                    //console.log("SearchEdit: Keys.onEnterPressed....")
                    var searchResDlg = searchResultLoader.item
                    if (text.length <= 0 || !searchResDlg || searchResDlg.songList == null)
                        return

                    var type = -1
                    if (searchResDlg.songList.length > 0)
                        type = 0
                    else if (searchResDlg.artistModel.count > 0)
                        type = 1
                    else if (searchResDlg.albumModel.count > 0)
                        type = 2

                    titleBar.searchItemTriggered(text, type)
                    searchResDlg.visible = false
                }

                onTextChanged: titleRowLayout.updateSearchResults(text)
                onActiveFocusChanged: {
                    EventsFilter.setEnabled(!activeFocus)
                }
                function onMousePressed(x, y) {
                    var object = searchEdit.mapFromGlobal(x, y)
                    if (!searchEdit.contains(object)) {
                        searchEdit.focus = false
                    }
                }

                function importFinishedDatas(playlistHashs, failCount, sucessCount, existCount) {
                    searchEdit.enabled = Presenter.isExistMeta()
                }

                function deleteFinishedDatas(playlistHash) {
                    searchEdit.enabled = Presenter.isExistMeta()
                }

                Component.onCompleted: {
                    Presenter.importFinished.connect(importFinishedDatas);
                    Presenter.deleteFinished.connect(deleteFinishedDatas);
                    EventsFilter.mousePress.connect(onMousePressed)
                    searchEditRef = searchEdit
                }
            }
            Loader {
                id: searchResultLoader
                onLoaded: {
                    if (titleBar.pendingSearchText.length <= 0)
                        return

                    var text = titleBar.pendingSearchText
                    titleBar.pendingSearchText = ""
                    titleRowLayout.updateSearchResults(text)
                }
                onStatusChanged: {
                    if (status === Loader.Error) {
                        titleBar.pendingSearchText = ""
                        console.warn("Failed to load search result dialog")
                        sourceComponent = undefined
                    }
                }
            }
            Component {
                id: searchResultComponent
                SearchResultDialog {
                    width: 360
                    x: searchEdit.x - (width - searchEdit.width) / 2
                    y: 50

                    visible: false
                    function onSearchItemTriggered(value, type) {
                        titleBar.searchItemTriggered(value, type)
                        searchEdit.text = value
                        visible = false
                        //searchEdit.updateSearchText(value)
                    }
                }
            }

    function updateSearchResults(text) {
        if (text.length <= 0) {
            titleBar.pendingSearchText = ""
            if (searchResultLoader.item)
                searchResultLoader.item.visible = false
            return
        }

        if (searchResultLoader.status === Loader.Null) {
            titleBar.pendingSearchText = text
            searchResultLoader.sourceComponent = searchResultComponent
            return
        }
        if (searchResultLoader.status !== Loader.Ready) {
            titleBar.pendingSearchText = text
            return
        }

        var searchResDlg = searchResultLoader.item
        searchResDlg.songList = []
        searchResDlg.artistModel.clear()
        searchResDlg.albumModel.clear()
        searchResDlg.pattern = text

        var result = Presenter.quickSearchText(text)
        var albums = result["albums"]
        var artists = result["artists"]

        searchResDlg.songList = result["metas"]
        if (artists != null) {
            for (var i = 0; i < artists.length; i++) {
                artists[i].type = "artist"
                searchResDlg.artistModel.append(artists[i])
            }
        }
        if (albums != null) {
            for (var j = 0; j < albums.length; j++) {
                albums[j].type = "album"
                searchResDlg.albumModel.append(albums[j])
            }
        }

        searchResDlg.visible = !((searchResDlg.songList == null || searchResDlg.songList.length == 0)
                                 && searchResDlg.artistModel.count <= 0
                                 && searchResDlg.albumModel.count <= 0)
    }

            IconButton {
                id: addBtn
                icon.name: "action_add"
                visible: !isLyricShow && titleRowLayout.showSearchEdit  // 窗口窄时隐藏
                opacity: opat
                hoverEnabled: true
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: 40

                ToolTip {
                    visible: addBtn.hovered
                    text: qsTr("Add music")
                }

                onClicked: {
                    globalVariant.globalFileDlgOpen()
                }
            }
        }
    }

    function toggleLyrics(toggle) {
        if (toggle) {
            raiseTitlebarAnimation.start()
        } else {
            hideTitlebarAnimation.start()
        }
    }

    ParallelAnimation {
        id: hideTitlebarAnimation
        NumberAnimation {
            target: titleBar
            property: "opat"
            from: 0
            to: 1
            duration: 200
        }
        NumberAnimation {
            target: titleBar
            property: "disableOpat"
            from: 0
            to: 0.4
            duration: 200
        }
    }

    ParallelAnimation {
        id: raiseTitlebarAnimation
        NumberAnimation {
            target: titleBar
            property: "opat"
            from: 1
            to: 0
            duration: 200
        }
        NumberAnimation {
            target: titleBar
            property: "disableOpat"
            from: 0.4
            to: 0
            duration: 200
        }
    }

    Connections {
        target: globalVariant
        function onShowSearchEdit() {
            if (!isLyricShow && searchEditRef) {
                searchEditRef.forceActiveFocus()
            }
        }
    }
}
