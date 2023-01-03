import QtQuick 2.0
import Qt.labs.platform 1.0

FolderDialog {
    id: folderDialog
    folder: StandardPaths.standardLocations(StandardPaths.MusicLocation)[0]
    onAccepted: {
        var list = []
        list.push(folder)
        Presenter.importMetas(list)
    }
}
