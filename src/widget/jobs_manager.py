#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011 Deepin, Inc.
#               2011 Hou Shaohui
#
# Author:     Hou Shaohui <houshao55@gmail.com>
# Maintainer: Hou ShaoHui <houshao55@gmail.com>
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
from dtk.ui.button import ImageButton, ToggleButton
from dtk.ui.label import Label

from logger import Logger
from widget.skin import app_theme
from widget.ui_utils import draw_alpha_mask
from helper import Dispatcher
from nls import _

class JobsManager(gtk.HBox):
    __jobs = []
    __id_updater = None
    
    def __init__(self):
        super(JobsManager,self).__init__(spacing=6)
        self.connect("expose-event", self.draw_bg_mask)
        
        self.jobs_label = Label("0 " + _("jobs waiting!"), app_theme.get_color("labelText"), 8)
        self.jobs_label.set_size_request(150, 12)
        label_align = gtk.Alignment()
        label_align.set(0.5, 0.5, 0, 0)
        label_align.set_padding(0, 0, 10, 0)
        label_align.add(self.jobs_label)
        
        self.progress_label = Label("", app_theme.get_color("labelText"), 8)
        self.progress_label.set_size_request(500, 10)
        self.__paused = False
        btn_cancel = self.__create_simple_button("stop", self.stop)
        self.__btn_pause = self.__create_begin_button(self.pause)
        
        btn_pause_align = gtk.Alignment()
        btn_pause_align.set(0.5, 0.5, 0, 0)
        btn_pause_align.add(self.__btn_pause)
        
        btn_cancel_align = gtk.Alignment()
        btn_cancel_align.set(0.5, 0.5, 0, 0)
        btn_cancel_align.set_padding(0, 0, 0, 10)
        btn_cancel_align.add(btn_cancel)
        
        self.pack_start(label_align, False, False)
        # self.pack_start(self.throbber, False, False)
        self.pack_start(self.progress_label,True,True)
        self.pack_start(btn_pause_align,False,False)
        self.pack_start(btn_cancel_align,False,False)
        self.show_all()
        self.set_no_show_all(True)
        self.hide()
        
        self.jobs_label.hide_all()
        
    def draw_bg_mask(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_alpha_mask(cr, rect.x, rect.y, rect.width, rect.height, "frameLight")
        
    def __create_simple_button(self, name, callback):    
        button = ImageButton(
            app_theme.get_pixbuf("jobs/%s_normal.png" % name),
            app_theme.get_pixbuf("jobs/%s_hover.png" % name),
            app_theme.get_pixbuf("jobs/%s_hover.png" % name),
            )
        if callback:
            button.connect("clicked", callback) 
        return button    
        
    def __create_begin_button(self, callback):    
        toggle_button = ToggleButton(
            app_theme.get_pixbuf("jobs/pause_normal.png"),            
            app_theme.get_pixbuf("jobs/begin_normal.png"),
            app_theme.get_pixbuf("jobs/pause_hover.png"),
            app_theme.get_pixbuf("jobs/begin_hover.png")            
            )
        if callback:
            toggle_button.connect("toggled", callback)
        return toggle_button    

    def add(self, job):
        job_id = job.connect("end",self.__job_end)
        self.__jobs.append((job, job_id))
        if len(self.__jobs) == 1:
            try: gobject.source_remove(self.__id_updater)
            except: pass
            self.__id_updater = gobject.timeout_add(250,self.__update)
            self.__jobs[0][0].start()
            if self.__paused:
                self.pause(self.__btn_pause)
            self.__update()

    def __job_end(self, ajob):
        gobject.idle_add(self.__job_end_cb, ajob)

    def __job_end_cb(self, ajob):
        job, job_id = self.__jobs.pop(0)
        job.disconnect(job_id)
        if self.__paused:
            self.pause(self.__btn_pause)
        if self.__jobs:
            jobs = [ (job[0].priority, job) for job in self.__jobs ]
            jobs.sort()
            self.__jobs = [ job[1] for job in jobs ]
            self.__jobs[0][0].start()
            self.__update()
        else:
            try: gobject.source_remove(self.__id_updater)
            except:pass
            self.__update()
        del job

    def pause(self, btn):
        if self.__jobs:
            if not self.__paused:
                self.__jobs[0][0].pause()
                self.__paused = True
            else:
                self.__jobs[0][0].unpause()
                self.__paused = False

    def stop(self,*args):
        if self.__jobs:
            if self.__paused:
                self.pause(self.__btn_pause)
                
            self.__jobs[0][0].stop()

    def __update(self):
        if len(self.__jobs)-1 > 0 :
            self.jobs_label.set_text("%d "% (len(self.__jobs)-1) + _("jobs waiting!"))
            self.jobs_label.show_all()
        else:    
            self.jobs_label.hide_all()

        if self.__jobs:
            Dispatcher.show_jobs()            
            message = self.__jobs[0][0].get_info()
            self.progress_label.set_text(message)
            self.show()
            return True
        else:
            Dispatcher.hide_jobs()            
            self.hide()
            self.__id_updater = None
            return False

jobs_manager = JobsManager()

class Job(gobject.GObject,Logger):
    __gsignals__ = { 
        "end" : (gobject.SIGNAL_RUN_LAST,
                gobject.TYPE_NONE,
                tuple())
    }
    __percent = 0
    __message = ""
    __pulse = False
    __stop = False
    __pause = False
    __is_alive = True
    __func = None
    __args = None
    priority = 0
    
    def __init__(self):
        super(Job,self).__init__()

        self.__thread = threading.Thread(target=self.__run)
        self.__thread.setDaemon(True)
        self.__cond = threading.Condition()
        jobs_manager.add(self)
        
    def is_alive(self):
        return self.__is_alive
    
    def start(self):
        self.on_start()
        self.__thread.start()

    def on_start(self):
        pass

    def unpause(self):
        self.__cond.acquire()
        self.__pause = False
        self.__cond.notify()
        self.__cond.release()
        
    def pause(self):
        self.__cond.acquire()
        self.__pause = True
        self.__cond.notify()
        self.__cond.release()
        
    def stop(self):
        self.__cond.acquire()
        self.__stop = True
        self.__cond.release()

    def on_stop(self):
        pass

    def get_info(self):
        self.__cond.acquire()
        info = self.__message
        self.__cond.release()
        return info
    
    def job(self):
        """ job to execute must yield  text, percent and pulse on each iteration """
        pass

    def __run(self):
        for text in self.job():
            self.__cond.acquire()
            while self.__pause:
                self.__cond.wait()
            self.__message = text

            if self.__stop:
                self.__cond.release()
                break
            self.__cond.release()

        self.__cond.acquire()
        self.on_stop()
        self.__is_alive = False
        self.emit("end")
        self.__cond.release()
