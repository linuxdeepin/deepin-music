#!/usr/bin/python
# -*- coding: utf-8 -*-
""" 
    Object browser GUI in Qt
    
    #####################
    # TODO: version 1.0 #
    #####################

    # What todo when getmembers fails
    # Installing
     - rename attribute_model.py to attributemodel.py
     - sphynx
    
    
    #####################
    # TODO: version 1.x #
    #####################
    # Ordered dict should not sort keys 
    # hide non-attributes' attributes? That is, list's and dict's attributes are hidden.
    # tool-tips
    # python 3
    # Examples:
     - Qt

"""

from __future__ import absolute_import
from __future__ import print_function
import os
import logging, traceback
from qframer.qt import QtCore, QtGui
from qframer.qt.QtCore import Qt
from qframer import FMainWindow
from qframer.resources import *
from qframer import setSkinForApp

from objbrowser.version import PROGRAM_NAME, PROGRAM_VERSION, PROGRAM_URL, DEBUGGING
from objbrowser.utils import setting_str_to_bool
from objbrowser.treemodel import TreeModel
from objbrowser.toggle_column_mixin import ToggleColumnTreeView
from objbrowser.attribute_model import DEFAULT_ATTR_COLS, DEFAULT_ATTR_DETAILS

logger = logging.getLogger(__name__)


# The main window inherits from a Qt class, therefore it has many 
# ancestors public methods and attributes.
# pylint: disable=R0901, R0902, R0904, W0201 

# It's not possible to use locals() as default for obj by taking take the locals
# from one stack frame higher; you can't know if the ObjectBrowser.__init__ was
# called directly, via the browse() wrapper or via a descendants' constructor.

class ObjectBrowser(FMainWindow):
    """ Object browser main application window.
    """
    _n_instances = 0
    
    def __init__(self, obj,  
                 name = '',
                 attribute_columns = DEFAULT_ATTR_COLS,  
                 attribute_details = DEFAULT_ATTR_DETAILS,  
                 show_routine_attributes = None,
                 show_special_attributes = None, 
                 reset = False):
        """ Constructor
        
            :param obj: any Python object or variable
            :param name: name of the object as it will appear in the root node
            :param attribute_columns: list of AttributeColumn objects that define which columns
                are present in the table and their defaults
            :param attribute_details: list of AttributeDetails objects that define which attributes
                can be selected in the details pane.
            :param show_routine_attributes: if True rows where the 'is attribute' and 'is routine'
                columns are both True, are displayed. Otherwise they are hidden. 
            :param show_special_attributes: if True rows where the 'is attribute' is True and
                the object name starts and ends with two underscores, are displayed. Otherwise 
                they are hidden.
            :param reset: If true the persistent settings, such as column widths, are reset. 
        """
        super(ObjectBrowser, self).__init__()

        ObjectBrowser._n_instances += 1
        self._instance_nr = self._n_instances        
        
        # Model
        self._attr_cols = attribute_columns
        self._attr_details = attribute_details
        
        (show_routine_attributes, 
         show_special_attributes) = self._readModelSettings(reset = reset, 
                                                            show_routine_attributes = show_routine_attributes,
                                                            show_special_attributes = show_special_attributes)
        self.show_routine_attributes = show_routine_attributes
        self.show_special_attributes = show_special_attributes
        
        # Views
        self._setup_views()

        self.setModel(obj)

        self._setup_actions()
        self._setup_menu()

        self.setWindowTitle("{} - {}".format(PROGRAM_NAME, name))
        app = QtGui.QApplication.instance()
        app.lastWindowClosed.connect(app.quit)

        self.initSize()

        self.titleBar().skinButton.hide()
        self.titleBar().closeButton.clicked.connect(self.close)

        self.setWindowIcon(":/icons/dark/appbar.tree.png")

        self.titleBar().modeButton.click()

        self.toggle_callable_action.toggled.emit(False)
        self.toggle_special_attribute_action.toggled.emit(False)

        self.setskin()

    def setskin(self, skinID="BB"):
        path = os.path.dirname(__file__)
        setSkinForApp( os.path.join(path, '%s.qss' % skinID))  # 设置主窗口样式

    def initSize(self):
        desktopWidth = QtGui.QDesktopWidget().availableGeometry().width()
        desktopHeight = QtGui.QDesktopWidget().availableGeometry().height()
        self.resize(
            desktopWidth * 0.6,
            desktopHeight * 0.8)
        self.moveCenter()

    def setModel(self, obj):
        self._tree_model = TreeModel(obj, 
            root_obj_name = '',
            attr_cols = self._attr_cols,  
            show_routine_attributes = self.show_routine_attributes,
            show_special_attributes = self.show_special_attributes
        )

        self.obj_tree.setModel(self._tree_model)

        first_row = self._tree_model.first_item_index()
        self.obj_tree.setCurrentIndex(first_row)

        # Connect signals
        selection_model = self.obj_tree.selectionModel()
        selection_model.currentChanged.connect(self._update_details)

    def _make_show_column_function(self, column_idx):
        """ Creates a function that shows or hides a column."""
        show_column = lambda checked: self.obj_tree.setColumnHidden(column_idx, not checked)
        return show_column

    def _setup_actions(self):
        """ Creates the main window actions.
        """
        # Show/hide callable objects
        self.toggle_callable_action = \
            QtGui.QAction("Show routine attributes", self, checkable=True, 
                          statusTip = "Shows/hides attributes that are routings (functions, methods, etc)")
        self.toggle_callable_action.toggled.connect(self.toggle_callables)
                              
        # Show/hide special attributes
        self.toggle_special_attribute_action = \
            QtGui.QAction("Show __special__ attributes", self, checkable=True, 
                          statusTip = "Shows or hides __special__ attributes")
        self.toggle_special_attribute_action.toggled.connect(self.toggle_special_attributes)
 
    def _setup_menu(self):
        """ Sets up the main menu.
        """
        # file_menu = self.menuBar().addMenu("&File")
        self.menu = QtGui.QMenu(self)
        self.menu.addAction("C&lose", self.close_window, "Ctrl+W")
        self.menu.addAction("E&xit", self.quit_application, "Ctrl+Q")
        if DEBUGGING is True:
            self.menu.addSeparator()
            self.menu.addAction("&Test", self.my_test, "Ctrl+T")

        self.show_cols_submenu = self.menu.addMenu("Table columns")
        self.menu.addAction(self.toggle_callable_action)
        self.menu.addAction(self.toggle_special_attribute_action)

        self.titleBar().settingDownButton.setMenu(self.menu)
        self.titleBar().settingMenuShowed.connect(
            self.titleBar().settingDownButton.showMenu)

    def _setup_views(self):
        """ Creates the UI widgets. 
        """
        self.central_splitter = QtGui.QSplitter(self, orientation = QtCore.Qt.Vertical)
        self.setCentralWidget(self.central_splitter)
        # central_layout = QtGui.QVBoxLayout()
        # self.central_splitter.setLayout(central_layout)
        
        # Tree widget
        self.obj_tree = ToggleColumnTreeView()
        self.obj_tree.setAlternatingRowColors(True)
        self.obj_tree.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.obj_tree.setUniformRowHeights(True)
        self.obj_tree.setAnimated(True)
        self.obj_tree.add_header_context_menu()
        
        # Stretch last column? 
        # It doesn't play nice when columns are hidden and then shown again.
        obj_tree_header = self.obj_tree.header()
        # obj_tree_header.setMovable(True)
        obj_tree_header.setStretchLastSection(False)
        for action in self.obj_tree.toggle_column_actions_group.actions():
            self.show_cols_submenu.addAction(action)

        self.central_splitter.addWidget(self.obj_tree)

        # Bottom pane
        bottom_pane_widget = QtGui.QWidget()
        bottom_layout = QtGui.QHBoxLayout()
        bottom_layout.setSpacing(0)
        bottom_layout.setContentsMargins(5, 5, 5, 5) # left top right bottom
        bottom_pane_widget.setLayout(bottom_layout)
        self.central_splitter.addWidget(bottom_pane_widget)
        
        group_box = QtGui.QGroupBox("Details")
        bottom_layout.addWidget(group_box)
        
        group_layout = QtGui.QHBoxLayout()
        group_layout.setContentsMargins(2, 2, 2, 2) # left top right bottom
        group_box.setLayout(group_layout)
        
        # Radio buttons
        radio_widget = QtGui.QWidget()
        radio_layout = QtGui.QVBoxLayout()
        radio_layout.setContentsMargins(0, 0, 0, 0) # left top right bottom        
        radio_widget.setLayout(radio_layout) 

        self.button_group = QtGui.QButtonGroup(self)
        for button_id, attr_detail in enumerate(self._attr_details):
            radio_button = QtGui.QRadioButton(attr_detail.name)
            radio_layout.addWidget(radio_button)
            self.button_group.addButton(radio_button, button_id)

        self.button_group.buttonClicked[int].connect(self._change_details_field)
        self.button_group.button(0).setChecked(True)
                
        radio_layout.addStretch(1)
        group_layout.addWidget(radio_widget)

        # Editor widget
        font = QtGui.QFont()
        font.setFamily('Courier')
        font.setFixedPitch(True)
        #font.setPointSize(14)

        self.editor = QtGui.QPlainTextEdit()
        self.editor.setReadOnly(True)
        self.editor.setFont(font)
        group_layout.addWidget(self.editor)
        
        # Splitter parameters
        self.central_splitter.setCollapsible(0, False)
        self.central_splitter.setCollapsible(1, True)
        self.central_splitter.setSizes([400, 200])
        self.central_splitter.setStretchFactor(0, 10)
        self.central_splitter.setStretchFactor(1, 0)

    # End of setup_methods
    
    
    def _settings_group_name(self, prefix):
        """ The persistent settings are stored per combination of column names
            and windows instance number.
        """
        column_names = ",".join([col.name for col in self._attr_cols])
        settings_str = column_names + str(self._instance_nr)
        settings_grp = "{}_{}".format(prefix, hex(hash(settings_str)))
        logger.debug("  settings group is: {!r}".format(settings_grp))
        return settings_grp

                
    def _readModelSettings(self, 
                           reset=False, 
                           show_routine_attributes = None,
                           show_special_attributes = None):
        """ Reads the persistent model settings .
            The persistent settings (show_routine_attributes, show_special_attributes) can be \
            overridden by giving it a True or False value.
            If reset is True and the setting is None, True is used as default.
        """ 
        default_sra = True
        default_ssa = True
        if reset:
            logger.debug("Resetting persistent model settings")
            if show_routine_attributes is None:
                show_routine_attributes = default_sra
            if show_special_attributes is None:
                show_special_attributes = default_ssa
        else:
            logger.debug("Reading model settings for window: {:d}".format(self._instance_nr))
            settings = QtCore.QSettings()
            settings.beginGroup(self._settings_group_name('model'))
            if show_routine_attributes is None:
                show_routine_attributes = setting_str_to_bool(
                    settings.value("show_routine_attributes", default_sra))
            if show_special_attributes is None:
                show_special_attributes = setting_str_to_bool(
                    settings.value("show_special_attributes", default_ssa))
            settings.endGroup()
            logger.debug("read show_routine_attributes: {!r}".format(show_routine_attributes))
            logger.debug("read show_special_attributes: {!r}".format(show_special_attributes))
        return (show_routine_attributes, show_special_attributes)
                    
    
    def _writeModelSettings(self):
        """ Writes the model settings to the persistent store
        """         
        logger.debug("Writing model settings for window: {:d}".format(self._instance_nr))
        
        settings = QtCore.QSettings()
        settings.beginGroup(self._settings_group_name('model'))
        logger.debug("writing show_routine_attributes: {!r}".format(self._tree_model.getShowCallables()))
        logger.debug("wrting show_special_attributes: {!r}".format(self._tree_model.getShowSpecialAttributes()))
        settings.setValue("show_routine_attributes", self._tree_model.getShowCallables())
        settings.setValue("show_special_attributes", self._tree_model.getShowSpecialAttributes())
        settings.endGroup()
        
    
    def _readViewSettings(self, reset=False):
        """ Reads the persistent program settings
        
            :param reset: If True, the program resets to its default settings
        """ 
        pos = QtCore.QPoint(20 * self._instance_nr, 20 * self._instance_nr)
        window_size = QtCore.QSize(1024, 700)
        details_button_idx = 0
        
        header = self.obj_tree.header()
        header_restored = False
        
        if reset:
            logger.debug("Resetting persistent view settings")
        else:
            logger.debug("Reading view settings for window: {:d}".format(self._instance_nr))
            settings = QtCore.QSettings()
            settings.beginGroup(self._settings_group_name('view'))
            pos = settings.value("main_window/pos", pos)
            window_size = settings.value("main_window/size", window_size)
            details_button_idx = int(settings.value("details_button_idx", details_button_idx))
            self.central_splitter.restoreState(settings.value("central_splitter/state"))
            header_restored = self.obj_tree.read_view_settings('table/header_state', 
                                                               settings, reset) 
            settings.endGroup()

        if not header_restored:
            column_sizes = [col.width for col in self._attr_cols]
            column_visible = [col.col_visible for col in self._attr_cols]
        
            for idx, size in enumerate(column_sizes):
                if size > 0: # Just in case 
                    header.resizeSection(idx, size)
    
            for idx, visible in enumerate(column_visible):
                self.obj_tree.toggle_column_actions_group.actions()[idx].setChecked(visible)  
            
        self.resize(window_size)
        self.move(pos)
        button = self.button_group.button(details_button_idx)
        if button is not None:
            button.setChecked(True)



    def _writeViewSettings(self):
        """ Writes the view settings to the persistent store
        """         
        logger.debug("Writing view settings for window: {:d}".format(self._instance_nr))
        
        settings = QtCore.QSettings()
        settings.beginGroup(self._settings_group_name('view'))
        self.obj_tree.write_view_settings("table/header_state", settings)
        settings.setValue("central_splitter/state", self.central_splitter.saveState())
        settings.setValue("details_button_idx", self.button_group.checkedId())
        settings.setValue("main_window/pos", self.pos())
        settings.setValue("main_window/size", self.size())
        settings.endGroup()
            

    @QtCore.Slot(QtCore.QModelIndex, QtCore.QModelIndex)
    def _update_details(self, current_index, _previous_index):
        """ Shows the object details in the editor given an index.
        """
        tree_item = self._tree_model.treeItem(current_index)
        self._update_details_for_item(tree_item)

        
    def _change_details_field(self, _button_id=None):
        """ Changes the field that is displayed in the details pane
        """
        #logger.debug("_change_details_field: {}".format(_button_id))
        current_index = self.obj_tree.selectionModel().currentIndex()
        tree_item = self._tree_model.treeItem(current_index)
        self._update_details_for_item(tree_item)
        
            
    def _update_details_for_item(self, tree_item):
        """ Shows the object details in the editor given an tree_item
        """
        self.editor.setStyleSheet("color: black;")
        try:
            #obj = tree_item.obj
            button_id = self.button_group.checkedId()
            assert button_id >= 0, "No radio button selected. Please report this bug."
            attr_details = self._attr_details[button_id]
            data = attr_details.data_fn(tree_item)
            self.editor.setPlainText(data)
            self.editor.setWordWrapMode(attr_details.line_wrap)
            
        except StandardError, ex:
            self.editor.setStyleSheet("color: red;")
            stack_trace = traceback.format_exc()
            self.editor.setPlainText("{}\n\n{}".format(ex, stack_trace))
            self.editor.setWordWrapMode(QtGui.QTextOption.WrapAtWordBoundaryOrAnywhere)
            if DEBUGGING is True:
                raise

    
    def toggle_callables(self, checked):
        """ Shows/hides the special callable objects.
            Callable objects are functions, methods, etc. They have a __call__ attribute. 
        """
        self.show_routine_attributes = checked
        logger.debug("toggle_callables: {}".format(checked))
        self._tree_model.setShowCallables(checked)
        if self._tree_model.show_root_node:
            self.obj_tree.expandToDepth(0)


    def toggle_special_attributes(self, checked):
        """ Shows/hides the special attributes.
            Special attributes are objects that have names that start and end with two underscores.
        """
        self.show_special_attributes = checked
        logger.debug("toggle_special_attributes: {}".format(checked))
        self._tree_model.setShowSpecialAttributes(checked)
        if self._tree_model.show_root_node:
            self.obj_tree.expandToDepth(0)

    def my_test(self):
        """ Function for testing """
        logger.debug("my_test")
        
    def about(self):
        """ Shows the about message window. """
        message = u"{} version {}\n\n{}""".format(PROGRAM_NAME, PROGRAM_VERSION, PROGRAM_URL)
        QtGui.QMessageBox.about(self, "About {}".format(PROGRAM_NAME), message)

    def close_window(self):
        """ Closes the window """
        self.close()
        
    def quit_application(self):
        """ Closes all windows """
        self.close()

    # def closeEvent(self, event):
    #     """ Close all windows (e.g. the L0 window).
    #     """
    #     logger.debug("closeEvent")
    #     # self._writeModelSettings()                
    #     # self._writeViewSettings()                
    #     self.close()
    #     event.accept()
