import QtQuick 2.3
import QtQuick.Controls 1.2
import Deepin.Widgets 1.0

BaseSection {
    id: root
    content: SectionItem {
        Column{
            spacing: 10

            DCheckBox {
                id: isShortcutEnableCheckBox
                text: I18nWorker.setting_enableKeyBindings
                checked: ConfigWorker.isShortcutEnable
                Binding {
                    target: ConfigWorker
                    property: 'isShortcutEnable'
                    value: isShortcutEnableCheckBox.checked
                }
            }

            LabelShortCutInput {
                id: shortcut_preivousInput
                label: I18nWorker.setting_preivous
                shortcut: ConfigWorker.shortcut_preivous


                Binding {
                    target: ConfigWorker
                    property: 'shortcut_preivous'
                    value: shortcut_preivousInput.shortcut
                }
            }

            LabelShortCutInput {
                id: shortcut_nextInput
                label: I18nWorker.setting_next
                shortcut: ConfigWorker.shortcut_next

                Binding {
                    target: ConfigWorker
                    property: 'shortcut_next'
                    value: shortcut_nextInput.shortcut
                }
            }

            LabelShortCutInput {
                id: shortcut_volumnIncreaseInput
                label: I18nWorker.setting_volumnIncrease
                shortcut: ConfigWorker.shortcut_volumnIncrease
                Binding {
                    target: ConfigWorker
                    property: 'shortcut_volumnIncrease'
                    value: shortcut_volumnIncreaseInput.shortcut
                }
            }

            LabelShortCutInput {
                id: shortcut_volumeDecreaseInput
                label: I18nWorker.setting_volumeDecrease
                shortcut: ConfigWorker.shortcut_volumeDecrease

                Binding {
                    target: ConfigWorker
                    property: 'shortcut_volumeDecrease'
                    value: shortcut_volumeDecreaseInput.shortcut
                }
            }

            LabelShortCutInput {
                id: shortcut_playPauseInput
                label: I18nWorker.setting_playPause
                shortcut: ConfigWorker.shortcut_playPause
                Binding {
                    target: ConfigWorker
                    property: 'shortcut_playPause'
                    value: shortcut_playPauseInput.shortcut
                }
            }

            LabelShortCutInput {
                id: shortcut_simpleFullModeInput
                label: I18nWorker.setting_simpleFullMode
                shortcut: ConfigWorker.shortcut_simpleFullMode
                Binding {
                    target: ConfigWorker
                    property: 'shortcut_simpleFullMode'
                    value: shortcut_simpleFullModeInput.shortcut
                }
            }

            LabelShortCutInput {
                id: shortcut_miniFullModeInput
                label: I18nWorker.setting_miniFullMode
                shortcut: ConfigWorker.shortcut_miniFullMode

                Binding {
                    target: ConfigWorker
                    property: 'shortcut_miniFullMode'
                    value: shortcut_miniFullModeInput.shortcut
                }
            }

            LabelShortCutInput {
                id: shortcut_hideShowWindowInput
                label: I18nWorker.setting_hideShowWindow
                shortcut: ConfigWorker.shortcut_hideShowWindow

                Binding {
                    target: ConfigWorker
                    property: 'shortcut_hideShowWindow'
                    value: shortcut_hideShowWindowInput.shortcut
                }
            }

            LabelShortCutInput {
                id: shortcut_hideShowDesktopLRCInput
                label: I18nWorker.setting_hideShowDesktopLRC
                shortcut: ConfigWorker.shortcut_hideShowDesktopLRC

                Binding {
                    target: ConfigWorker
                    property: 'shortcut_hideShowDesktopLRC'
                    value: shortcut_hideShowDesktopLRCInput.shortcut
                }
            }
        }
    }
}