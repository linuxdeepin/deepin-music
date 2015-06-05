import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {
    id: section

    property alias sectionTitle: titleText.text
    property var contentItem
    property int headerHeight: 46
    property int expandHeight: 250

    width: parent.width

    state: 'expanded'

    Column {
    	id: layout
        anchors.fill: parent
        anchors.margins: 20
        spacing: 5

        Rectangle {
        	id: lineHeader
            width: parent.width
            height:1
            color: "lightgray"
        }

        DText {
        	id: titleText
            width: parent.width
            height: 20
            font.pixelSize: 12
            color: "black"
            horizontalAlignment: Text.AlignLeft
            text: QmlDialog.songObj.title
        }

        Rectangle{
            id: baseinfoRect
            width: parent.width
            height: expandHeight - layout.anchors.margins * 2 - lineHeader.height - titleText.height
            children: [contentItem]
            Behavior on height { NumberAnimation {} }
        }
    }

    DUpDownButton {
        id: expandButton
        width: 25
        height: 25
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 25
        visible:  {
        	if (section.state == 'closed'){
                return true;
            }else{
                return false;
            }
        }
        onClicked:{
            if (section.state == 'closed'){
                section.state = 'expanded';
            }else{
                section.state = 'closed';
            }
        }
    }

    states:[
        State {
            name: "closed"
            PropertyChanges { target: section; height: headerHeight}
        },
        State {
            name: "expanded"
            PropertyChanges { target: section; height: expandHeight}
        }
    ]
}