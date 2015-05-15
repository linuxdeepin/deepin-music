import QtQuick 2.3
import QtQuick.Window 2.2
import DMusic 1.0
import "../DMusicWidgets/LrcWindow"

DMovableWindow {
    id: root
    property bool hovered: false
    x: 200
    y: Screen.desktopAvailableHeight - 200
    width: Screen.desktopAvailableWidth - 400
    height: 100
    // color: root.hovered? Qt.rgba(0, 0, 0, 0.2) : Qt.rgba(0, 0, 0, 0)

    // MouseArea {
    //     anchors.fill: parent
    //     hoverEnabled: true
    //     propagateComposedEvents: true

    //     onPressed:{
    //         mouse.accepted = false
    //     }

    //     onClicked:{
    //         mouse.accepted = false
    //     }

    //     onDoubleClicked:{
    //         mouse.accepted = false
    //     }

    //     onPositionChanged:{
    //         mouse.accepted = false
    //     }
    // }

    Rectangle {
        id: toolBar
        
        property int largerHeight: 24
        property int normalHeight: 16

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 18
        height: anchors.topMargin + toolBar.lagerHeight

        // Row {
        //     id: layout
        //     width: 436
        //     anchors.centerIn: parent
        //     spacing: 14
        //     Row {
        //         id: playLayout
        //         spacing: 10
        //         Rectangle {
        //             width: toolBar.normalHeight
        //             height: toolBar.largerHeight
        //             color: 'transparent'
        //             DLrcPreButton{
        //                 anchors.centerIn: parent
        //                 width: toolBar.normalHeight
        //                 height: toolBar.normalHeight
        //             }
        //         }

        //         DLrcPlayButton {
        //             width: toolBar.largerHeight
        //             height: toolBar.largerHeight
        //         }
        //         Rectangle {
        //             width: toolBar.normalHeight
        //             height: toolBar.largerHeight
        //             color: 'transparent'
        //             DLrcNextButton{
        //                 anchors.centerIn: parent
        //                 width: toolBar.normalHeight
        //                 height: toolBar.normalHeight
        //             }
        //         }
        //     }

        //     Rectangle {

        //         width: layout.width - playLayout.width - 14
        //         height:toolBar.largerHeight
        //         color: "transparent"
        //         Row {
        //             height: toolBar.normalHeight
        //             anchors.verticalCenter: parent.verticalCenter
        //             spacing: 14
        //             DLrcFontPlusButton {
        //                 width: toolBar.normalHeight
        //                 height: toolBar.normalHeight
        //             }

        //             DLrcFontMinusButton {
        //                 width: toolBar.normalHeight
        //                 height: toolBar.normalHeight
        //             }

        //             DLrcBackButton{
        //                 width: toolBar.normalHeight
        //                 height: toolBar.normalHeight
        //             }

        //             DLrcForwordButton{
        //                 width: toolBar.normalHeight
        //                 height: toolBar.normalHeight
        //             }

        //             DLrcThemeButton{
        //                 width: toolBar.normalHeight
        //                 height: toolBar.normalHeight
        //             }
        //             DLrcSingleLineButton{
        //                 width: toolBar.normalHeight
        //                 height: toolBar.normalHeight
        //             }

        //             DLrcDoubleLineButton{
        //                 width: toolBar.normalHeight
        //                 height: toolBar.normalHeight
        //             }

        //             DLrcKalaokButton{
        //                 width: toolBar.normalHeight
        //                 height: toolBar.normalHeight
        //             }

        //             DLrcLockButton{
        //                 width: toolBar.normalHeight
        //                 height: toolBar.normalHeight
        //             }

        //             DLrcSettingButton{
        //                 width: toolBar.normalHeight
        //                 height: toolBar.normalHeight
        //             }
        //             DLrcSearchButton{
        //                 width: toolBar.normalHeight
        //                 height: toolBar.normalHeight
        //             }
        //             DLrcCloseButton{
        //                 width: toolBar.normalHeight
        //                 height: toolBar.normalHeight
        //             }
        //         }
        //     }

            
        // }
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
            anchors.centerIn: parent
            font.pixelSize: 30
            color: "green"
            text: '111111111111111111111'
            // onTextChanged:{
            //     // print(text)
            //     // canvas.requestPaint();
            // }

            // MouseArea {
            //     anchors.fill: parent
            //     hoverEnabled: true
            //     propagateComposedEvents: true
            //     onEntered: {
            //         print('+++++++++++')
            //         root.hovered = true
            //     }
            //     onExited:{
            //         root.hovered = false
            //     }

            //     onPressed:{
            //         mouse.accepted = false
            //     }

            //     onClicked:{
            //         mouse.accepted = false
            //     }

            //     onDoubleClicked:{
            //         mouse.accepted = false
            //     }

            //     onPositionChanged:{
            //         mouse.accepted = false
            //     }
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
