import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import QtGraphicalEffects 1.0
import org.deepin.dtk 1.0


DialogWindow {
    id: dialog
    property var deleteHashList: []
    property string listHash: ""
    property string musicTitle: ""
    property bool removeMusic: true
    width: 400;
//    color: Qt.rgba(247,247,247,0.80);
    modality: Qt.ApplicationModal
    icon: globalVariant.appIconName
    ColumnLayout {
        width: parent.width
        Label {
            id:deleteSongsLabel
            Layout.preferredWidth: parent.width
            Layout.alignment: Qt.AlignHCenter
            font: DTK.fontManager.t5
            wrapMode: Text.WordWrap
            horizontalAlignment: Qt.AlignHCenter
            text: {
                if(deleteHashList.length <= 1 && removeMusic){
                    return qsTr("Are you sure you want to remove %1?").arg(dialog.musicTitle);
                }else if(deleteHashList.length > 1 && removeMusic){
                    return qsTr("Are you sure you want to remove the selected %1 songs?").arg(deleteHashList.length);
                }else {
                    return qsTr("Are you sure you want to delete this playlist?");
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            Layout.bottomMargin: 10
            Layout.topMargin: 10
            Layout.fillWidth: true
            WarningButton {
                text: removeMusic ? qsTr("Remove") : qsTr("Delete")
                Layout.preferredWidth: 185; Layout.preferredHeight: 36
                Layout.alignment: Qt.AlignRight
                onClicked: {
                    if(removeMusic){
                        if (listHash === "musicResult")
                            listHash = "all"
                        Presenter.removeFromPlayList(deleteHashList, listHash);
                        globalVariant.clearSelectGroup(); //清除选中
                    }else{
                        Presenter.deletePlaylist(listHash);
                    }
                    dialog.close();
                }
            }
            Button {
                text: qsTr("Cancel")
                Layout.preferredWidth: 185; Layout.preferredHeight: 36
                onClicked: {
                    dialog.close();
                }
            }
            Item {Layout.fillWidth: true}
        }
    }
}

