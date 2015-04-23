import QtQuick 2.3

Rectangle {
    id: progressBar
    property double dprogress
    property int lineWidth: 2
    property color strokeStyle: "#31a4fa"
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
        anchors.fill: parent
        antialiasing: true
        onPaint:{
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            var centerX = progressBar.width / 2;
            var centerY = progressBar.height / 2;
            var radius = progressBar.width / 2 - progressBar.lineWidth;
            ctx.lineWidth = progressBar.lineWidth;
            ctx.strokeStyle = progressBar.strokeStyle;
            ctx.beginPath();
            ctx.arc(centerX, centerY, radius, -0.5 * Math.PI, Math.PI * (progressBar.dprogress / 100 * 2 - 0.5), false);
            ctx.stroke();
            ctx.restore();
        }
    }

    onDprogressChanged:{
        canvas.requestPaint();
    }
}