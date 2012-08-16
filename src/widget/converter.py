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

import os
import gtk
import gobject
import pango
import math

from dtk.ui.combo import ComboBox
from dtk.ui.dialog import DialogBox, DIALOG_MASK_SINGLE_PAGE
from dtk.ui.listview import ListView
from dtk.ui.label import Label
from dtk.ui.button import Button, CheckButton
from dtk.ui.utils import get_content_size
from dtk.ui.entry import InputEntry
from dtk.ui.draw import draw_pixbuf, cairo_disable_antialias, draw_text
from dtk.ui.scrolled_window import ScrolledWindow

from transcoder import Transcoder, FORMATS, TranscodeError
from nls import _
from widget.skin import app_theme
from widget.ui_utils import (set_widget_left, render_item_text)
from constant import DEFAULT_FONT_SIZE
from helper import Dispatcher
from player import Player

class TranscoderJob(gobject.GObject):
    
    __gsignals__ = {
        "end"            : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, tuple()),
        "redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        }
    
    __stop     = False
    __pause    = False
    __is_alive = True
    priority   = 0
    
    def __init__(self, trans_data):
        gobject.GObject.__init__(self)
        self.__updater_id = None
        
        # Init data.
        self.angle = 0
        self.status_icon = app_theme.get_pixbuf("transcoder/wait.png").get_pixbuf()
        self.status_icon_press = app_theme.get_pixbuf("transcoder/wait_press.png").get_pixbuf()
        self.stop_icon = app_theme.get_pixbuf("transcoder/stop.png").get_pixbuf()
        # self.stop_icon_press = app_theme.get_pixbuf("transcoder/stop_press.png").get_pixbuf()
        self.progress_ratio = 0.0
        self.trans_data = trans_data
        self.init_transcoder(trans_data)   
        self.__update()        
        
    def start(self):    
        self.transcoder.start_transcode()
        self.__set_status_icon("working")
        self.__updater_id = gobject.timeout_add(500, self.update_progress)
        
    def update_progress(self):    
        self.set_progress_ratio(self.transcoder.get_ratio())
        return True
    
    def __update(self):    
        self.title = self.trans_data["song"].get_str("title")
        
        self.status_icon_padding_x = 10
        self.status_icon_padding_y = 5
        self.status_icon_w, self.status_icon_h = (self.status_icon.get_width(), self.status_icon.get_height())
        
        self.title_padding_x = 5
        self.title_padding_y = 5
        self.title_w, self.title_h = get_content_size(self.title, DEFAULT_FONT_SIZE)
        
        self.progress_padding_x = 10
        self.progress_padding_y = 5
        self.progress_w, self.progress_h = 100, 10
        
        self.stop_icon_padding_x = 5
        self.stop_icon_padding_y = 5
        self.stop_icon_w, self.stop_icon_h = (self.stop_icon.get_width(), self.stop_icon.get_height())
        
    def init_transcoder(self, attr):
        self.output_path = attr["output"]
        self.transcoder = Transcoder()        
        self.transcoder.set_format(attr["format"])
        self.transcoder.set_quality(attr["quality"])
        self.transcoder.set_input(attr["song"].get_path())
        self.transcoder.set_output(self.output_path)
        self.transcoder.end_cb = self.__end
        
    def __end(self):    
        self.emit("end")
        self.__set_status_icon("success")
        self.set_progress_ratio(1.0)
        try:
            gobject.source_remove(self.__updater_id)
        except: pass
        
        
    def force_stop(self):    
        try:
            gobject.source_remove(self.__updater_id)
        except: pass    
        
        if self.transcoder.running:
            self.transcoder.pause()
        try:    
            os.unlink(self.output_path)
        except: pass

        
    def playpause(self):    
        if self.transcoder.running:
            if self.transcoder.is_pause:
                self.transcoder.playing()
                self.__updater_id = gobject.timeout_add(500, self.update_progress)
            else:    
                try:
                    gobject.source_remove(self.__updater_id)
                except:    
                    pass
                self.transcoder.pause()
        
    def set_progress_ratio(self, value):    
        self.progress_ratio = value
        self.emit_redraw_request()
        
    def set_index(self, index):
        '''Update index.'''
        self.index = index
        
    def get_index(self):
        '''Get index.'''
        return self.index
    
    def __set_status_icon(self, name):
        self.status_icon = app_theme.get_pixbuf("transcoder/%s.png" % name).get_pixbuf()
        self.status_icon_press = app_theme.get_pixbuf("transcoder/%s_press.png" % name).get_pixbuf()
        self.emit_redraw_request()
        
    def set_error_status(self):    
        self.__set_status_icon("error")
        
    def emit_redraw_request(self):
        '''Emit redraw-request signal.'''
        self.emit("redraw-request")
        
    def render_icon(self, cr, rect, in_select, in_highlight):    
        icon_x = rect.x + self.status_icon_padding_x
        icon_y = rect.y + (rect.height - self.status_icon_h) / 2
        if in_select:
            draw_pixbuf(cr, self.status_icon_press, icon_x, icon_y)
        else:    
            draw_pixbuf(cr, self.status_icon, icon_x, icon_y)
    
    def render_title(self, cr, rect, in_select, in_highlight):
        rect.x += self.title_padding_x
        rect.width -= self.title_padding_x * 2
        render_item_text(cr, self.title, rect, in_select, in_highlight)
    
    def render_progress(self, cr, rect, in_select, in_highlight):
        progress_x = rect.x + self.progress_padding_x
        progress_y = rect.y + (rect.height - self.progress_h) / 2
        with cairo_disable_antialias(cr):
            cr.set_line_width(1)
            # if in_select:
            #     cr.set_source_rgb(1, 1, 1)
            # else:    
            cr.set_source_rgb(0.4, 0.4, 0.4)
            cr.rectangle(progress_x, progress_y, self.progress_w, self.progress_h)
            cr.stroke()
            
            cr.set_source_rgb(1, 1, 1)
            cr.rectangle(progress_x, progress_y, self.progress_w - 1, self.progress_h - 1)
            cr.fill()
            
            cr.set_source_rgb(0.4, 0.8, 0.2)
            cr.rectangle(progress_x , progress_y ,(self.progress_w - 1) * self.progress_ratio, self.progress_h - 1)
            cr.fill()
            
        # if in_select:    
        #     text_color = "#ffffff"
        # else:    
        #     text_color = "#000000"
        draw_text(cr, str(int(self.progress_ratio * 100)) + "%", progress_x, progress_y, self.progress_w, self.progress_h,
                  7, alignment=pango.ALIGN_CENTER)    
    
    def render_stop(self, cr, rect, in_select, in_highlight):
        icon_x = rect.x + self.stop_icon_padding_x
        icon_y = rect.y + (rect.height - self.stop_icon_h) / 2
        # if in_select:
        #     draw_pixbuf(cr, self.stop_icon_press, icon_x, icon_y)
        # else:    
        draw_pixbuf(cr, self.stop_icon, icon_x, icon_y)
    
    def get_column_sizes(self):
        return [
            (36, self.status_icon_h + self.status_icon_padding_y * 2),
            (120, self.title_h + self.title_padding_y * 2),
            (155, self.progress_h + self.progress_padding_y * 2),
            (26, self.stop_icon_h + self.stop_icon_padding_y * 2)
            ]
    
    def get_renders(self):
        return [ self.render_icon, self.render_title, self.render_progress, self.render_stop]
    
    
class JobsView(ListView):    
    
    def __init__(self, *args, **kwargs):
        ListView.__init__(self, *args, **kwargs)
        del self.keymap["Delete"]
        self.__jobs = []
        
    def add_job(self, job):    
        job_id = job.connect("end", self.__job_end)
        
        self.__jobs.append((job, job_id))
        self.add_items([job])
        
        if len(self.__jobs) == 1:
            try:
                self.__jobs[0][0].start()
            except TranscodeError:    
                self.__jobs[0][0].set_error_status()
                self.__job_end_cb(self.__jobs[0][0])
        
    def __job_end(self, ajob):        
        gobject.idle_add(self.__job_end_cb, ajob)
        
    def __job_end_cb(self, ajob):    
        job, job_id = self.__jobs.pop(0)
        job.disconnect(job_id)
        if self.__jobs:
            jobs = [(job[0].priority, job) for job in self.__jobs]
            jobs.sort()
            self.__jobs = [ job[1] for job in jobs ]
            try:
                self.__jobs[0][0].start()
            except TranscodeError:    
                self.__jobs[0][0].set_error_status()
                self.__job_end_cb(self.__jobs[0][0])
        del job    
        
    def delete_job(self, ajob):    
        pass
    
    def playpause_jobs(self):
        if len(self.__jobs) > 0:
            self.__jobs[0][0].playpause()
        
class TranscoderJobManager(DialogBox):    
    def __init__(self):
        DialogBox.__init__(self, _("转换任务列表"), 350, 450, DIALOG_MASK_SINGLE_PAGE,
                           modal=False, close_callback=self.hide_all)
        
        scrolled_window = ScrolledWindow(0, 0)
        scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        
        scrolled_align = gtk.Alignment()
        scrolled_align.set(1, 1, 1, 1)
        scrolled_align.set_padding(10, 0, 0, 0)
        scrolled_align.add(scrolled_window)
        
        self.jobs_view = JobsView()
        self.jobs_view.draw_mask = self.get_mask_func(self.jobs_view)
        scrolled_window.add_child(self.jobs_view)
        
        pause_button = Button("暂停转换")
        pause_button.connect("clicked", self.pause_job)
        stop_button = Button("终止转换")
        
        self.body_box.add(scrolled_align)
        self.right_button_box.set_buttons([pause_button, stop_button])
        
        Dispatcher.connect("transfor-job", self.add_new_job)
        
    def add_new_job(self, obj, job):    
        if self.get_property("visible"):
            if not self.is_active():
                self.present()
        else:        
            self.show_window()
        self.jobs_view.add_job(job)    
        
    def pause_job(self, widget):    
        self.jobs_view.playpause_jobs()
            
    
class Attributes(DialogBox):
    
    def __init__(self, song=None):
        DialogBox.__init__(self, _("转换格式"), 385, 200, DIALOG_MASK_SINGLE_PAGE,
                           modal=True)
        
        self.song = song or Player.song
        default_format = "MP3 (VBR)"
        default_index = FORMATS.keys().index(default_format)
        format_box, self.format_combo_box = self.create_combo_widget(_("输出格式:"),
                                                                [(key, None) for key in FORMATS.keys()],
                                                                default_index)
        quality_box, self.quality_combo_box = self.create_combo_widget(_("输出质量:"),
                                                                       self.get_quality_items(default_format),
                                                                       self.get_quality_index(default_format),
                                                                       65)
        format_quality_box = gtk.HBox(spacing=68)
        format_quality_box.pack_start(format_box, False, False)
        format_quality_box.pack_start(quality_box, False, False)
        
        exists_box, exists_combo_box = self.create_combo_widget(_("已存在时:"),
                                                                [(key, None) for key in "询问 覆盖".split()],
                                                                0)
        
        start_button = Button(_("开始转换"))
        
        main_table = gtk.Table()
        main_table.set_row_spacings(10)
        main_table.attach(format_quality_box, 0, 2, 0, 1, yoptions=gtk.FILL)
        main_table.attach(set_widget_left(exists_box), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(self.create_output_box(), 0, 2, 2, 3, yoptions=gtk.FILL)
        
        main_align = gtk.Alignment()
        main_align.set_padding(10, 10, 15, 10)
        main_align.add(main_table)
        
        # Init data.
        self.trans_data = {}
        
        # Connect signals.
        self.format_combo_box.connect("item-selected", self.reset_quality_items)
        start_button.connect("clicked", self.add_and_close)
        
        self.body_box.pack_start(main_align, False, True)
        self.right_button_box.set_buttons([start_button])
        
    def create_output_box(self):
        output_label = Label(_("输出目录:"))
        self.output_entry = InputEntry(os.path.expanduser("~/"))
        self.output_entry.set_size(210, 24)
        change_button = Button(_("更改"))
        output_box = gtk.HBox(spacing=5)
        output_box.pack_start(output_label, False, False)
        output_box.pack_start(self.output_entry, False, False)
        output_box.pack_start(change_button, False, False)
        return output_box
        
    def get_quality_items(self, name):    
        kbs_steps = [str(key) for key in FORMATS[name]["kbs_steps"]]
        return zip(kbs_steps, FORMATS[name]["raw_steps"])
    
    def get_quality_index(self, name):
        return FORMATS[name]["default_index"]
        
    def create_combo_widget(self, label_content, items, select_index=0, max_width=None):    
        label = Label(label_content)
        combo_box = ComboBox(items, select_index=select_index, max_width=max_width)
        
        hbox = gtk.HBox(spacing=5)
        hbox.pack_start(label, False, False)
        hbox.pack_start(combo_box, False, False)
        return hbox, combo_box
    
    def reset_quality_items(self, widget, label, allocated_data, index):
        self.quality_combo_box.set_items(self.get_quality_items(label))
        self.quality_combo_box.set_select_index(self.get_quality_index(label))
        self.quality_combo_box.queue_draw()
        
    def get_output_location(self):    
        ext = FORMATS[self.format_combo_box.get_current_item()[0]]["extension"]
        filename = "%s.%s" % (self.song.get_filename(), ext)
        return os.path.join(self.output_entry.get_text(), filename)
    
    def add_and_close(self, widget):
        self.trans_data["format"] = self.format_combo_box.get_current_item()[0]
        self.trans_data["quality"] = self.quality_combo_box.get_current_item()[1]
        self.trans_data["priority"] = 0
        self.trans_data["song"] = self.song
        self.trans_data["output"] = self.get_output_location()
        Dispatcher.transfor_job(TranscoderJob(self.trans_data))
        self.destroy()
        
transcoder_job_manager =TranscoderJobManager()
