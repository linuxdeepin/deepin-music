import QtQuick 2.3

Rectangle {
    id: progressBar
    property double dprogress: 0
    property int lineWidth: 2
    property var strokeStyle: "#31a4fa"
    property var fontSize: 7
    width: 28
    height: 28
    antialiasing: true

    Text {
        anchors.centerIn: parent
        font.pointSize: parent.fontSize
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
            ctx.strokeStyle = 'lightgray';
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
}