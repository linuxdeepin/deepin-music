import QtQuick 2.3
import QtQuick.Controls 1.2
import Deepin.Widgets 1.0

Rectangle{
    id: playlistNameDelegate
    width: 120
    height: 16
    color: "transparent"
    Rectangle {
        id: textBox
        width: 94
        height: 16

        color: "transparent"
        CheckBox {
            id: playlistNameText
            text: {
                var name = playlistNameDelegate.ListView.view.model[index].name;
                if(name == 'favorite'){
                    return I18nWorker.favorite
                }else if(name == 'temporary'){
                    return I18nWorker.temporary
                }else{
                    return name;
                }
                
            }

            onCheckedChanged:{
                var checkNames = playlistNameDelegate.ListView.view.checkNames;
                if (checked){
                    checkNames[index] = true;
                }else{
                    checkNames[index] = false;
                }
            }
        }
    }

    Component.onCompleted: {

    }
}
