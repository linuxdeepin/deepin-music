import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"
import Deepin.Widgets 1.0

Flickable {
    id: flick

    anchors.fill: parent
    anchors.leftMargin: 25
    flickableDirection: Flickable.VerticalFlick
    contentWidth: edit.paintedWidth
    contentHeight: edit.paintedHeight
    clip: true

    function ensureVisible(r)
    {
        if (contentX >= r.x)
            contentX = r.x;
        else if (contentX+width <= r.x+r.width)
            contentX = r.x+r.width-width;
        if (contentY >= r.y)
            contentY = r.y;
        else if (contentY+height <= r.y+r.height)
            contentY = r.y+r.height-height;
    }

    TextEdit {
        id: edit
        width: flick.width - 20
        height: flick.height
        focus: true
        wrapMode: TextEdit.Wrap
        readOnly: true
        text: QmlDialog.songObj.lyric
        onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
    }
}