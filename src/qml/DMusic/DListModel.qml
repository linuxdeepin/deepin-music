import QtQuick 2.3

ListModel {
    id: listModel
    property var pymodel

    signal appendSignal(var obj)
    signal clearSignal()
    signal insertSignal(int index, var obj)
    signal moveSignal(int from, int to, int n)
    signal removeSignal(int index)
    signal setSignal(int index, var obj)
    signal setPropertySignal(int index, var property, var value)

    function initModel() {
        listModel.clear()
        for(var i=0; i<pymodel.data.length; i++){
            var obj = pymodel.data[i];
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
        listModel.insertSignal(index, obj);
    }

    function dmove(from ,to , n){
        listModel.moveSignal(from, to, n);
    }

    function dremove(index){
        listModel.removeSignal(index);
    }

    function dset(index, obj){
        listModel.setSignal(index, obj);
    }

    function dsetProperty(index, property , value){
        listModel.setPropertySignal(index, property, value);
    }

    onPymodelChanged:{
        if (pymodel){
            initModel();
        }
    }

    Component.onCompleted: {
        listModel.appendSignal.connect(pymodel.qml2Py_appendSignal);
        listModel.clearSignal.connect(pymodel.qml2py_clearSignal);
        listModel.insertSignal.connect(pymodel.qml2py_insertSignal);
        listModel.moveSignal.connect(pymodel.qml2py_moveSignal);
        listModel.removeSignal.connect(pymodel.qml2py_removeSignal);
        listModel.setSignal.connect(pymodel.qml2py_setSignal);
        listModel.setPropertySignal.connect(pymodel.qml2py_setPropertySignal);
        pymodel.py2qml_appendSignal.connect(listModel.append);
        pymodel.py2qml_clearSignal.connect(listModel.clear);
        pymodel.py2qml_insertSignal.connect(listModel.insert);
        pymodel.py2qml_moveSignal.connect(listModel.move);
        pymodel.py2qml_removeSignal.connect(listModel.remove);
        pymodel.py2qml_setSignal.connect(listModel.set);
        pymodel.py2qml_setPropertySignal.connect(listModel.setProperty);
    }
}