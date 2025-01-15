// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Dialogs
import Qt.labs.platform 1.0

FileDialog {
//    property string listHash: ""
      id: fileDialog
      folder: StandardPaths.writableLocation(StandardPaths.MusicLocation)
      nameFilters: {
          var suffix = Presenter.supportedSuffixList()
          var string = qsTr("All Music")
          string += "("
          string += suffix[0]
          var suffixList = []

          for (var i = 1; i < suffix.length; i++) {
              string += " "
              string += suffix[i]
          }
          string += ")"

          suffixList.push(string)
          //console.log(string)

          return suffixList
      }
      fileMode: FileDialog.OpenFiles
      onAccepted: {
          var list = []
          for (var i = 0; i < files.length; i++)
              list.push(files[i])
          Presenter.importMetas(list, globalVariant.curListPage)
      }
}
