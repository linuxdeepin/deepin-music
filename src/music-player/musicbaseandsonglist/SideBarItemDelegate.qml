// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
import QtQml.Models 2.11
import org.deepin.dtk 1.0


ItemDelegate {
    property string type: "library"
    id: item
    // 屏蔽空格响应
    Keys.onSpacePressed: { event.accepted=false; }
    Keys.onReleased: { event.accepted=(event.key===Qt.Key_Space); }

    Drag.active: mouseArea.drag.active
    Drag.supportedActions: Qt.CopyAction
    Drag.dragType: Drag.Automatic
    Drag.hotSpot.x: -25
    Drag.hotSpot.y: -25
    Drag.mimeData: {
        "index": index,
        "uuid": uuid
    }

    MouseArea {
        id: mouseArea
        anchors.fill: item
        acceptedButtons: Qt.RightButton | Qt.LeftButton
        drag.target: type === "library" || index === 0  ||
                     (index === 1 && uuid === "cdarole") ? null : item

        onPressed: {
            dragItem.grabToImage(function(result) {
                parent.Drag.imageSource = result.url
            })
        }
        onReleased: {
            if(parent.Drag.supportedActions === Qt.CopyAction){
                item.x = 0;
                item.y = index * item.height;
                sideModel.setProperty(index, "dragFlag", false)
            }
        }

        onDoubleClicked:{
            item.rename();
        }
        onClicked: {
            sideListView.currentIndex = index;
            item.forceActiveFocus();
            if(mouse.button === Qt.RightButton){
                control.itemRightClicked(model.uuid, model.displayName);
            }
            control.itemClicked(model.uuid, model.displayName);
            item.checked = true;
        }
        onWheel: {
            wheel.accepted = false
        }
    }
    DciIcon {
        id: siderIcon
        anchors.left: item.left; anchors.leftMargin: 10
        anchors.verticalCenter: item.verticalCenter
        name: item.checked ? model.icon_checked : model.icon
        sourceSize: Qt.size(20, 20)
        palette: DTK.makeIconPalette(item.palette)
    }
    Label {
        id: songName
        width: 154;
        anchors.left: siderIcon.right; anchors.leftMargin: 10
        anchors.verticalCenter: item.verticalCenter
        text: "%1".arg(model.displayName)
        textFormat: Text.PlainText
        elide: Text.ElideRight
    }
    LineEdit {
        id: keyLineEdit
        width: item.width - 8
        anchors {
            left: item.left
            leftMargin: 4
            verticalCenter: item.verticalCenter
        }
        visible: false
        maximumLength: 30

        Keys.onEscapePressed: {
            item.checked = true;
            songName.visible = true;
            siderIcon.visible = true;
            keyLineEdit.visible = false;
            keyLineEdit.text = model.displayName
        }

        onEditingFinished: {
            item.checked = true;
            songName.visible = true;
            siderIcon.visible = true;
            keyLineEdit.visible = false;
            if(keyLineEdit.text !== "" && Presenter.renamePlaylist(keyLineEdit.text, model.uuid)){
                songName.text = keyLineEdit.text;
            }
        }
        onActiveFocusChanged: {
            EventsFilter.setEnabled(!activeFocus)
        }
        function onMousePressed(x, y) {
            if (!keyLineEdit)
                return
            var object = keyLineEdit.mapFromGlobal(x, y)
            if (!keyLineEdit.contains(object)) {
                keyLineEdit.focus = false
            }
        }
        Component.onCompleted: {
            EventsFilter.mousePress.connect(onMousePressed)
        }
    }

    Rectangle {
        id: bottomDivider
        width: parent.width
        height: 1
        y: parent.height
        color: palette.highlight
        visible: dragFlag
    }

    Rectangle {
        id: dragItem
        width: txt.width + 10
        height: txt.height
        radius: 5
        visible: false

        Text {
            id: txt
            anchors.centerIn: parent
            font: DTK.fontManager.t8
            text: model.displayName
        }
    }

    function enableRename(){
        control.itemClicked(model.uuid, model.displayName); //新建歌单后，自动切换到新歌单列表
        item.checked = true;
        keyLineEdit.text = songName.text;
        keyLineEdit.forceActiveFocus()
        songName.visible = false;
        siderIcon.visible = false;
        keyLineEdit.visible = true;
        item.checked = false;
    }
    function rename(){
        if(!model.editable)
            return;
        enableRename();
        keyLineEdit.selectAll();
    }
    function switchToPrevious(){
        item.forceActiveFocus();
        control.itemClicked(model.uuid, model.displayName);
        item.checked = true
    }
}
