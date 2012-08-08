#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 ~ 2012 Deepin, Inc.
#               2011 ~ 2012 Hou Shaohui
# 
# Author:     Hou Shaohui <houshao55@gmail.com>
# Maintainer: Hou Shaohui <houshao55@gmail.com>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import gtk
import gobject
import threading

from dtk.ui.combo import ComboBox
from dtk.ui.dialog import DialogBox, DIALOG_MASK_SINGLE_PAGE
from dtk.ui.listview import ListView
from dtk.ui.label import Label
from dtk.ui.button import Button, CheckButton
from dtk.ui.entry import InputEntry

from transcoder import Transcoder, FORMATS
from nls import _
from widget.ui_utils import (create_separator_box,
                             create_right_align)

class TranscoderJob(gobject.GObject):
    
    __gsignals__ = {
        "end"            : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, tuple()),
        "redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        }
    
    __stop     = False
    __pause    = False
    __is_alive = True
    priority   = 0
    
    def __init__(self):
        super(TranscoderJob, self).__init__(self)
        
        # Audio converter thread.
        self.__thread = threading.Thread(target=self.__run)
        self.__thread.setDaemon(True)
        self.__condition = threading.Condition()
        self.transcoder = Transcoder()
        
    def __run(self):    
        for p in self.job():
            self.__condition.acquire()
            while self.__pause:
                self.__condition.wait()
            
            if self.__stop:    
                self.__condition.release()
                break
            self.__condition.release()
            
        self.__condition.acquire()    
        self.on_stop()
        self.__is_alive = False
        self.emit("end")
        self.__condition.release()
    
    def job(self):
        self.transcoder.set_format()
        self.transcoder.set_quality()
    
    def on_start(self):
        pass
    
    def start(self):
        self.on_start()
        self.__thread.start()
        
    def pause(self):    
        self.__condition.acquire()
        self.__pause = False
        self.__condition.notify()
        self.__condition.release()
        
    def unpuase(self):    
        self.__condition.acquire()
        self.__pause = True
        self.__condition.notify()
        self.__condition.release()

    def on_stop(self):    
        pass
    
    def stop(self):
        self.condition.acquire()
        self.__stop = True
        self.condition.release()
        
    # widget.    
    def set_index(self, index):
        '''Update index.'''
        self.index = index
        
    def get_index(self):
        '''Get index.'''
        return self.index
        
    def emit_redraw_request(self):
        '''Emit redraw-request signal.'''
        self.emit("redraw-request")


class JobsView(ListView):        
    
    def __init__(self):
        super(ListView, self).__init__()
        self.__job_id = None
        self.active_job = None
        
    def add_jobs(self, jobs):    
        self.add_items(jobs)
        if not self.active_job:
            self.start_new_job()
        
    def __job_end(self, ajob):    
        self.active_job.disconnect(self.__job_id)
        self.start_new_job()
        
    def start_new_job(self):                    
        unfinished_jobs = self.get_unfinished_jobs()
        if unfinished_jobs:
            new_job = unfinished_jobs[0]
            self.__job_id = new_job.connect("end", self.__job_end)
            new_job.start()
            self.active_job = new_job

    def get_unfinished_jobs(self):
        pass
    
    
class Attributes(DialogBox):
    
    def __init__(self):
        DialogBox.__init__(self, _("Transcoder"), 460, 300, DIALOG_MASK_SINGLE_PAGE,
                           modal=True)
        self.body_box.set_spacing(10)
        self.body_box.pack_start(self.create_format_box(), False, True)
        self.body_box.pack_start(self.create_option_box(), False, True)
        
    def create_format_box(self):    
        title_label = Label(_("编码格式"))
        default_format = FORMATS.keys()[0]
        format_box, self.format_combo_box = self.create_combo_widget(_("输出格式:"), 
                                                                     [(key, None) for key in FORMATS.keys()],
                                                                     0)
        quality_box, self.quality_combo_box = self.create_combo_widget(_("质量:"),
                                                                       self.get_items_from_name(default_format),
                                                                       FORMATS[default_format]["default_index"])
        # desc_label = Label(FORMATS[default_format]["desc"], enable_select=False, wrap_width=420, text_size=9)
        
        main_table = gtk.Table(3, 2)
        main_table.set_row_spacings(8)
        main_table.attach(title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=20)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(format_box, 0, 1, 2, 3, yoptions=gtk.FILL, xpadding=30)
        main_table.attach(quality_box, 1, 2, 2, 3, yoptions=gtk.FILL)
        return main_table
    
    def create_option_box(self):
        title_label = Label(_("选项设置"))
        output_label = Label(_("目标文件夹:"))
        output_entry = InputEntry("~/")
        output_entry.set_size(250, 26)
        change_button = Button(_("更改"))
        output_box = gtk.HBox(spacing=5)
        output_box.pack_start(output_label, False, False)
        output_box.pack_start(output_entry, False, False)
        output_box.pack_start(change_button, False, False)
        
        priority_box, priority_combo_box = self.create_combo_widget("优先级:",
                                                                    [(key, None) for key in "low middle heigh".split()],
                                                                    1)
        exist_box, exist_combo_box = self.create_combo_widget("当目标文件存在时:",
                                                              [(key, None) for key in "ask cover".split()],
                                                              0)
        finish_check_button = CheckButton(_("Add to playlist"))
        
        main_table = gtk.Table(4, 2)
        main_table.set_row_spacings(8)
        main_table.attach(title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=20)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)

        main_table.attach(priority_box, 0, 1, 2, 3, yoptions=gtk.FILL, xpadding=30)
        main_table.attach(exist_box, 1, 2, 2, 3, yoptions=gtk.FILL)
        main_table.attach(output_box, 0, 2, 3, 4, yoptions=gtk.FILL, xpadding=30)        
        main_table.attach(finish_check_button, 0, 1, 4, 5, yoptions=gtk.FILL, xpadding=28)
        return main_table
        
    def get_items_from_name(self, name):    
        return [(str(key), None) for key in FORMATS[name]["raw_steps"]]
        
    def create_combo_widget(self, label_content, items, select_index=0):    
        label = Label(label_content)
        combo_box = ComboBox(items, select_index=select_index)
        
        hbox = gtk.HBox(spacing=5)
        hbox.pack_start(label, False, False)
        hbox.pack_start(combo_box, False, False)
        return hbox, combo_box
