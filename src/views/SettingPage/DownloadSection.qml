import QtQuick 2.3
import QtQuick.Controls 1.2
import Deepin.Widgets 1.0

BaseSection {
    id: root
    content: SectionItem {
        Column {
            spacing: 5

            LabelFileChoseInput{
                label: I18nWorker.setting_downloadFolder
                text: ConfigWorker.DownloadSongPath

                onFileChooseClicked:{
                    fielChooseDialog.visible = true
                }
            }
        }
    }
}
