""" Module that defines AttributeModel
"""
from __future__ import absolute_import

__all__ = ['browse', 'execute', 'create_object_browser', 'logging_basic_config']

from qframer.qt.QtCore import Qt
from qframer.qt.QtGui import QTextOption

import logging, inspect, string, pprint

try:
    import numpy as np
except ImportError:
    _NUMPY_INSTALLED = False
else:
    _NUMPY_INSTALLED = True

logger = logging.getLogger(__name__)


SMALL_COL_WIDTH = 120
MEDIUM_COL_WIDTH = 200

_PRETTY_PRINTER = pprint.PrettyPrinter(indent=4)

_ALL_PREDICATES = (inspect.ismodule, inspect.isclass, inspect.ismethod,
                   inspect.isfunction, inspect.isgeneratorfunction, inspect.isgenerator,
                   inspect.istraceback, inspect.isframe, inspect.iscode,
                   inspect.isbuiltin, inspect.isroutine, inspect.isabstract,
                   inspect.ismethoddescriptor, inspect.isdatadescriptor, 
                   inspect.isgetsetdescriptor, inspect.ismemberdescriptor) 

class AttributeModel(object):
    """ Determines how an object attribute is rendered in a table column or details pane
    """ 
    def __init__(self, name,
                 doc = "<no help available>",  
                 data_fn = None,  
                 col_visible = True, 
                 width = SMALL_COL_WIDTH,
                 alignment = Qt.AlignLeft, 
                 line_wrap = QTextOption.NoWrap):
        """
            Constructor
            
            :param name: name used to describe the attribute
            :type name: string
            :param doc: short string documenting the attribute
            :type doc: string
            :param data_fn: function that calculates the value shown in the UI
            :type  data_fn: function(TreeItem_ to string.
            :param col_visible: if True, the attribute is col_visible by default in the table
            :type col_visible: bool
            :param width: default width in the attribute table
            :type with: int
            :param alignment: alignment of the value in the table
            :type alighment: Qt.AlignmentFlag 
            :param line_wrap: Line wrap mode of the attribute in the details pane
            :type line_wrap: QtGui.QPlainTextEdit
        """

        if not callable(data_fn):
            raise ValueError("data_fn must be function(TreeItem)->string")
            
        self.name = name
        self.doc = doc
        self.data_fn = data_fn
        self.col_visible = col_visible
        self.width = width
        self.alignment = alignment
        self.line_wrap = line_wrap
        
    def __repr__(self):
        """ String representation """
        return "<AttributeModel for {!r}>".format(self.name)
        
    
    @property
    def settings_name(self):
        """ The name where spaces are replaced by underscores 
        """
        sname = self.name.replace(' ', '_')
        return sname.translate(None, string.punctuation).translate(None, string.whitespace)


###################
# Data functions ##
###################


def tio_call(obj_fn, tree_item):
    """ Calls obj_fn(tree_item.obj)
    """
    return obj_fn(tree_item.obj)


def safe_tio_call(obj_fn, tree_item, log_exceptions=False):
    """ Call the obj_fn(tree_item.obj). 
        Returns empty string in case of an error.
    """ 
    tio = tree_item.obj
    try:
        return str(obj_fn(tio))
    except StandardError, ex:
        if log_exceptions:
            logger.exception(ex)
        return ""    


def safe_data_fn(obj_fn, log_exceptions=False):
    """ Creates a function that returns an empty string in case of an exception.
        
        :param fnobj_fn: function that will be wrapped
        :type obj_fn: object to basestring function
        :returns: function that can be used as AttributeModel data_fn attribute
        :rtype: objbrowser.treeitem.TreeItem to string function 
    """
    def data_fn(tree_item):
        """ Call the obj_fn(tree_item.obj). 
            Returns empty string in case of an error
        """ 
        return safe_tio_call(obj_fn, tree_item, log_exceptions=log_exceptions)
    
    return data_fn


def tio_predicates(tree_item):
    """ Returns the inspect module predicates that are true for this object
    """
    tio = tree_item.obj
    predicates = [pred.__name__ for pred in _ALL_PREDICATES if pred(tio)]
    return ", ".join(predicates)


def tio_summary(tree_item):
    """ Returns a small summary of regular objects. 
        For callables and modules an empty string is returned.
    """
    tio = tree_item.obj
    if isinstance(tio, basestring):
        return tio
    elif isinstance(tio, (list, tuple, set, frozenset, dict)):  
        n_items = len(tio)
        if n_items == 0:
            return "empty {}".format(type(tio).__name__)
        if n_items == 1:
            return "{} of {} item".format(type(tio).__name__, n_items)
        else:
            return "{} of {} items".format(type(tio).__name__, n_items)
    elif _NUMPY_INSTALLED and isinstance(tio, np.ndarray):
        return "array of {}, shape: {}".format(tio.dtype, tio.shape)
    elif callable(tio) or inspect.ismodule(tio):
        return "" 
    else:
        return str(tio)
    
    
def tio_is_attribute(tree_item):
    """ Returns 'True' if the tree item object is an attribute of the parent 
        opposed to e.g. a list element.
    """
    if tree_item.is_attribute is None:
        return ''
    else:
        return str(tree_item.is_attribute)
   
    
def tio_is_callable(tree_item):
    "Returns 'True' if the tree item object is callable"
    return str(callable(tree_item.obj)) # Python 2
    #return str(hasattr(tree_item.obj, "__call__")) # Python 3?


def tio_doc_str(tree_item):
    """ Returns the doc string of an object
    """
    tio = tree_item.obj
    try:
        return tio.__doc__
    except AttributeError:
        return '<no doc string found>'
          

#######################
# Column definitions ##
#######################

ATTR_MODEL_NAME = AttributeModel('name', 
    doc         = "The name of the object.", 
    data_fn     = lambda(tree_item): tree_item.obj_name if tree_item.obj_name else '<root>',
    col_visible = True,  
    width       = SMALL_COL_WIDTH) 

ATTR_MODEL_PATH = AttributeModel('path', 
    doc         = "A path to the data: e.g. var[1]['a'].item", 
    data_fn     = lambda(tree_item): tree_item.obj_path if tree_item.obj_path else '<root>', 
    col_visible = True,  
    width       = MEDIUM_COL_WIDTH) 

ATTR_MODEL_SUMMARY = AttributeModel('summary', 
    doc         = "A summary of the object for regular objects (not callables or modules)", 
    data_fn     = tio_summary,
    col_visible = True,  
    alignment   = Qt.AlignLeft,
    width       = MEDIUM_COL_WIDTH) 

ATTR_MODEL_UNICODE = AttributeModel('unicode', 
    doc         = "The unicode representation of the object using the unicode() function.",
    data_fn     = lambda(tree_item): unicode(tree_item.obj),
    col_visible = True,  
    width       = MEDIUM_COL_WIDTH, 
    line_wrap   = QTextOption.WrapAtWordBoundaryOrAnywhere) 

ATTR_MODEL_STR = AttributeModel('str', 
    doc         = "The string representation of the object using the str() function.",
    data_fn     = lambda(tree_item): str(tree_item.obj),
    col_visible = False,  
    width       = MEDIUM_COL_WIDTH, 
    line_wrap   = QTextOption.WrapAtWordBoundaryOrAnywhere) 
 
ATTR_MODEL_REPR = AttributeModel('repr', 
    doc         = "The string representation of the object using the repr() function.", 
    data_fn     = lambda(tree_item): repr(tree_item.obj),         
    col_visible = True,  
    width       = MEDIUM_COL_WIDTH, 
    line_wrap   = QTextOption.WrapAtWordBoundaryOrAnywhere) 

ATTR_MODEL_TYPE = AttributeModel('type', 
    doc         = "Type of the object determined using the builtin type() function", 
    data_fn     = lambda(tree_item): str(type(tree_item.obj)),
    col_visible = False,  
    width       = MEDIUM_COL_WIDTH) 

ATTR_MODEL_CLASS = AttributeModel('type name', 
    doc         = "The name of the class of the object via obj.__class__.__name__", 
    data_fn     = lambda(tree_item): type(tree_item.obj).__name__,
    col_visible = True,  
    width       = MEDIUM_COL_WIDTH) 

ATTR_MODEL_LENGTH = AttributeModel('length', 
    doc         = "The length of the object using the len() function", 
    #data_fn     = tio_length,
    data_fn      = safe_data_fn(len),  
    col_visible = False,  
    alignment   = Qt.AlignRight,
    width       = SMALL_COL_WIDTH) 

ATTR_MODEL_ID = AttributeModel('id', 
    doc         = "The identifier of the object with calculated using the id() function", 
    data_fn     = lambda(tree_item): "0x{:X}".format(id(tree_item.obj)), 
    col_visible = False, 
    alignment   = Qt.AlignRight, 
    width       = SMALL_COL_WIDTH) 

ATTR_MODEL_IS_ATTRIBUTE = AttributeModel('is attribute', 
    doc         = "The object is an attribute of the parent (opposed to e.g. a list element)", 
    data_fn     = tio_is_attribute, 
    col_visible = False,  
    width       = SMALL_COL_WIDTH) 

ATTR_MODEL_CALLABLE = AttributeModel('is callable', 
    doc         = "The if the is callable (e.g. a function or a method)", 
    data_fn     = tio_is_callable, 
    col_visible = True,  
    width       = SMALL_COL_WIDTH) 

ATTR_MODEL_IS_ROUTINE = AttributeModel('is routine', 
    doc         = "True if the object is a routine (function, method, etc.)" ,
    data_fn     = lambda(tree_item): str(inspect.isroutine(tree_item.obj)), 
    col_visible = False,  
    width       = SMALL_COL_WIDTH) 

ATTR_MODEL_PRED = AttributeModel('inspect predicates', 
    doc         = "Predicates from the inspect module" ,
    data_fn     = tio_predicates, 
    col_visible = False,  
    width       = MEDIUM_COL_WIDTH) 

ATTR_MODEL_PRETTY_PRINT = AttributeModel('pretty print', 
    doc         = "Pretty printed representation of the object using the pprint module.", 
    data_fn     = lambda(tree_item): _PRETTY_PRINTER.pformat(tree_item.obj),         
    col_visible = False,  
    width       = MEDIUM_COL_WIDTH) 
        
ATTR_MODEL_DOC_STRING = AttributeModel('doc string', 
    doc         = "The object's doc string", 
    data_fn     = tio_doc_str,         
    col_visible = False,  
    width       = MEDIUM_COL_WIDTH)
        
ATTR_MODEL_GET_DOC = AttributeModel('inspect.getdoc', 
    doc         = "The object's doc string cleaned up by inspect.getdoc()", 
    data_fn     = safe_data_fn(inspect.getdoc),         
    col_visible = False,  
    width       = MEDIUM_COL_WIDTH)
        
ATTR_MODEL_GET_COMMENTS = AttributeModel('inspect.getcomments', 
    doc         = "Comments above the object's definition is retrieved using inspect.getcomments()", 
    data_fn     = lambda(tree_item): inspect.getcomments(tree_item.obj),         
    col_visible = False,  
    width       = MEDIUM_COL_WIDTH)
        
ATTR_MODEL_GET_MODULE = AttributeModel('inspect.getmodule', 
    doc         = "The object's module retrieved using inspect.module", 
    data_fn     = safe_data_fn(inspect.getmodule),         
    col_visible = False,  
    width       = MEDIUM_COL_WIDTH) 
        
ATTR_MODEL_GET_FILE = AttributeModel('inspect.getfile', 
    doc         = "The object's file retrieved using inspect.getfile", 
    data_fn     = safe_data_fn(inspect.getfile),         
    col_visible = False,  
    width       = MEDIUM_COL_WIDTH)
        
ATTR_MODEL_GET_SOURCE_FILE = AttributeModel('inspect.getsourcefile', # calls inspect.getfile()
    doc         = "The object's file retrieved using inspect.getsourcefile", 
    data_fn     = safe_data_fn(inspect.getsourcefile),         
    col_visible = False,  
    width       = MEDIUM_COL_WIDTH)
        
ATTR_MODEL_GET_SOURCE_LINES = AttributeModel('inspect.getsourcelines', 
    doc         = "Uses inspect.getsourcelines() to get a list of source lines for the object", 
    data_fn     = safe_data_fn(inspect.getsourcelines),         
    col_visible = False,  
    width       = MEDIUM_COL_WIDTH)
        
ATTR_MODEL_GET_SOURCE = AttributeModel('inspect.getsource', 
    doc         = "The source code of an object retrieved using inspect.getsource", 
    data_fn     = safe_data_fn(inspect.getsource),         
    col_visible = False,  
    width       = MEDIUM_COL_WIDTH) 
        

ALL_ATTR_MODELS = (
    ATTR_MODEL_NAME,
    ATTR_MODEL_PATH, 
    ATTR_MODEL_SUMMARY,
    ATTR_MODEL_UNICODE, 
    ATTR_MODEL_STR, 
    ATTR_MODEL_REPR,    
    ATTR_MODEL_TYPE, 
    ATTR_MODEL_CLASS, 
    ATTR_MODEL_LENGTH, 
    ATTR_MODEL_ID, 
    ATTR_MODEL_IS_ATTRIBUTE, 
    ATTR_MODEL_CALLABLE, 
    ATTR_MODEL_IS_ROUTINE,     
    ATTR_MODEL_PRED,
    ATTR_MODEL_PRETTY_PRINT,
    ATTR_MODEL_DOC_STRING, 
    ATTR_MODEL_GET_DOC, 
    ATTR_MODEL_GET_COMMENTS, 
    ATTR_MODEL_GET_MODULE, 
    ATTR_MODEL_GET_FILE, 
    ATTR_MODEL_GET_SOURCE_FILE, 
    ATTR_MODEL_GET_SOURCE_LINES, 
    ATTR_MODEL_GET_SOURCE)


DEFAULT_ATTR_COLS = (
    ATTR_MODEL_NAME,
    ATTR_MODEL_PATH, 
    ATTR_MODEL_SUMMARY,
    ATTR_MODEL_UNICODE, 
    ATTR_MODEL_STR, 
    ATTR_MODEL_REPR,    
    ATTR_MODEL_LENGTH, 
    ATTR_MODEL_TYPE, 
    ATTR_MODEL_CLASS, 
    ATTR_MODEL_ID, 
    ATTR_MODEL_IS_ATTRIBUTE,     
    ATTR_MODEL_CALLABLE, 
    ATTR_MODEL_IS_ROUTINE,     
    ATTR_MODEL_PRED,    
    ATTR_MODEL_GET_MODULE, 
    ATTR_MODEL_GET_FILE, 
    ATTR_MODEL_GET_SOURCE_FILE)

DEFAULT_ATTR_DETAILS = (
    ATTR_MODEL_PATH, # to allow for copy/paste  
    #ATTR_MODEL_SUMMARY, # Too similar to unicode column
    ATTR_MODEL_UNICODE, 
    #ATTR_MODEL_STR, # Too similar to unicode column
    ATTR_MODEL_REPR,
    ATTR_MODEL_PRETTY_PRINT,
    #ATTR_MODEL_DOC_STRING, # not used, too similar to ATTR_MODEL_GET_DOC
    ATTR_MODEL_GET_DOC, 
    ATTR_MODEL_GET_COMMENTS, 
    #ATTR_MODEL_GET_MODULE, # not used, already in table 
    ATTR_MODEL_GET_FILE,         
    #ATTR_MODEL_GET_SOURCE_FILE,  # not used, already in table 
    #ATTR_MODEL_GET_SOURCE_LINES, # not used, ATTR_MODEL_GET_SOURCE is better
    ATTR_MODEL_GET_SOURCE)

# Sanity check for duplicates
assert len(ALL_ATTR_MODELS) == len(set(ALL_ATTR_MODELS))
assert len(DEFAULT_ATTR_COLS) == len(set(DEFAULT_ATTR_COLS))
assert len(DEFAULT_ATTR_DETAILS) == len(set(DEFAULT_ATTR_DETAILS))



