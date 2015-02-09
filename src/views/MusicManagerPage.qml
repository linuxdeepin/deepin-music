import QtQuick 2.0

Rectangle {
	id: root
	property var viewID: 'MusciManagerPage'

    Text{
        anchors.centerIn: root
        text: 'Welcome, this is Music Manager Page!'
        font.family: "微软雅黑"
        font.pointSize: 20
        color: "Black"
    }
}