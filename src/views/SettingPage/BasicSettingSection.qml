import QtQuick 2.3
import QtQuick.Controls 1.2

BaseSection {
    id: root
    content: Rectangle {
        anchors.fill: parent
        anchors.leftMargin: 40
        anchors.topMargin: 10
        anchors.rightMargin: 40
        anchors.bottomMargin: 40

        Column {

            spacing: 5

            Label {
                text: I18nWorker.setting_start
                font.pixelSize: 14
            }

            CheckBox {
                text: I18nWorker.setting_autoPlay
                checked: true
            }

            CheckBox {
                text: I18nWorker.setting_showDesktopLRC
                checked: true
            }

            CheckBox {
                text: I18nWorker.setting_continueLastPlayProgress
                checked: true
            }

            CheckBox {
                text: I18nWorker.setting_coverSkin
                checked: false
            }

            CheckBox {
                text: I18nWorker.setting_fade
                checked: false
            }

            Label {
                text: I18nWorker.setting_close
                font.pixelSize: 14
            }

            ExclusiveGroup { id: closeGroup }

            RadioButton {
                text: I18nWorker.setting_minimized
                checked: true
                exclusiveGroup: closeGroup
            }

            RadioButton {
                text: I18nWorker.setting_quit
                checked: false
                exclusiveGroup: closeGroup
            }

            Label {
                text: I18nWorker.setting_addtoPlaylist
                font.pixelSize: 14
            }

            ExclusiveGroup { id: playGroup }

            RadioButton {
                text: I18nWorker.setting_firstPlay
                checked: true
                exclusiveGroup: playGroup
            }

            RadioButton {
                text: I18nWorker.setting_nextPaly
                checked: false
                exclusiveGroup: playGroup
            }

            RadioButton {
                text: I18nWorker.setting_lastPlay
                checked: false
                exclusiveGroup: playGroup
            }
        }
    }
}