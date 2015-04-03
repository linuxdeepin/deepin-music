import QtQuick 2.4
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: item

    property var cover
    property var name
    property var count

    property var contentWidth: 108

    width: item.contentWidth + 56
    height: item.contentWidth + 12 + 12 + 18

    Rectangle {
        anchors.fill: parent
        anchors.leftMargin: 28
        anchors.rightMargin: 28
        Column {

            Item {
                width: item.contentWidth
                height: item.contentWidth

                Image {
                    asynchronous: true
                    width: item.contentWidth
                    height: item.contentWidth
                    source: item.cover
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons:  Qt.LeftButton | Qt.RightButton
                    propagateComposedEvents: true
                    hoverEnabled: true
                    onEntered: {
                        playButton.visible = true;
                    }
                    onExited: {
                        playButton.visible = false;
                    }
                    onClicked: {
                        if (mouse.button == Qt.LeftButton){
                            item.GridView.view.clicked(item.name);
                            mouse.accpted = true
                        }else if (mouse.button == Qt.RightButton){
                            item.GridView.view.rightClicked(item.name);
                            mouse.accpted = true
                        }
                    }

                    onPressed:{
                        mouse.accepted = true;
                    }

                    onDoubleClicked:{
                        mouse.accepted = false;
                    }

                    onPositionChanged:{
                        mouse.accepted = false;
                    }
                }

                DPlayButton {
                    id: playButton
                    anchors.centerIn: parent
                    width: 40
                    height: 40
                    visible: false
                    onHovered:{
                        visible = true;
                    }
                    onClicked:{
                        item.GridView.view.play(item.name);
                    }

                    
                }
            }
            

           
            Item {
                width: item.contentWidth
                height: 10
            }

            DText {
                width: item.contentWidth
                height: 12
                color: "#252525"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                text: item.name
            }
            Item {
                width: item.contentWidth
                height: 8
            }
            DText {
                width: item.contentWidth
                height: 12
                color: "#888888"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                text: item.count
            }
        }
    }
}
