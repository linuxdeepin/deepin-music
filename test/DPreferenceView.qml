import QtQuick 2.3
import QtQml.Models 2.1

Rectangle {
    id: root
    width: 800; height: 600

    ListModel {
        id: navgationModel
        ListElement {
            name: "基础设置"
        }
        ListElement {
            name: "快捷键"
        }
        ListElement {
            name: "桌面歌词"
        }
        ListElement {
            name: "下载"
        }
        ListElement {
            name: "关于"
        }
    }

    ObjectModel {
        id: itemModel

        Rectangle {
            property var sectionId: 'section1'
            width: col.width
            height: 400
            color: "#111111"
        }

        Rectangle {
            property var sectionId: 'section2'
            width: col.width
            height: 200
            color: "#222222"
        }

        Rectangle {
            property var sectionId: 'section3'
            width: col.width
            height: 300
            color: "#333333"
        }

        Rectangle {
            property var sectionId: 'section4'
            width: col.width
            height: 500
            color: "#444444"
        }

        Rectangle {
            property var sectionId: 'section5'
            width: col.width
            height: root.height
            color: "#555555"
        }
    }

    Row {
        id: layout
        anchors.fill: parent
        anchors.margins: 20
        Rectangle {
            id: navgationBar
            width: 120
            height: layout.height
            Component{
                id: delegateItem
                Rectangle {
                    id: delegateRect
                    width: parent.width
                    height: 24
                    Text {
                        id: navText
                        anchors.fill: parent
                        anchors.leftMargin: 24
                        color: "#3a3a3a"
                        horizontalAlignment: Text.AlignLeft
                        text: name
                    }
                    states: [
                        State {
                            name: "Current"
                            when: delegateRect.ListView.isCurrentItem
                            PropertyChanges { target: navText; color: "#4ba3fb"}
                        }
                    ]
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            flickable.currentIndex  = index
                        }
                    }
                }
            }

            ListView {
                id: navgationListView
                width: navgationBar.width
                height: navgationListView.count * 50
                
                snapMode: ListView.SnapOneItem
                highlightMoveDuration: 1
                currentIndex: flickable.currentIndex
                focus: true
                model: navgationModel
                delegate: delegateItem
            }
        }


        Flickable {
            id: flickable
            property int currentIndex
            property ObjectModel objectModel: itemModel
            width: layout.width - navgationBar.width
            height: layout.height
            contentWidth: col.childrenRect.width
            contentHeight: col.childrenRect.height
            flickableDirection: Flickable.VerticalFlick
            clip: true

            Behavior on contentY {
                NumberAnimation { easing.type: Easing.InOutCubic; duration: 300 }
            }

            function scrollbyIndex(index) {
                var visibleChildren = col.visibleChildren
                contentY = visibleChildren[index].y
            }

            onCurrentIndexChanged: scrollbyIndex(currentIndex)

            onContentYChanged: {
                if (!flicking) return

                if(atYEnd) {
                    flickable.currentIndex = col.visibleChildren.length - 1;
                } else {
                    var currentTopItem = col.childAt(50, contentY)
                    var visibleChildren = col.visibleChildren
                    for (var i = 0; i < visibleChildren.length; i++) {
                        if (visibleChildren[i] === currentTopItem) {
                            flickable.currentIndex = i;
                        }
                    }
                }
            }

            Column {
                id: col
                width: flickable.width
                height: flickable.height
            }

            onObjectModelChanged: {
                col.children = objectModel.children
            }
        }
    }
}