import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Controls 1.3
import QtGraphicalEffects 1.0
 
Window {
    id: tooltip
    
    property var target: parent
    property alias text: tooltipText.text
    property alias textItem: tooltipText

    flags: Qt.FramelessWindowHint | Qt.Popup | Qt.WindowStaysOnTopHint

    width: tooltipText.width + 20
    height: tooltipText.height + 10
    color: "#dd000000"
   
    Text {
        id: tooltipText
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        color: "white"
        font.pointSize: 10
        font.bold: true
    }
}
