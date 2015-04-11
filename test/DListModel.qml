import QtQuick 2.4

ListModel {
    id: listModel
    property var _model: myListModel

    signal appendSignal(var obj)
    signal clearSignal()
    signal insertSignal(int index, var obj)
    signal moveSignal(int from, int to, int n)
    signal removeSignal(int index)
    signal setSignal(int index, var obj)
    signal setPropertySignal(int index, var property, var value)
    

    function initModel() {
        for(var i=0; i<_model.data.length; i++){
            var obj = _model.data[i];
            listModel.append(obj);
        }
    }

    function dappend(obj){
        listModel.appendSignal(obj);
    }

    function dclear(){
        listModel.clearSignal();
    }

    function dinsert(index, obj){
        listModel.insert(index, obj);
    }

    function dmove(from ,to , n){
        listModel.moveSignal(from, to, n);
    }

    function dremove(index){
        listModel.removeSignal(index);
    }

    function dset(index, obj){
        listModel.dset(index, obj);
    }

    function dsetProperty(index, property , value){
        listModel.setPropertySignal(index, property, value);
    }

    Component.onCompleted: {
        initModel();
        listModel.appendSignal.connect(_model.qml2Py_appendSignal)
        listModel.clearSignal.connect(_model.qml2py_clearSignal)
        listModel.insertSignal.connect(_model.qml2py_insertSignal)
        listModel.moveSignal.connect(_model.qml2py_moveSignal)
        listModel.removeSignal.connect(_model.qml2py_removeSignal)
        listModel.setSignal.connect(_model.qml2py_setSignal)
        listModel.setPropertySignal.connect(_model.qml2py_setPropertySignal)
        _model.py2qml_appendSignal.connect(listModel.append)
        _model.py2qml_clearSignal.connect(listModel.clear)
        _model.py2qml_insertSignal.connect(listModel.insert)
        _model.py2qml_moveSignal.connect(listModel.move)
        _model.py2qml_removeSignal.connect(listModel.remove)
        _model.py2qml_setSignal.connect(listModel.set)
        _model.py2qml_setPropertySignal.connect(listModel.setProperty)
    }
}