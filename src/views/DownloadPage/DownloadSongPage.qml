import QtQuick 2.3
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import DMusic 1.0

Rectangle {
    id: root
    anchors.fill: parent
    Rectangle {
        id: contentBox
        anchors.fill: parent
        anchors.leftMargin: 34
        anchors.rightMargin: 34
        color: "white"
        Column {
            id: layout

            anchors.fill: parent
            spacing: 14

            Rectangle {
                id: tootBar
                width: contentBox.width
                height: 20

                Row {
                    spacing: 5
                    DIconTextButton {
                        id: allDownloadButton
                        normal_image: '../../skin/icons/dark/appbar.arrow.down.png'
                        hover_image: '../../skin/icons/dark/appbar.arrow.down.png'
                        pressed_image: '../../skin/icons/light/appbar.arrow.down.png'
                        disabled_image: '../../skin/icons/dark/appbar.arrow.down.png'
                        text: '全部下载'
                        width: 100
                        height: 20
                    }

                    DIconTextButton {
                        id: allPausedButton
                        normal_image: '../../skin/icons/dark/appbar.control.play.png'
                        hover_image: '../../skin/icons/dark/appbar.control.play.png'
                        pressed_image: '../../skin/icons/light/appbar.control.play.png'
                        disabled_image: '../../skin/icons/dark/appbar.control.play.png'
                        text: '全部暂停'
                        width: 100
                        height: 20
                    }

                    DIconTextButton {
                        id: openFolderButton
                        text: '打开目录'
                        width: 72
                        height: 20
                        border.width: 0
                        iconWidth: 0
                        iconHeight: 0
                        textColor: '#929292'
                        fontSize: 8
                    }
                }
            }

            DownloadSongListView {
                id: songListView
                datamodel: DownloadSongListModel
                width: contentBox.width
                height: contentBox.height - layout.spacing - tootBar.height
            }
        }
    }

    DownloadSongPageController {
        allDownloadButton: allDownloadButton
        allPausedButton: allPausedButton
        openFolderButton: openFolderButton
        songListView: songListView
    }
}