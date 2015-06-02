import QtQuick 2.3
import QtQuick.Controls 1.2


BaseSection {
    id: root

    content: SectionItem {
        Column {
            spacing: 5

            LabelComboBox {
                id: fontTypeCombox
                label: I18nWorker.setting_fontType
                items: ConfigWorker.fontType
                index: ConfigWorker.desktopLRC_fontType

                Binding {
                    target: ConfigWorker
                    property: 'desktopLRC_fontType'
                    value: fontTypeCombox.index
                }
            }

            LabelSpinner {
                id: fontSizeSpinner
                label: I18nWorker.setting_fontSize
                initValue: ConfigWorker.desktopLRC_fontSize
                min: ConfigWorker.desktopLRC_fontSize_minValue
                max: ConfigWorker.desktopLRC_fontSize_maxValue

                onValueChanged: {
                    ConfigWorker.desktopLRC_fontSize = value;
                }
            }

            LabelComboBox {
                id: fontItalicCombox
                label: I18nWorker.setting_fontItalic
                items: ConfigWorker.fontItalic
                index: ConfigWorker.desktopLRC_fontItalic
                Binding {
                    target: ConfigWorker
                    property: 'desktopLRC_fontItalic'
                    value: fontItalicCombox.index
                }
            }

            LabelComboBox {
                id: lineNumberCombox
                label: I18nWorker.setting_lineNumber
                items: ConfigWorker.lineNumber
                index: ConfigWorker.desktopLRC_lineNumber
                Binding {
                    target: ConfigWorker
                    property: 'desktopLRC_lineNumber'
                    value: lineNumberCombox.index
                }
            }

            LabelComboBox {
                id: fontAlignmentCombox
                label: I18nWorker.setting_fontAlignment
                items: ConfigWorker.fontAlignment
                index: ConfigWorker.desktopLRC_fontAlignment
                Binding {
                    target: ConfigWorker
                    property: 'desktopLRC_fontAlignment'
                    value: fontAlignmentCombox.index
                }
            }

            LabelComboBox {
                id: background1Combox
                label: I18nWorker.setting_background1
                items: ConfigWorker.background1
                index: ConfigWorker.desktopLRC_background1
                Binding {
                    target: ConfigWorker
                    property: 'desktopLRC_background1'
                    value: background1Combox.index
                }
            }

            LabelComboBox {
                id: background2Combox
                label: I18nWorker.setting_background2
                items: ConfigWorker.background2
                index: ConfigWorker.desktopLRC_background2
                Binding {
                    target: ConfigWorker
                    property: 'desktopLRC_background2'
                    value: background2Combox.index
                }
            }

            LabelSpinner {
                id: backgroundSpinner
                label: I18nWorker.setting_backgroundSize
                initValue: ConfigWorker.desktopLRC_backgroundSize
                min: ConfigWorker.desktopLRC_backgroundSize_minValue
                max: ConfigWorker.desktopLRC_backgroundSize_maxValue

                onValueChanged: {
                    ConfigWorker.desktopLRC_backgroundSize = value;
                }
            }
        }
    }
}