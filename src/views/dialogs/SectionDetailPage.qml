import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"

Rectangle {

	Column{
        spacing: 10
        TextItem {
            keyText: I18nWorker.info_songTitle
            valueText: QmlDialog.songObj.title
        }

        TextItem {
            keyText: I18nWorker.info_songArtist
            valueText: QmlDialog.songObj.artist
        }

        TextItem {
            keyText: I18nWorker.info_songAlbum
            valueText: QmlDialog.songObj.album
        }

        TextItem {
            keyText: I18nWorker.info_albumArtist
            valueText: QmlDialog.songObj.artist
        }

        TextItem {
            keyText: I18nWorker.info_songAge
            valueText: QmlDialog.songObj.date
        }

        TextItem {
            keyText: I18nWorker.info_songComposer
            valueText: QmlDialog.songObj.composer
        }

        TextItem {
            keyText: I18nWorker.info_songTracks
            valueText: UtilWorker.int_to_string(QmlDialog.songObj.tracknumber) + '/' + UtilWorker.int_to_string(QmlDialog.songObj.discnumber)
        }
    }
}