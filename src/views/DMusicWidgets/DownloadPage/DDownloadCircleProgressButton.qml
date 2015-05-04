import QtQuick 2.3
import DMusic 1.0

Rectangle {
    id: progressBar
    property bool downloading
    property var button: button
    property double dprogress: 0
    property int lineWidth: 2
    property var strokeStyle: "#31a4fa"
    property var fontSize: 7

    property var start_image: '../../../skin/svg/download.start.svg'
    property var pause_image: '../../../skin/svg/download.pause.svg'

    property var hovered_image: progressBar.downloading? pause_image: start_image
    property var pressed_image: progressBar.downloading? start_image: pause_image

    width: 28
    height: 28
    antialiasing: true

    signal switchdownloadedStatus(bool downloaded)

    Text {
        id: progressText
        anchors.centerIn: parent
        font.pointSize: parent.fontSize
        visible: false
        text: '' + parseInt(progressBar.dprogress) + '%'
    }

    Canvas {
        id: canvas

        property var centerX: progressBar.width / 2
        property var centerY: progressBar.height / 2
        property var radius: progressBar.width / 2 - progressBar.lineWidth
        anchors.fill: parent
        antialiasing: true
        onPaint:{
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ctx.strokeStyle = 'gray';
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.arc(centerX, centerY, radius, 0, Math.PI * 2, false);
            ctx.closePath();
            ctx.stroke();

            ctx.strokeStyle = progressBar.strokeStyle;
            ctx.lineWidth = progressBar.lineWidth;
            ctx.beginPath();
            ctx.arc(centerX, centerY, radius, -0.5 * Math.PI, Math.PI * (progressBar.dprogress / 100 * 2 - 0.5), false);
            ctx.stroke();
        }
    }

    onDprogressChanged:{
        canvas.requestPaint();
    }

    DSvgButton {
        id: button
        normal_opacity: 1
        normal_image: progressBar.start_image
        hovered_image: progressBar.hovered_image
        pressed_image: progressBar.pressed_image

        anchors.centerIn: parent
        width: 26
        height: 26

        onHovered: {
            if (progressBar.downloading){
                progressText.visible = false;
            }
        }

        onExited:{
            if (progressBar.downloading){
                progressText.visible = true;
            }
        }

        onReleased:{
            if (progressBar.downloading){
                progressText.visible = true;
            }
        }

        onClicked:{
            progressBar.switchdownloadedStatus(!progressBar.downloading)
        }
    }

    onDownloadingChanged:{
        if (!progressBar.downloading){
            progressText.visible = false;
            button.normal_image = progressBar.start_image;
        }else{
            progressText.visible = true;
            button.normal_image = ''
        }
    }
}
