// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtGraphicalEffects 1.0
import org.deepin.dtk 1.0
import "../toolbar"
import audio.global 1.0
import "../allItems"

FloatingPanel {
    signal lyricToggleClicked()
    signal playlistBtnClicked()

    property var mediaData: globalVariant.currentMediaMeta
    property string imgPath: "qrc:/dsg/img/no_music.svg"
    property string songTitle: ""
    property string artistStr: ""
    property string  totalTime: "0:00"
    property string  currentTime: "0:00"
    property int minute: 0
    property int second: 0
    property int curMinute: 0
    property int curSecond: 0
    property bool favorite: false
    property bool bMute: false

    property int coverRectWidth: 40
    property int infoRectWidth: 142
    property int playControlRectWidth: 220
    property int rightAreaRectWidth: 228
    property int contentSpacing: 8
    property int leftPaddingWidth: 20
    property int rightPaddingWidth: 10

    property bool isVolSliderShow: false

    property int playMode: Presenter.getPlaybackMode()
    property var modeIcon: ["toolbar_music_sequence", "toolbar_music_repeat", "toolbar_music_repeatcycle", "toolbar_music_shuffle"]
    property var playModeTipText: [qsTr("Sequential play"), qsTr("List loop"), qsTr("Single loop"), qsTr("Shuffle")]
    property int playStatus: DmGlobal.Idle

    property ListModel pointModel: ListModel{}
    property var pointList: []

    id: toolbarRoot
    height:60
    width: parent.width

    outsideBorderColor: Palette {
        id: palette
        normal: Qt.rgba(0, 0, 0, 0.04)
    }

    anchors {
        left: parent.left
        right: parent.right
        bottom: parent.bottom
        leftMargin: 10
        rightMargin: 10
        bottomMargin: 10
    }
    contentItem: Row {
        width: parent.width
        height: parent.height
        anchors.fill: parent
        spacing: contentSpacing
        leftPadding: leftPaddingWidth

        Rectangle {
            property bool hovered: false

            id: coverRect
            width: coverRectWidth
            height: coverRectWidth
            radius: 8
            anchors.verticalCenter: parent.verticalCenter
            color: songTitle.length === 0 ? "#d7d7d7" : "transparent"
            enabled: songTitle.length === 0 ? false : true

            Image {
                id: img
                width: songTitle.length === 0 ? 24 : parent.width
                height: songTitle.length === 0 ? 24 : parent.height
                anchors.centerIn: parent
                source: mediaData ===  undefined || mediaData.localPath.length === 0 ? "qrc:/dsg/img/no_music.svg" : imgPath
                visible: songTitle.length === 0 ? true : false
            }
            Rectangle {
                id: mask
                anchors.fill: parent
                radius: 8
                visible: songTitle.length === 0 ? false : true
            }
            OpacityMask {
                anchors.fill: parent
                source: img
                maskSource: mask
                visible: songTitle.length === 0 ? false : true
            }

            // border
            Rectangle {
                id: borderRect
                anchors.fill: parent
                color: "transparent"
                border.color: Qt.rgba(0, 0, 0, 0.1)
                border.width: 1
                visible: true
                radius: 8
            }

            Rectangle {
                id: hoverMask
                width: parent.width
                height: parent.height
                color: "#99000000"
                radius: 8
                visible: coverRect.hovered
            }
            DciIcon {
                name: "toolbar_upglide";
                sourceSize: Qt.size(15, 15)
                anchors.centerIn: parent
                visible: coverRect.hovered && isLyricShow
            }
            DciIcon {
                id: downglide
                name: "toolbar_downglide";
                sourceSize: Qt.size(15, 15)
                anchors.centerIn: parent
                visible: coverRect.hovered && !isLyricShow
            }
            MouseArea {
                id: mouseArea
                anchors.fill: coverRect
                hoverEnabled: true
                onClicked: {
                    lyricToggleClicked()
                }
                onHoveredChanged: {
//                    console.log("...........................................", hovered)
                    coverRect.hovered = hovered
                }
                onExited: {
                    coverRect.hovered = false
                }
            }
        }
        Rectangle {
            id: infoRect
            width: infoRectWidth
            height: 40
            color: "transparent"
            anchors.verticalCenter: parent.verticalCenter
            Column {
                width: parent.width
                Text{
                    id: title
                    width: parent.width
                    color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(247, 247, 247, 0.9) : Qt.rgba(0, 0, 0, 0.9)
                    text: songTitle
                    font.pixelSize: 14
                    elide: Text.ElideRight
                }
                Text {
                    id: artist
                    width: parent.width
                    text: artistStr
                    font.pixelSize: 12
                    color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(247, 247, 247, 0.7) : Qt.rgba(0, 0, 0, 0.7)
                    font {
                        family: "SourceHanSansSC, SourceHanSansSC-Normal"
                        pixelSize: 12
                        weight: Font.Medium
                    }
                    elide: Text.ElideRight
                }
            }
        }
        Rectangle {
            id: playControlRect
            width: playControlRectWidth
            height: parent.height
            color: "transparent"
            anchors.verticalCenter: parent.verticalCenter
            Row {
                spacing: 10
                anchors.verticalCenter: parent.verticalCenter
                    ActionButton {
                        id: likeBtn
                        width: 36
                        height: 36
                        anchors.verticalCenter: parent.verticalCenter
                        icon.name: favorite ? "heart_check" : "heart"
                        icon.width: 20
                        icon.height: 20
                        enabled: songTitle.length === 0 ? false : true
                        palette.windowText: favorite ? "#F75B5B" : undefined
                        // contentItem.theme: DTK.themeType

                        ToolTip {
                            visible: likeBtn.hovered
                            text: favorite ? qsTr("Unfavorite") : qsTr("Favorite")
                        }

                        onClicked: {
                            if (mediaData === undefined)
                                return

                            var list = []
                            list.push(mediaData.hash)

                            if (mediaData.mmType !== DmGlobal.MimeTypeCDA) {
                                favorite ? Presenter.removeFromPlayList(list, "fav") : Presenter.addMetasToPlayList(list, "fav")
                                if(favorite === false)
                                    globalVariant.sendFloatingMessageBox(qsTr("My Favorites"), 2)
                            }
                        }
                    }
                    ActionButton {
                        id: prevBtn
                        width: 36
                        height: 36
                        anchors.verticalCenter: parent.verticalCenter
                        icon.name: "toolbar_previous"
                        icon.width: 20
                        icon.height: 20
                        checkable: true
                        enabled: songTitle.length === 0 ? false : true

                        ToolTip {
                            visible: prevBtn.hovered
                            //ToolTip.delay: 1000
                            text: qsTr("Previous")
                        }

                        onClicked: {
                            Presenter.playPre()
                        }
                    }
                    ActionButton {
                        id: playPauseBtn
                        width: 36
                        height: 36
                        anchors.verticalCenter: parent.verticalCenter
                        icon.name: playStatus == DmGlobal.Paused || playStatus == DmGlobal.Stopped || playStatus == DmGlobal.Idle?
                                       "toolbar_play" : (playStatus == DmGlobal.Playing ? "toolbar_pause" : "")
                        icon.width: 36
                        icon.height: 36
                        checkable: true

                        ToolTip {
                            visible: playPauseBtn.hovered
                            //ToolTip.delay: 1000
                            text: qsTr("Play/Pause")
                        }

                        onClicked: {
                            Presenter.playPause()
                        }
                    }
                    ActionButton {
                        id: nextBtn
                        width: 36
                        height: 36
                        anchors.verticalCenter: parent.verticalCenter
                        icon.name: "toolbar_next"
                        icon.width: 20
                        icon.height: 20
                        checkable: true
                        enabled: songTitle.length === 0 ? false : true

                        ToolTip {
                            visible: nextBtn.hovered
                            //ToolTip.delay: 1000
                            text: qsTr("Next")
                        }

                        onClicked: {
                            Presenter.nextMetaFromPlay(mediaData.hash)
                            Presenter.playNext()
                        }
                    }
                    ActionButton {
                        id: playModeBtn
                        width: 36
                        height: 36
                        anchors.verticalCenter: parent.verticalCenter
                        icon.name: modeIcon[playMode + 1]
                        icon.width: 36
                        icon.height: 36

                        ToolTip {
                            visible: playModeBtn.hovered
                            text: playModeTipText[playMode + 1]
                        }

                        onClicked: {
                            playMode++
                            //console.log(" set play mode:" + playMode)
                            if (playMode + 1>= modeIcon.length)
                                playMode = DmGlobal.RepeatNull
                            Presenter.setPlaybackMode(playMode)
                        }
                    }
            }
        }
        Rectangle {
            id: waveformRect
            width: parent.width - (coverRectWidth + infoRectWidth + playControlRectWidth + rightAreaRectWidth
                                   + contentSpacing * 4 + leftPaddingWidth + rightPaddingWidth)
            height: parent.height
            color: "transparent"
            enabled: songTitle.length === 0 ? false : true

            WaveformRect {
                id: waveform
                width: parent.width
                height: parent.height
                totalSecs: minute * 60 + second
                visible: songTitle.length === 0 ? false : true
            }
        }
        Rectangle {
            id: rightAreaRect
            width: rightAreaRectWidth
            height: parent.height
            color: "#00000000"
            Row {
                width: parent.width
                height: parent.height
                spacing: 10
                leftPadding: 10

                Rectangle {
                    id: timeLabel
                    width: 80
                    height: parent.height
                    color: "transparent"
                    anchors.verticalCenter: parent.verticalCenter

                    Row {
                        width: parent.width
                        height: parent.height
                        spacing: 4
                        leftPadding: 4

                        Text {
                            id: curTimeText
                            width: 37
                            anchors.verticalCenter: parent.verticalCenter
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignRight
                            enabled: songTitle.length === 0 ? false : true
                            text: enabled ? currentTime : "0:00"
                            color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(247, 247, 247, 0.7) : Qt.rgba(0, 0, 0, 0.7)
                            font: DTK.fontManager.t8
                        }
                        Text {
                            id: totalTimeText
                            width: 44
                            anchors.verticalCenter: parent.verticalCenter
                            elide: Text.ElideRight
                            enabled: songTitle.length === 0 ? false : true
                            text: "/ " + (enabled ? totalTime : "0:00")
                            color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(247, 247, 247, 0.7) : Qt.rgba(0, 0, 0, 0.7)
                            font: DTK.fontManager.t8

                        }
                    }
                }
                ToolButton {
                    id: lrcBtn
                    width: 36
                    height: 36
                    anchors.verticalCenter: parent.verticalCenter
                    icon.name: "toolbar_lrc"
                    icon.width: 36
                    icon.height: 36
                    checkable: true
                    checked: isLyricShow
                    enabled: songTitle.length === 0 ? false : true
                    ToolTip {
                        visible: lrcBtn.hovered
                        text: qsTr("Lyrics")
                    }
                    onClicked: {
                        lyricToggleClicked()
                    }
                }
                ToolButton {
                    id: volumeBtn
                    width: 36
                    height: 36
                    anchors.verticalCenter: parent.verticalCenter
                    icon.name: bMute ? ("toolbar_volume-")
                                     : ("toolbar_volume+")
                    icon.width: 36
                    icon.height: 36
                    checkable: true
                    //ColorSelector.pressed: true
                    onClicked: {
                        isVolSliderShow = !isVolSliderShow

                        if (volSliderLoader.status === Loader.Null) {
                            volSliderLoader.setSource("../toolbar/VolumeSlider.qml")
                            volSliderLoader.item.x = toolbarRoot.width - 10 * 3 - width * 2
                            volSliderLoader.item.y = -255
                            volSliderLoader.item.hoveredChanged.connect(onVolSliderHoveredChanged)
                        }
                        volSliderLoader.item.visible = isVolSliderShow
                    }
                    onHoveredChanged: {
                        if (!hovered) {
                            if (volSliderHideTimer.running) {
                                volSliderHideTimer.stop()
                            }
                            volSliderHideTimer.start()
                        }
                    }
                }
                ToolButton {
                    id: listBtn
                    width: 36
                    height: 36
                    anchors.verticalCenter: parent.verticalCenter
                    icon.name: "toolbar_playlist"
                    icon.width: 36
                    icon.height: 36
                    checkable: true

                    ToolTip {
                        visible: listBtn.hovered
                        text: qsTr("Play Queue")
                    }

                    onClicked: {
                        playlistBtnClicked()
                    }
                }
            }
        }
    }

    Timer {
        id: volSliderHideTimer
        interval: 1000
        repeat: false
        running: false
        onTriggered: {
            if (volSliderLoader.status !== Loader.Null)
                volSliderLoader.item.visible = false
            volumeBtn.checked = false
            isVolSliderShow = false
        }
    }

    Loader { id: volSliderLoader }

    onPlayModeChanged: {
        globalVariant.curPlayMode = playMode
        if (mediaData.hash == null)
            return

        updatePlayControlBtnStatus()
    }
    onWidthChanged: {
        if (volSliderLoader.status !== Loader.Null)
            volSliderLoader.item.x = width - 100
    }

    function metaChange(){
        var meta = Presenter.getActivateMeta()

        mediaData = meta
        songTitle = meta["title"]
        artistStr = meta["artist"]
        imgPath = "file:///" + meta["coverUrl"]
        favorite = meta.favourite

        if (songTitle.length === 0) {
            nextBtn.enabled = false
            prevBtn.enabled = false

            //无歌曲不显示波形图
            pointList = []
            waveform.onAudioDataChanged()

            return
        }

        var length = meta["length"]
        minute = Math.floor(length / 1000 / 60)
        second = Math.floor(length / 1000 % 60)
        totalTime = minute + (second < 10 ? ":0" : ":") + second

        updatePlayControlBtnStatus()
    }
    function positionChange(position, length) {
        var time = Math.round(position / 1000)
        curMinute = Math.floor(time / 60)
        curSecond = Math.floor(time % 60)
        if (curMinute >=0 && curSecond >= 0) {
            if (curSecond < 10)
                currentTime = curMinute + ":0" + curSecond
            else
                currentTime = curMinute + ":" + curSecond
        }
        //positionChanged(curMinute * 60 + curSecond)
        waveform.updatePosition(curMinute * 60 + curSecond)
    }
    function playbackStatusChange(status) {
        playStatus = status

        updatePlayControlBtnStatus()
    }
    function audioBufferChange(buffer,hash) {
        //console.log("toolbar audioBufferChange:...........")
        pointList = []
        pointList = buffer
        waveform.onAudioDataChanged()
    }
    function onDeleteOneMeta(playlistHashs, hash) {
        for (var i = 0; i < playlistHashs.length; i++){
            if (playlistHashs[i] == "fav") {
                if (mediaData.hash === hash) {
                    favorite = false
                    break
                }
            }
        }
    }
    function onAddOneMeta(playlistHashs, meta) {
        for (var i = 0; i < playlistHashs.length; i++){
            if (playlistHashs[i] == "fav") {
                if (mediaData.hash === meta.hash) {
                    favorite = true
                    break
                }
            }
        }
    }
    function updatePlaylistBtnStatus(checked) {
        listBtn.checked = checked
    }
    function updatePlayControlBtnStatus() {
//        console.log("updatePlayControlBtnStatus, playingCount:" + globalVariant.playingCount, " playlistExist:", globalVariant.playlistExist,
//                    " nextMetaFromPlay:", Presenter.nextMetaFromPlay(mediaData.hash), " preMetaFromPlay:", Presenter.preMetaFromPlay(mediaData.hash), " playMode:", playMode)
        if ((globalVariant.playlistExist && globalVariant.playingCount <= 1) ||
                (!Presenter.nextMetaFromPlay(mediaData.hash) && playMode === DmGlobal.RepeatNull && mediaData.mmType !== DmGlobal.MimeTypeCDA))
            nextBtn.enabled = false
        else
            nextBtn.enabled = true

        if ((globalVariant.playlistExist && globalVariant.playingCount <= 1) ||
                (!Presenter.preMetaFromPlay(mediaData.hash) && playMode === DmGlobal.RepeatNull && mediaData.mmType !== DmGlobal.MimeTypeCDA))
            prevBtn.enabled = false
        else
            prevBtn.enabled = true
    }

    function onVolSliderHoveredChanged() {
        if (!volSliderLoader.item.hovered) {
            if (volSliderHideTimer.running) {
                volSliderHideTimer.stop()
            }
            volSliderHideTimer.start()
        } else {
            volSliderHideTimer.stop()
        }
    }

    Component.onCompleted: {
        Presenter.metaChanged.connect(metaChange)
        Presenter.positionChanged.connect(positionChange)
        Presenter.playbackStatusChanged.connect(playbackStatusChange)
        Presenter.audioBuffer.connect(audioBufferChange)
        Presenter.deleteOneMeta.connect(onDeleteOneMeta)
        Presenter.addOneMeta.connect(onAddOneMeta)

        bMute = Presenter.valueFromSettings("base.play.mute")
        playMode = Presenter.getPlaybackMode()

        Presenter.restorePlaybackStatus();
        if (Presenter.valueFromSettings("base.play.remember_progress")) {
            positionChange(Presenter.getPosition())
        }

        Presenter.muteChanged.connect(function(mute){
            bMute = mute
        })

        updatePlayControlBtnStatus()
        globalVariant.playingCountChanged.connect(function(){updatePlayControlBtnStatus()})
    }
}
