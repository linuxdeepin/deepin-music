import QtQuick 2.3
import QtQuick.Controls 1.2
import Deepin.Widgets 1.0

BaseSection {
    id: root
    content: Rectangle {
        anchors.fill: parent
        anchors.leftMargin: 40
        anchors.topMargin: 10
        anchors.rightMargin: 40
        anchors.bottomMargin: 40
        Column{
            spacing: 5

            CheckBox {
                text: I18nWorker.setting_enableKeyBindings
                checked: true
            }

            LabelInput {
                labelText: I18nWorker.setting_last
            }

        }
    }
}