import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0
 
Window {
    id: popupWindow
    
    property  string tooltip
    property alias textItem: tooltipText

    flags: Qt.FramelessWindowHint | Qt.Dialog | Qt.WindowStaysOnTopHint

    width: tooltipText.contentWidth
    height: tooltipText.contentHeight
    color: "transparent"
   
    Text {
        id: tooltipText
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        color: "white"
        font.pointSize: 10
        // font.bold: true
        text: tooltip
    }
}
