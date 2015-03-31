import QtQuick 2.4
import "../DMusicWidgets"

Rectangle {
    id: rootWindow

    Column {
        id: layout

        property var magrin: 8

        anchors.fill: parent
        anchors.leftMargin: layout.magrin
        anchors.rightMargin: layout.magrin
        spacing: 12

        Rectangle {
            id: navigationBar
            width: 108 + 28
            height: 20

            Rectangle {
                anchors.leftMargin: 28
                anchors.fill: parent

                DText {
                    width: 80

                    height: parent.height
                    color: "#252525"
                    font.pixelSize: 12
                    text: catgoryCombox.currentText
                }

                DUpDownButton {
                    anchors.centerIn: parent
                    width: 16
                    height: 16
                    switchflag: catgoryCombox.visible
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked :{
                        catgoryCombox.visible = !catgoryCombox.visible;
                    }
                }
            }
        }

        Loader {
            id: musicManagerLoader
            height: rootWindow.height - navigationBar.height - layout.spacing
            width: rootWindow.width - layout.magrin * 2
        }
    }

    Rectangle {
        id: noMusicTip
        anchors.fill: parent
        visible: false
        Text {
            id: linkTipText
            anchors.centerIn: parent
            color: "#535353"
            linkColor: "#31a4fa"
            font.pixelSize: 14
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: "到 <a href=\"Online\" style=\"text-decoration:none;\">网络音乐</a> 和<a href=\"Local\" style=\"text-decoration:none;\">本地乐库</a>添加你喜欢的音乐吧"
        }
    }

    MusicCatgoryComBox {
        id: catgoryCombox
        x: 36
        y: layout.spacing + navigationBar.height - 5
        visible: false
    }

    MusicManagerController {
        id: musicManagerController
        rootWindow: rootWindow
        catgoryCombox: catgoryCombox
        musicManagerLoader: musicManagerLoader
        noMusicTip: noMusicTip
        linkTipText: linkTipText
    }
}