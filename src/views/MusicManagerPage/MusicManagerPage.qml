import QtQuick 2.3
import "../DMusicWidgets"

Rectangle {
    id: rootWindow

    signal clearDetailLoader()

    Column {
        id: layout

        property var magrin: 8

        anchors.fill: parent
        anchors.leftMargin: layout.magrin
        anchors.rightMargin: layout.magrin
        spacing: 12

        Rectangle {
            id: rowBox
            width: layout.width - 2 * layout.magrin
            height: 20

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

            Rectangle {
                x: 100
                y: 0
                width: rowBox.width - x
                height: rowBox.height

                Text {
                    id: statusText
                    anchors.fill: parent
                    color: "#535353"
                    linkColor: "#31a4fa"
                    font.pixelSize: 14
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
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
        anchors.right: rootWindow.right
        anchors.bottom: rootWindow.bottom
        width: 500
        height: 20
        color: "#eae4e4"
        visible: false
        Text {
            id: scanStatusText
            anchors.fill: parent
            anchors.leftMargin: 5
            anchors.rightMargin: 20
            color: "#535353"
            linkColor: "#31a4fa"
            font.pixelSize: 14
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter

            onTextChanged:{
                if (text == ""){
                    parent.visible = false;
                }else{
                    parent.visible = true;
                }
            }
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
            text: "到 <a href=\"SearchAllDriver\" style=\"text-decoration:none;\">扫描全盘</a> 和<a href=\"SearchOneFolder\" style=\"text-decoration:none;\">指定目录</a>添加你喜欢的音乐吧"
        }
    }

    MusicCatgoryComBox {
        id: catgoryCombox
        x: 36
        y: layout.spacing + navigationBar.height - 5
        visible: false
    }


    Loader {
        id: detailLoader
        anchors.fill: parent
    }

    MusicManagerController {
        id: musicManagerController
        rootWindow: rootWindow
        catgoryCombox: catgoryCombox
        statusText: statusText
        scanStatusText: scanStatusText
        musicManagerLoader: musicManagerLoader
        detailLoader: detailLoader
        noMusicTip: noMusicTip
        linkTipText: linkTipText

    }
}