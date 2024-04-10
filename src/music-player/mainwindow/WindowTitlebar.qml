// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0
import "../dialogs"

TitleBar {
    signal lrcHideBtnClicked()
    signal searchItemTriggered(string text, int type)

    Loader { id: equalizerDlgLoader }
    Loader { id: settingDlgLoader }

    id: titleBar
    icon.name: isLyricShow ? "" : globalVariant.appIconName
    hoverEnabled: false
    DciIcon {
        name: ""
        visible: isLyricShow
        sourceSize: Qt.size(12, 12)
        x: 20
        y: 26
        ActionButton {
        anchors.fill: parent
        icon.name: "go_down"
        icon.width: parent.width
        icon.height: parent.height
        hoverEnabled: false;
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                lrcHideBtnClicked()
            }
        }
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
                width: 360; height:362
                productName: qsTr("Music")
                productIcon: globalVariant.appIconName
                description: qsTr("Music is a local music player with beautiful design and simple functions.")
                version: qsTr("Version:") + "%1".arg(Qt.application.version)
                companyLogo: globalVariant.appIconName
                websiteName: DTK.deepinWebsiteName
                websiteLink: DTK.deepinWebsiteLink
                license: qsTr("%1 is released under %2").arg("Music").arg("GPLV3")
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

    Component {
        id: titleBarContent

        RowLayout {
            width: parent.width - 20
            anchors {
                left: parent.left
                leftMargin: 30
            }

            RowLayout {
                spacing: 10
                IconButton {
                    id: preBtn
                    icon.name: "arrow_ordinary_left"
                    visible: !isLyricShow
                    enabled: (globalVariant.globalSwitchButtonStatus === 2) ? true: false
                    hoverEnabled: true
                    onClicked: {
                        globalVariant.globalSwitchButtonStatus = 1; //切换到上一级页面
                    }

                    ToolTip {
                        visible: preBtn.hovered
                        text: qsTr("Previous page")
                    }
                }
                IconButton {
                    id: nextBtn
                    icon.name: "arrow_ordinary_right"
                    visible: !isLyricShow
                    enabled: (globalVariant.globalSwitchButtonStatus === 1) ? true: false
                    hoverEnabled: true
                    onClicked: {globalVariant.globalSwitchButtonStatus = 2;} //切换到下一级页面

                    ToolTip {
                        visible: nextBtn.hovered
                        text: qsTr("Next page")
                    }
                }
            }
            SearchEdit {
                id: searchEdit
                visible: !isLyricShow
                Layout.preferredWidth: 300
                Layout.alignment: Qt.AlignCenter
                placeholder: qsTr("Search")
                Keys.onReturnPressed: {
                    //console.log("SearchEdit: Keys.onEnterPressed....")
                    if (text.length <= 0 || searchResDlg.songList == null)
                        return

                    var type = -1
                    if (searchResDlg.songList.length > 0)
                        type = 0
                    else if (searchResDlg.artistModel.count > 0)
                        type = 1
                    else if (searchResDlg.albumModel.count > 0)
                        type = 2

                    searchResDlg.searchItemTriggered(text, type)
                    //searchResDlg.visible = false
                }

                onTextChanged: {
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

                    if ((searchResDlg.songList == null || searchResDlg.songList.length == 0)
                            && searchResDlg.artistModel.count <= 0 && searchResDlg.albumModel.count <= 0) {
                        searchResDlg.visible = false
                    } else {
                        searchResDlg.visible = true
                    }
                }
                onActiveFocusChanged: {
                    EventsFilter.setEnabled(!activeFocus)
                }
                function onMousePressed(x, y) {
                    var object = searchEdit.mapFromGlobal(x, y)
                    if (!searchEdit.contains(object)) {
                        searchEdit.focus = false
                    }
                }

                Component.onCompleted: {
                    EventsFilter.mousePress.connect(onMousePressed)
                }
            }
            SearchResultDialog {
                id: searchResDlg
                width: 360
                x: searchEdit.x - (width - searchEdit.width) / 2
                y: 50

                visible: false
                onSearchItemTriggered: {
                    titleBar.searchItemTriggered(value, type)
                    searchEdit.text = value
                    visible = false
                    //searchEdit.updateSearchText(value)
                }
            }

            IconButton {
                id: addBtn
                icon.name: "action_add"
                visible: !isLyricShow
                hoverEnabled: true
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: 20

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
}
