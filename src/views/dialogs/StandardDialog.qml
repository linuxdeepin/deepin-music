import QtQuick 2.3
import DMusic 1.0
import QtGraphicalEffects 1.0
import "../DMusicWidgets"

Rectangle {
    id: standardDialog
    property var centerItem

    signal closed()
    signal entered()

    height: centerItem.height + titleBar.height + bottomBar.height
    Glow {
        anchors.fill: parent
        radius: 4
        samples: radius * 2
        source: standardDialog
        color: Qt.rgba(0, 0, 0, 0.2)
        transparentBorder: true
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 1
        color: "white"
        Column {
            anchors.fill: parent
            
            Rectangle {
                id: titleBar
                width: parent.width
                height: closeButton.height
                DCloseButton{
                    id: closeButton
                    width: 25
                    height: 25

                    anchors.right: parent.right
                    anchors.top: parent.top
                    onClicked:{
                        standardDialog.closed();
                    }
                }
            }

            Item {
                width: parent.width
                height: parent.height - titleBar.height - bottomBar.height
                children:[
                    Rectangle {
                        color: "white"
                        anchors.fill: parent
                        children: [centerItem]
                    }
                ]
            }

            Rectangle {
                id: bottomBar
                width: parent.width
                height: 30

                Row {
                    id: layout
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: 10
                    spacing: 10

                    Item {
                        id: spacing
                        width: bottomBar.width - enterButton.width - cancelButton.width - 3 * layout.spacing
                        height: 20
                    }

                    DTextButton {
                        id: enterButton
                        width: 80
                        height: 20
                        text: "确定"

                        onClicked:{
                            standardDialog.entered();
                        }
                    }

                    DTextButton {
                        id: cancelButton
                        width: 80
                        height: 20
                        text: "取消"
                        onClicked:{
                            standardDialog.closed();
                        }
                    }
                }
            }
        }
    }
}