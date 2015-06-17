#!/usr/bin/python
# -*- coding: utf-8 -*-

from .qt.QtCore import *
from .qt.QtGui import *


class FDragRowsTableWidget(QTableWidget):

    def __init__(self, rows=0, cloumns=2, parent=None):
        super(FDragRowsTableWidget, self).__init__(rows, cloumns, parent)
        self.parent = parent
        self.setEditTriggers(self.NoEditTriggers)
        self.setDragEnabled(True)
        self.setAcceptDrops(True)
        self.viewport().setAcceptDrops(True)
        self.setDragDropOverwriteMode(False)
        self.setDropIndicatorShown(True)

        self.setSelectionMode(QAbstractItemView.SingleSelection)
        self.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.setDragDropMode(QAbstractItemView.InternalMove)

        headerview = QHeaderView(Qt.Horizontal, self)
        self.setHorizontalHeader(headerview)

        self.setFocusPolicy(Qt.NoFocus)
        self.verticalHeader().setVisible(False)

    def dropEvent(self, event):
        if event.source() == self and \
                (event.dropAction() == Qt.MoveAction or
                 self.dragDropMode() == QAbstractItemView.InternalMove):
            success, row, col, topIndex = self.dropOn(event)
            if success:
                selRows = self.getSelectedRowsFast()

                top = selRows[0]
                # print 'top is %d'%top
                dropRow = row
                if dropRow == -1:
                    dropRow = self.rowCount()
                # print 'dropRow is %d'%dropRow
                offset = dropRow - top
                # print 'offset is %d'%offset

                for i, row in enumerate(selRows):
                    r = row + offset
                    if r > self.rowCount() or r < 0:
                        r = 0
                    self.insertRow(r)
                    # print 'inserting row at %d'%r

                selRows = self.getSelectedRowsFast()
                # print 'selected rows: %s'%selRows

                top = selRows[0]
                # print 'top is %d'%top
                offset = dropRow - top
                # print 'offset is %d'%offset
                for i, row in enumerate(selRows):
                    r = row + offset
                    if r > self.rowCount() or r < 0:
                        r = 0

                    for j in range(self.columnCount()):
                        # print 'source is (%d, %d)'%(row, j)
                        # print 'item text: %s'%self.item(row,j).text()
                        source = QTableWidgetItem(self.item(row, j))
                        # print 'dest is (%d, %d)'%(r,j)
                        self.setItem(r, j, source)

                # Why does this NOT need to be here?
                # for row in reversed(selRows):
                    # self.removeRow(row)

                event.accept()

        else:
            QTableView.dropEvent(event)

    def getSelectedRowsFast(self):
        selRows = []
        for item in self.selectedItems():
            if item.row() not in selRows:
                selRows.append(item.row())
        return selRows

    def droppingOnItself(self, event, index):
        dropAction = event.dropAction()

        if self.dragDropMode() == QAbstractItemView.InternalMove:
            dropAction = Qt.MoveAction

        if event.source() == self and \
                event.possibleActions() & Qt.MoveAction and \
                dropAction == Qt.MoveAction:
            selectedIndexes = self.selectedIndexes()
            child = index
            while child.isValid() and child != self.rootIndex():
                if child in selectedIndexes:
                    return True
                child = child.parent()

        return False

    def dropOn(self, event):
        if event.isAccepted():
            return False, None, None, None

        index = QModelIndex()
        row = -1
        col = -1

        if self.viewport().rect().contains(event.pos()):
            index = self.indexAt(event.pos())
            if not index.isValid() or \
                    not self.visualRect(index).contains(event.pos()):
                index = self.rootIndex()

        if self.model().supportedDropActions() & event.dropAction():
            if index != self.rootIndex():
                dropIndicatorPosition = self.position(
                    event.pos(), self.visualRect(index), index)

                if dropIndicatorPosition == QAbstractItemView.AboveItem:
                    row = index.row()
                    col = index.column()
                    # index = index.parent()
                elif dropIndicatorPosition == QAbstractItemView.BelowItem:
                    row = index.row() + 1
                    col = index.column()
                    # index = index.parent()
                else:
                    row = index.row()
                    col = index.column()

            if not self.droppingOnItself(event, index):
                # print 'row is %d'%row
                # print 'col is %d'%col
                return True, row, col, index

        return False, None, None, None

    def position(self, pos, rect, index):
        r = QAbstractItemView.OnViewport
        margin = 2
        if pos.y() - rect.top() < margin:
            r = QAbstractItemView.AboveItem
        elif rect.bottom() - pos.y() < margin:
            r = QAbstractItemView.BelowItem
        elif rect.contains(pos, True):
            r = QAbstractItemView.OnItem

        if r == QAbstractItemView.OnItem and \
                not (self.model().flags(index) & Qt.ItemIsDropEnabled):
            r = QAbstractItemView.AboveItem if pos.y() < rect.center(
            ).y() else QAbstractItemView.BelowItem

        return r


class FDetailShow(QTextEdit):

    def __init__(self, jsondata, parent=None):
        super(FDetailShow, self).__init__(parent)
        self.parent = parent
        self.setText(jsondata)
        self.setReadOnly(True)
        self.installEventFilter(self)
        self.setFocus()

    def mousePressEvent(self, event):
        if event.button() == Qt.RightButton:
            pass
        else:
            super(FDetailShow, self).mousePressEvent(event)


class FTableItemDetailWidget(QFrame):

    def __init__(self, jsondata, row, column, parent=None):
        super(FTableItemDetailWidget, self).__init__(parent)
        self.parent = parent
        self.startX = 0
        self.row = row
        for i in range(column):
            self.startX += self.parent.columnWidth(i)
        self.setWindowFlags(Qt.Popup)
        self.setFixedSize(self.parent.columnWidth(3), 220)
        detailShow = FDetailShow(jsondata, self)
        detailShow.setFixedSize(self.width(), 200)

        self.titleLabel = QLabel("Data", self)
        self.titleLabel.setAlignment(Qt.AlignCenter)
        self.titleLabel.setObjectName("FTableItemDetailWidgetTitlebar")
        self.titleLabel.setFixedSize(self.parent.columnWidth(3), 20)
        mainlayout = QVBoxLayout()
        mainlayout.addWidget(self.titleLabel)
        mainlayout.addWidget(detailShow)
        mainlayout.setSpacing(0)
        mainlayout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(mainlayout)
        self.installEventFilter(self)

        self.show()

    def eventFilter(self, obj, event):
        if event.type() == QEvent.MouseButtonPress:
            self.close()
            return True
        else:
            return super(FTableItemDetailWidget, self).eventFilter(obj, event)

    def showDetail(self):
        self.jsonshowPosX = self.parent.mapToGlobal(QPoint(self.startX, 0)).x()
        self.jsonshowPosY = self.parent.mapToGlobal(QPoint(self.startX, self.parent.rowViewportPosition(self.row))).y()\
            - self.height() + self.parent.horizontalHeader().height()
        self.move(self.jsonshowPosX, self.jsonshowPosY)
        self.show()

    def resizeEvent(self, event):
        self.titleLabel.setFixedWidth(self.width())
