import QtQuick 2.3
import QtQuick.Controls 1.2
import Deepin.Widgets 1.0

BaseSection {
    id: root
    content: SectionItem {
        Column {

            spacing: 10

            Label {
                text: I18nWorker.setting_start
                font.pixelSize: 14
            }

            DCheckBox {
                id: isAutoPlayCheckBox
                text: I18nWorker.setting_autoPlay
                checked: ConfigWorker.isAutoPlay
                Binding {
                    target: ConfigWorker
                    property: 'isAutoPlay'
                    value: isAutoPlayCheckBox.checked
                }
            }

            DCheckBox {
                id: isDesktopLrcShowCheckBox
                text: I18nWorker.setting_showDesktopLRC
                checked: ConfigWorker.isDesktopLrcShow
                Binding {
                    target: ConfigWorker
                    property: 'isDesktopLrcShow'
                    value: isDesktopLrcShowCheckBox.checked
                }
            }

            DCheckBox {
                id: isContinueLastPlayProgressCheckBox
                text: I18nWorker.setting_continueLastPlayProgress
                checked: ConfigWorker.isContinueLastPlayProgress
                Binding {
                    target: ConfigWorker
                    property: 'isContinueLastPlayProgress'
                    value: isContinueLastPlayProgressCheckBox.checked
                }
            }

            DCheckBox {
                id: isCoverBackgroundCheckBox
                text: I18nWorker.setting_coverSkin
                checked: ConfigWorker.isCoverBackground

                Binding {
                    target: ConfigWorker
                    property: 'isCoverBackground'
                    value: isCoverBackgroundCheckBox.checked
                }
            }

            DCheckBox {
                id: isFadeCheckBox
                text: I18nWorker.setting_fade
                checked: ConfigWorker.isFade

                Binding {
                    target: ConfigWorker
                    property: 'isFade'
                    value: isFadeCheckBox.checked
                }
            }

            Label {
                text: I18nWorker.setting_close
                font.pixelSize: 14
            }

            ExclusiveGroup { 
                id: closeGroup
                onCurrentChanged:{
                    if (current === minimizedRadio){
                        ConfigWorker.isExitedWhenClosed = 0;
                    }else if( current == quitRadio){
                        ConfigWorker.isExitedWhenClosed = 1;
                    }
                } 
            }

            DRadio {
                id: minimizedRadio
                text: I18nWorker.setting_minimized
                checked: {
                    if (ConfigWorker.isExitedWhenClosed == 0){
                        return true
                    }else{
                        return false
                    }
                }
                exclusiveGroup: closeGroup
            }

            DRadio {
                id: quitRadio
                text: I18nWorker.setting_quit
                checked: {
                    if (ConfigWorker.isExitedWhenClosed == 1){
                        return true
                    }else{
                        return false
                    }
                }
                exclusiveGroup: closeGroup
            }

            Label {
                text: I18nWorker.setting_addtoPlaylist
                font.pixelSize: 14
            }

            ExclusiveGroup {
                id: playGroup
                onCurrentChanged:{
                    if (current === firstPlayRadio){
                        ConfigWorker.addSongToPlaylistPlayMode = 0;
                    }else if( current == nextPalyRadio){
                        ConfigWorker.addSongToPlaylistPlayMode = 1;
                    }else if( current == lastPlayRadio){
                        ConfigWorker.addSongToPlaylistPlayMode = 2;
                    }
                } 
            }

            DRadio {
                id: firstPlayRadio
                text: I18nWorker.setting_firstPlay
                checked: {
                    if (ConfigWorker.addSongToPlaylistPlayMode == 0){
                        return true
                    }else{
                        return false
                    }
                }
                exclusiveGroup: playGroup
            }

            DRadio {
                id: nextPalyRadio
                text: I18nWorker.setting_nextPaly
                checked: {
                    if (ConfigWorker.addSongToPlaylistPlayMode == 1){
                        return true
                    }else{
                        return false
                    }
                }
                exclusiveGroup: playGroup
            }

            DRadio {
                id: lastPlayRadio
                text: I18nWorker.setting_lastPlay
                checked: {
                    if (ConfigWorker.addSongToPlaylistPlayMode == 2){
                        return true
                    }else{
                        return false
                    }
                }
                exclusiveGroup: playGroup
            }
        }
    }
}