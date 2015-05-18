import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets/MusicManager"

Rectangle {
    id: detailPage
    property var model
    property var type
    property var parentItem

    anchors.fill: parent

    Column {

        anchors.fill: parent
        anchors.leftMargin: 22
        anchors.rightMargin: 22
        Rectangle {
            id: backbar
            width: detailPage.width
            height: 30
            
            DBackButton {
                id: backButton
                width: 30
                height: 30

                onClicked:{
                    parentItem.clearDetailLoader()
                }
            }
        }

        Rectangle {
            id: detailHeader
            width: detailPage.width
            height: 108 + 28

            Rectangle {
                anchors.fill: parent
                anchors.bottomMargin: 28

                Row {
                    spacing: 20
                    DetailImageItem {
                        id: detailImageItem
                        source: model.cover
                    }

                    Rectangle {
                        width: detailPage.width
                        height: 108

                        MouseArea {
                            anchors.fill: parent
                            propagateComposedEvents: false
                        }

                        Text {
                            id: nameText
                            width: 300
                            height: parent.height
                            color: "#8a8a8a"
                            font.pixelSize: 30
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            text: model.name
                        }

                        Text {
                            id: artistTetx
                            x: nameText.contentWidth + 20
                            anchors.right: parent.right
                            width: parent.width - nameText.contentWidth - 20
                            height: parent.height
                            anchors.leftMargin: 12
                            color: "#8a8a8a"
                            font.pixelSize: 20
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            text: {
                                if (model.artist == undefined){
                                    return ''
                                }else{
                                    return '( ' + model.artist + ' )'
                                }
                            }
                        }

                        Text {
                            id: countTetx
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 10
                            width: nameText.contentWidth
                            color: "#8a8a8a"
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            text: '( ' + model.count + ' )'
                        }
                    }

                }
            }
        }

        Rectangle {
            width: detailPage.width - 44
            height: detailPage.height - backbar.height - detailHeader.height - 20
            
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                propagateComposedEvents: false
            }

            ListView {
                id: detailView
                anchors.fill: parent
                clip: true
                highlightMoveDuration: 1
                delegate: SongDelegate {}
                model: songListModel
                currentIndex: -1
                focus: true
                spacing: 20
                snapMode:ListView.SnapToItem
                signal playMusicByUrl(string url)
                signal menuShowed(string url)

                DScrollBar {
                    flickable: parent
                    inactiveColor: 'black'
                }

                Component.onCompleted: positionViewAtIndex(currentIndex, ListView.Center)
            }
        }
    }

    DListModel {
        id: songListModel
        pymodel: DetailSongListModel
    }

    DetailController {
        detailPage: detailPage
        detailImageItem: detailImageItem
        detailView: detailView
        songListModel: songListModel
    }
}
