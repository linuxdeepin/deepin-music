import QtQuick 2.3
import DMusic 1.0
import "../DMusicWidgets"
import Deepin.Widgets 1.0

Rectangle {

    Column{
        spacing: 10
        TextItem {
            keyText: I18nWorker.info_songType
            valueText: QmlDialog.songObj.genre
        }

        TextItem {
            keyText: I18nWorker.info_songFormat
            valueText: QmlDialog.songObj.ext
        }

        TextItem {
            keyText: I18nWorker.info_songSize
            valueText: UtilWorker.size_to_string(QmlDialog.songObj.size)
        }

        TextItem {
            keyText: I18nWorker.info_songDuration
            valueText: UtilWorker.duration_to_string(QmlDialog.songObj.duration)
        }

        TextItem {
            keyText: I18nWorker.info_songAge
            valueText: QmlDialog.songObj.date
        }

        TextItem {
            keyText: I18nWorker.info_songBitrate
            valueText: UtilWorker.bitrate_to_string(QmlDialog.songObj.bitrate)
        }

        TextItem {
            keyText: I18nWorker.info_songSampleRate
            valueText: UtilWorker.sampleRate_to_string(QmlDialog.songObj.sample_rate)
        }

        TextItem {
            keyText: I18nWorker.info_songPlayCount
            valueText:  UtilWorker.int_to_string(QmlDialog.songObj.playCount)
        }
    }

}