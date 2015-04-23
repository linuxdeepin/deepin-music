import QtQuick 2.3

Rectangle {
    width: 600
    height: 400
    color: "white"

    Rectangle {
        id: progressBar
        property double progress: 0
        property int lineWidth: 2
        property color strokeStyle: "green"
        property var fontSize: 12
        x: 100
        y: 100
        width: 28
        height: 28
        antialiasing: true

        Text {
            anchors.centerIn: parent
            font.pointSize: 7
            text: '' + parseInt(progressBar.progress* 100) + '%'
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
                ctx.arc(centerX, centerY, radius, -0.5 * Math.PI, Math.PI * (progressBar.progress * 2 - 0.5), false);
                ctx.stroke();
                ctx.restore();
            }
        }

        onProgressChanged:{
            canvas.requestPaint();
        }
    }

    Timer {
        interval: 100
        repeat: true
        running:true
        onTriggered:{
            progressBar.progress += 0.01
        }
    }

    // Component.onCompleted: {
    //     mycanvas.requestPaint()
    // }
}

