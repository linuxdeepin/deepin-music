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
import gio
from operator import attrgetter
from copy import deepcopy

from dtk.ui.combo import ComboBox
from dtk.ui.dialog import ConfirmDialog, DialogBox, DIALOG_MASK_SINGLE_PAGE
from dtk.ui.listview import ListView
from dtk.ui.label import Label
from dtk.ui.button import Button, CheckButton
from dtk.ui.utils import get_content_size
from dtk.ui.menu import Menu
from dtk.ui.entry import InputEntry
from dtk.ui.draw import draw_pixbuf
from dtk.ui.scrolled_window import ScrolledWindow
from dtk.ui.progressbar import ProgressBuffer

from transcoder import Transcoder, FORMATS, TranscodeError
from nls import _
from widget.skin import app_theme
from widget.dialog import WinDir
from widget.ui_utils import (set_widget_left, render_item_text2)
from constant import DEFAULT_FONT_SIZE
from helper import Dispatcher
from player import Player
from song import Song
from library import MediaDB

import utils

class TranscoderJob(gobject.GObject):
    
    __gsignals__ = {
        "end"            : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        "redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
        }
    
    __stop     = False
    __pause    = False
    __is_alive = True
    priority   = 10
    
    def __init__(self, trans_data):
        gobject.GObject.__init__(self)
        self.__updater_id = None
        
        # Init data.
        self.angle = 0
        self.status_icon = app_theme.get_pixbuf("transcoder/wait.png").get_pixbuf()
        self.status_icon_press = app_theme.get_pixbuf("transcoder/wait_press.png").get_pixbuf()
        self.stop_icon = app_theme.get_pixbuf("transcoder/stop.png").get_pixbuf()
        self.progress_ratio = 0.0
        self.skip_flag = False        
        self.trans_data = trans_data
        self.init_transcoder(trans_data)   
        self.__update()        
        self.progress_buffer = ProgressBuffer()

    def start(self):    
        if self.skip_flag:
            self.finish_job()
        else:    
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
        
        self.ext_padding_x = 5
        self.ext_padding_y = 5
        self.ext_w, self.ext_h = get_content_size(self.output_ext, DEFAULT_FONT_SIZE)
        
    def init_transcoder(self, attr):
        self.raw_song = attr["song"]
        self.output_path = attr["output"]
        if os.path.exists(self.output_path) and attr["prompt"]:
            self.exists_prompt = True
        else:    
            self.exists_prompt = False
            
        if self.raw_song.get_path() == self.output_path:
            self.skip_flag = True
                        
        self.output_ext = FORMATS[attr["format"]]["extension"]
        self.transcoder = Transcoder()        
        self.transcoder.set_format(attr["format"])
        self.transcoder.set_quality(attr["quality"])
        if self.raw_song.get_type() == "audiocd":
            self.transcoder.set_cd_input(self.raw_song.get("uri"))
        else:    
            self.transcoder.set_input(attr["song"].get_path())
        self.transcoder.set_output(self.output_path)
        self.transcoder.end_cb = self.__end
        
    def __end(self):    
        try:
            self.write_tags()
        except:    
            pass
        
        self.finish_job()
                
    def finish_job(self):            
        self.emit("end")
        self.__set_status_icon("success")
        self.set_progress_ratio(1.0)
        try:
            gobject.source_remove(self.__updater_id)
        except: pass
        
        if self.trans_data["to_playlist"]:
            tags = {"uri" : utils.get_uri_from_path(self.output_path)}
            song = MediaDB.get_or_create_song(tags, "local", read_from_file=True)
            if song:
                Dispatcher.add_songs([song])
                
    def write_tags(self):    
        tags = deepcopy(self.raw_song)
        tags["uri"] = utils.get_uri_from_path(self.output_path)
        s = Song()
        s.init_from_dict(tags)
        s.write_to_file()
        
    @property    
    def is_running(self):    
        return self.transcoder.running
        
    @property
    def is_finish(self):
        return self.transcoder.is_eos
    
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
                return True
            else:    
                try:
                    gobject.source_remove(self.__updater_id)
                except:    
                    pass
                self.transcoder.pause()
                return True
        return False    
        
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
        render_item_text2(cr, self.title, rect, in_select, in_highlight)
    
    def render_progress(self, cr, rect, in_select, in_highlight):
        self.progress_buffer.progress = self.progress_ratio * 100
        progress_x = rect.x + self.progress_padding_x
        progress_y = rect.y + (rect.height - self.progress_h) / 2
        progress_rect = gtk.gdk.Rectangle(progress_x, progress_y, self.progress_w, self.progress_h)
        self.progress_buffer.render(cr, progress_rect)        
    
    def render_stop(self, cr, rect, in_select, in_highlight):
        icon_x = rect.x + self.stop_icon_padding_x
        icon_y = rect.y + (rect.height - self.stop_icon_h) / 2
        draw_pixbuf(cr, self.stop_icon, icon_x, icon_y)
        
    def render_ext(self, cr, rect, in_select, in_highlight):    
        rect.x += self.ext_padding_x
        rect.width -= self.ext_padding_x * 2
        render_item_text2(cr, self.output_ext.upper(), rect, in_select, in_highlight)
        
    def get_ext_type(self):    
        gio_file = gio.File(self.output_path)
        gio_file_info = gio_file.query_info(",".join([gio.FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                                      gio.FILE_ATTRIBUTE_STANDARD_TYPE, 
                                                      gio.FILE_ATTRIBUTE_STANDARD_NAME,
                                                      gio.FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
                                                      gio.FILE_ATTRIBUTE_STANDARD_SIZE,
                                                      gio.FILE_ATTRIBUTE_STANDARD_ICON,
                                                      gio.FILE_ATTRIBUTE_TIME_MODIFIED,
                                                      gio.FILE_ATTRIBUTE_TIME_CHANGED,]))
        
        info_attr = gio_file_info.get_attribute_as_string(gio.FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE)                
        return gio.content_type_get_description(info_attr)
    
    def get_column_sizes(self):
        return [
            (36, self.status_icon_h + self.status_icon_padding_y * 2),
            (135, self.title_h + self.title_padding_y * 2),
            (120, self.progress_h + self.progress_padding_y * 2),
            # (26, self.stop_icon_h + self.stop_icon_padding_y * 2),
            (50, self.ext_h + self.ext_padding_y),
            ]
    
    def get_renders(self):
        return [ self.render_icon, self.render_title, self.render_progress, self.render_ext]
    
    
class JobsView(ListView):    
    
    def __init__(self, *args, **kwargs):
        ListView.__init__(self, *args, **kwargs)
        del self.keymap["Delete"]
        
        self.connect("right-press-items", self.popup_right_menu)
        self.__jobs = []
        
    def add_job(self, job):    
        job_id = job.connect("end", self.__job_end)
        
        
        self.__jobs.append((job, job_id))
        self.add_items([job])
        
        if len(self.__jobs) == 1:
            try:
                self.__run_check(self.__jobs[0][0])
            except TranscodeError:    
                self.__jobs[0][0].set_error_status()
                self.__job_end_cb(self.__jobs[0][0])
                
    def __run_check(self, ajob):            
        if ajob.exists_prompt and not ajob.skip_flag:
            ConfirmDialog(_("Prompt"), _("Target file already exists, do you want to overwrite it?"),
                          confirm_callback=lambda : ajob.start(),
                          cancel_callback=lambda :self.start_new_job(ajob)
                          ).show_window()
        else:    
            ajob.start()
            
    def start_new_job(self, ajob):        
        self.__job_end(ajob)        
        self.delete_job(ajob)
        
    def __job_end(self, ajob):        
        self.__job_end_cb(ajob)
        
    def __job_end_cb(self, ajob):    
        job, job_id = self.__jobs.pop(0)
        job.disconnect(job_id)
        if self.__jobs:
            try:
                self.__run_check(self.__jobs[0][0])
            except TranscodeError:    
                self.__jobs[0][0].set_error_status()
                self.__job_end_cb(self.__jobs[0][0])
        del job    
        
    def delete_job(self, ajob):    
        if ajob.is_running:
            ajob.force_stop()
            self.__job_end(ajob)
        else:    
            if len(self.__jobs):
                for index, job_item in enumerate(self.__jobs):
                    if job_item[0] == ajob:
                        job_item[0].disconnect(job_item[1])
                        del self.__jobs[index]
                        break
        try:            
            self.delete_items([ajob])
        except:
            pass
        
        
    def delete_finish_jobs(self):    
        new_jobs = filter(attrgetter("is_finish"), self.items)
        if new_jobs:
            self.delete_items(new_jobs)
        
    def playpause_jobs(self):
        if len(self.__jobs) > 0:
            return self.__jobs[0][0].playpause()
        return False
            
    def set_job_priority(self, job):
        job.priority = 0
        
    def popup_right_menu(self, widget, x, y, item, select_items):        
        menu_items = [
            # (None, _("优先转换"), self.set_job_priority, item),
            (None, _("Open directory"), lambda : utils.open_file_directory(item.output_path)),
            None,            
            (None, _("Delete task"), self.delete_job, item),            
            (None, _("Clear finished tasks"), self.delete_finish_jobs)
            ]
        Menu(menu_items, True).show((int(x), int(y)))
        
class TranscoderJobManager(DialogBox):    
    def __init__(self):
        DialogBox.__init__(self, _("Task Manager"), 350, 450, DIALOG_MASK_SINGLE_PAGE,
                           modal=False, close_callback=self.hide_all)
        
        self.is_changed = False
        scrolled_window = ScrolledWindow(0, 0)
        scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        
        scrolled_align = gtk.Alignment()
        scrolled_align.set(1, 1, 1, 1)
        scrolled_align.set_padding(10, 0, 0, 0)
        scrolled_align.add(scrolled_window)
        
        self.jobs_view = JobsView()
        self.jobs_view.draw_mask = self.get_mask_func(self.jobs_view)
        scrolled_window.add_child(self.jobs_view)
        
        pause_button = Button(_("Pause"))
        pause_button.connect("clicked", self.pause_job)
        stop_button = Button(_("Close"))
        stop_button.connect("clicked", lambda w: self.hide_all())
        
        self.body_box.add(scrolled_align)
        self.right_button_box.set_buttons([pause_button, stop_button])
        
        Dispatcher.connect("transfor-job", self.add_new_job)
        
    def add_new_job(self, obj, job):    
        self.visible_it()
        self.idle_add_job(job)
        
    def idle_add_job(self, job):
        gobject.idle_add(self.jobs_view.add_job, job)
        
    def pause_job(self, widget):    
        value = self.jobs_view.playpause_jobs()
        if value:
            if widget.label == _("Pause"):
                widget.label = (_("Continue"))
            else:    
                widget.label = (_("Pause"))
        
    def visible_it(self):    
        if self.get_property("visible"):
            if not self.is_active():
                self.present()
        else:        
            self.show_window()
    
class AttributesUI(DialogBox):
    
    def __init__(self, songs=None):
        DialogBox.__init__(self, _("Converter"), 385, 200, DIALOG_MASK_SINGLE_PAGE,
                           modal=True)
        
        self.songs = songs or [Player.song]
        default_format = "MP3 (CBR)"
        default_index = FORMATS.keys().index(default_format)
        format_box, self.format_combo_box = self.create_combo_widget(_("Format"),
                                                                [(key, None) for key in FORMATS.keys()],
                                                                default_index)
        quality_box, self.quality_combo_box = self.create_combo_widget(_("Quality"),
                                                                       self.get_quality_items(default_format),
                                                                       self.get_quality_index(default_format),
                                                                       65)
        format_quality_box = gtk.HBox(spacing=68)
        format_quality_box.pack_start(format_box, False, False)
        format_quality_box.pack_start(quality_box, False, False)
        
        exists_box, self.exists_combo_box = self.create_combo_widget(_("Target file already exists"),
                                                                [(_("Ask"), True), (_("Cover"), False)],
                                                                0)
        
        start_button = Button(_("Start"))
        close_button = Button(_("Close"))
        self.add_check_button = CheckButton(_("Add to Playlist after finished"), padding_x=2)
        
        main_table = gtk.Table()
        main_table.set_row_spacings(10)
        main_table.attach(format_quality_box, 0, 2, 0, 1, yoptions=gtk.FILL)
        main_table.attach(set_widget_left(exists_box), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(self.create_output_box(), 0, 2, 2, 3, yoptions=gtk.FILL)
        main_table.attach(set_widget_left(self.add_check_button), 0, 2, 3, 4, yoptions=gtk.FILL)
        
        main_align = gtk.Alignment()
        main_align.set_padding(10, 10, 15, 10)
        main_align.add(main_table)
        
        # Connect signals.
        self.format_combo_box.connect("item-selected", self.reset_quality_items)
        start_button.connect("clicked", self.add_and_close)
        close_button.connect("clicked", lambda w: self.destroy())
        
        self.body_box.pack_start(main_align, False, True)
        self.right_button_box.set_buttons([start_button, close_button])
        
    def create_output_box(self):
        output_label = Label("%s:" % _("Output"))
        self.output_entry = InputEntry(os.path.expanduser("~/"))
        self.output_entry.set_size(210, 24)
        change_button = Button(_("Change"))
        change_button.connect("clicked", self.set_output_directory)
        output_box = gtk.HBox(spacing=5)
        output_box.pack_start(output_label, False, False)
        output_box.pack_start(self.output_entry, False, False)
        output_box.pack_start(change_button, False, False)
        return output_box
    
    def set_output_directory(self, widget): 
        directory = WinDir(False).run()
        if directory:
            self.output_entry.set_text(directory)
        
    def get_quality_items(self, name):    
        kbs_steps = [str(key) for key in FORMATS[name]["kbs_steps"]]
        return zip(kbs_steps, FORMATS[name]["raw_steps"])
    
    def get_quality_index(self, name):
        return FORMATS[name]["default_index"]
        
    def create_combo_widget(self, label_content, items, select_index=0, max_width=None):    
        label = Label("%s:" % label_content)
        combo_box = ComboBox(items, select_index=select_index, max_width=max_width)
        
        hbox = gtk.HBox(spacing=5)
        hbox.pack_start(label, False, False)
        hbox.pack_start(combo_box, False, False)
        return hbox, combo_box
    
    def reset_quality_items(self, widget, label, allocated_data, index):
        self.quality_combo_box.add_items(self.get_quality_items(label), clear_first=True)
        self.quality_combo_box.set_select_index(self.get_quality_index(label))
        self.quality_combo_box.queue_draw()
        
    def get_output_location(self, song):    
        ext = FORMATS[self.format_combo_box.get_current_item()[0]]["extension"]        
        if song.get_type() == "audiocd":
            filename = "%s.%s" % (song.get("title"), ext)
        else:
            filename = "%s.%s" % (song.get_filename(), ext)
        return os.path.join(self.output_entry.get_text(), filename)
    
    def add_and_close(self, widget):
        trans_data = {}
        if self.songs:
            for song in self.songs:
                if not song.exists():
                    continue
                trans_data["format"] = self.format_combo_box.get_current_item()[0]
                trans_data["quality"] = self.quality_combo_box.get_current_item()[1]
                trans_data["song"] = song
                trans_data["output"] = self.get_output_location(song)
                trans_data["prompt"] = self.exists_combo_box.get_current_item()[1]
                trans_data["to_playlist"] = self.add_check_button.get_active()
                Dispatcher.transfor_job(TranscoderJob(trans_data))
            self.destroy()
        
convert_task_manager =TranscoderJobManager()
