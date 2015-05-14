import QtQuick 2.3
import QtQuick.Window 2.2
import DMusic 1.0

DMovableWindow {
    id: root
    x: 200
    y: Screen.desktopAvailableHeight - 200
    width: Screen.desktopAvailableWidth - 400
    height: 100
    color: Qt.rgba(0, 0, 0, 0)

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true
        onEntered: {
            root.color = Qt.rgba(0, 0, 0, 0.2)
        }
        onExited:{
            root.color = Qt.rgba(0, 0, 0, 0)
        }

        onPressed:{
            mouse.accepted = false
        }

        onClicked:{
            mouse.accepted = false
        }

        onDoubleClicked:{
            mouse.accepted = false
        }
    }

    Rectangle {
        id: toolBar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        height: 40
        color: 'red'
    }

    Rectangle {
        id: lrcBox
        anchors.left: parent.left
        anchors.top: toolBar.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        anchors.leftMargin: 50
        anchors.rightMargin: 50

        color: 'transparent'

        Text {
            id: lrcTextBox
            anchors.fill: parent
            font.pixelSize: 50
            color: "green"
            text: '111111111111111111111'
            // onTextChanged:{
            //     // print(text)
            //     // canvas.requestPaint();
            // }
        }

    //     // Canvas {
    //     //     id: canvas
    //     //     anchors.fill: parent
    //     //     antialiasing: true
    //     //     onPaint:{
    //     //         var ctx = getContext("2d")
    //     //         ctx.clearRect(0, 0, canvas.width, canvas.height);
    //     //         ctx.fillStyle = '#000000'; 
    //     //         ctx.font = 'normal normal 50px sans-serif';
    //     //         ctx.textBaseline = 'top';
    //     //         ctx.textAlign = 'left';
    //     //         ctx.globalAlpha = '1';
    //     //         ctx.shadowColor = '#787878';
    //     //         ctx.shadowBlur = '5';
    //     //         ctx.shadowOffsetX = '3';
    //     //         ctx.shadowOffsetY = '3';
    //     //         ctx.fillText(lrcTextBox.text, 0, 0)
    //     //         print(lrcTextBox.text)
    //     //     }
    //     // }

    //     // onDprogressChanged:{
    //     //     
    //     // }
    }



    LrcController {
        toolBar: toolBar
        lrcTextBox: lrcTextBox
        // canvas: canvas
    }

}
