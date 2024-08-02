// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
import Qt5Compat.GraphicalEffects
import org.deepin.dtk 1.0
import "../musicmousemenu"

Rectangle {
    id: backgroundButton1
    property Menu artistMoreMenu: ArtistMoreMenu{}
    property Menu albumMoreMenu: AlbumMoreMenu{}
    color: "transparent"
    Rectangle {
        id: imageRectangle
        color: "transparent"
        width: 184; height: 184
        RoundedImage{
            id: image
            backgroundWidth: 184; backgroundHeight: 184
            backgroundImageUrl: "file:///" + currentData.coverUrl
        }
    }

    Rectangle {
        anchors.bottom: imageRectangle.bottom
        anchors.left: imageRectangle.right; anchors.leftMargin: 20
        width: 479; height: 126
        color: "transparent"
        Label {
            id: artistName
            width: 479; height: 46
            elide: Text.ElideRight
            color: "#000000"
            text: (currentData.name === "") ? "undefine" : currentData.name
            font: DTK.fontManager.t2
        }
        Row {
            anchors.top: artistName.bottom; anchors.topMargin: 10
            width: 180; height: 20
            spacing: 14
            Label {
                elide: Text.ElideRight
                text: currentData.artist
                color: "#000000"
            }
            Label {
                anchors.bottom: parent.bottom
                width: 56; height: 17
                verticalAlignment: Text.AlignBottom
                text: currentData.musicCount === 1 ? qsTr("1 song") : qsTr("%1 songs").arg(currentData.musicCount)
                font: DTK.fontManager.t8
                color: "#7C7C7C"
            }
        }

        Button {
            id: palyall
            anchors.bottom: parent.bottom;
            width: 96; height: 30
            text: qsTr("Play All")
            checkable: false
            checked: true
            spacing: 5
            leftPadding: 0

            icon {
                name: "list_play"
                width: 20
                height: 20
            }

            onClicked: {
                if (pageHash == "album") {
                    Presenter.playAlbum(currentData.name);
                } else {
                    Presenter.playArtist(currentData.name);
                }
                checked = true
            }
        }
        FloatingButton {
            anchors.left: palyall.right; anchors.leftMargin: 20
            anchors.bottom: palyall.bottom
            width: 30
            height: 30
            checked: false
            icon {
                width: 20
                height: 20
                name: "list_more"
            }
            onClicked: {
                if(pageHash == "album"){
                    albumMoreMenu.currentPageHash = "albumSublist"
                    albumMoreMenu.albumData = currentData
                    albumMoreMenu.popup();
                }else {
                    artistMoreMenu.currentPageHash = "artistSublist"
                    artistMoreMenu.artistData = currentData
                    artistMoreMenu.popup();
                }
            }
        }
    }
}
