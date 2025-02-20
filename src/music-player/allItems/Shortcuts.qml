// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D

Item {
    id: item
    Shortcut {
        id: ctrl_F1 //帮助手册
        sequence: "F1"
        enabled: true
        autoRepeat: false
        onActivated: {
            D.ApplicationHelper.handleHelpAction()
        }
    }
    Shortcut {
        id: ctrl_Shift_H   //快捷键界面
        sequence: "Ctrl+Shift+/"
        onActivated: {
            ShortcutDialg.show(rootWindow.x+rootWindow.width/2,rootWindow.y+rootWindow.height/2)
        }
    }
    Shortcut {
        id: ctrlO   //快捷键 导入
        sequence: "Ctrl+O"
//        context: Qt.ApplicationShortcut
        onActivated: { globalVariant.globalFileDlgOpen() }
    }
    Shortcut {
        id: ctrlI   //快捷键 歌曲信息
        sequence: "Ctrl+I"
//        context: Qt.ApplicationShortcut
        onActivated: { globalVariant.musicInfoDlgShow() }
    }
    Shortcut {
        id: ctrlF   //快捷键 搜索
        sequence: "Ctrl+F"
        onActivated: { 
            globalVariant.showSearchEdit()
        }
    }
    Shortcut {
        id: ctrl_Shift_N  //快捷键 新建歌单
        sequence: "Ctrl+Shift+N"
        context: Qt.ApplicationShortcut
        enabled: item.enabled
        onActivated: {
            var tmpPlaylist = Presenter.addPlayList(qsTr("New playlist"));
            globalVariant.globalCustomPlaylistModel.onAddPlaylist(tmpPlaylist);
            globalVariant.renameNewItem();
        }
    }
    Shortcut {
        id: m  //快捷键 静音
        sequence: "M"
//        context: Qt.ApplicationShortcut
        onActivated: {
            var mute = Presenter.getMute();
            Presenter.setMute(!mute);
        }
    }
    // 不固定 快捷键
    Shortcut {
        id: play_pause  //快捷键 播放暂停
        sequence: { return Presenter.valueFromSettings("shortcuts.all.play_pause");}
//        context: Qt.ApplicationShortcut
        onActivated: {
            Presenter.playPause();
        }
    }
    Shortcut {
        id: previous  //快捷键 上一首
        sequence: { return Presenter.valueFromSettings("shortcuts.all.previous");}
//        context: Qt.ApplicationShortcut
        onActivated: {
            Presenter.playPre();
        }
    }
    Shortcut {
        id: next  //快捷键 下一首
        sequence: { return Presenter.valueFromSettings("shortcuts.all.next");}
//        context: Qt.ApplicationShortcut
        onActivated: {
            Presenter.playNext();
        }
    }
    Shortcut {
        id: volume_up  //快捷键 放大音量
        sequence: { return Presenter.valueFromSettings("shortcuts.all.volume_up");}
//        context: Qt.ApplicationShortcut
        onActivated: {
            var volume = Presenter.getVolume();
            volume = volume + 10;
            Presenter.setVolume(volume);
        }
    }
    Shortcut {
        id: volume_down  //快捷键 减小音量
        sequence: { return Presenter.valueFromSettings("shortcuts.all.volume_down");}
//        context: Qt.ApplicationShortcut
        onActivated: {
            var volume = Presenter.getVolume();
            volume = volume - 10;
            Presenter.setVolume(volume);
        }
    }
    Shortcut {
        id: favorite_song  //快捷键 我的喜欢
        sequence: { return Presenter.valueFromSettings("shortcuts.all.favorite_song");}
        onActivated: {
            if (!globalVariant.currentMediaMeta.title)
                return

            if(globalVariant.currentMediaMeta.favourite){
                Presenter.removeFromPlayList(globalVariant.currentMediaMeta.hash, "fav");
                globalVariant.currentMediaMeta.favourite = false;
                globalVariant.sendFloatingMessageBox(qsTr("My Favorites"), 2);
            }else{
                Presenter.addMetasToPlayList(globalVariant.currentMediaMeta.hash, "fav");
                globalVariant.currentMediaMeta.favourite = true;
            }
        }
    }

    function onValueChangedFromSettings(key, value){
//        console.log("======key: ", key, "======== value: ", value)
        switch (key){
        case "shortcuts.all.play_pause":
            play_pause.sequence = value;
            break;
        case "shortcuts.all.previous":
            previous.sequence = value;
            break;
        case "shortcuts.all.next":
            next.sequence = value;
            break;
        case "shortcuts.all.volume_up":
            volume_up.sequence = value;
            break;
        case "shortcuts.all.volume_down":
            volume_down.sequence = value;
            break;
        case "shortcuts.all.favorite_song":
            favorite_song.sequence = value;
            break;
        default:
            return;
        }
    }

    Component.onCompleted: {
        Presenter.valueChangedFromSettings.connect(onValueChangedFromSettings);
    }
}
