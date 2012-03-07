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
import pango
import gobject
import threading

from logger import Logger

class JobsManager(gtk.HBox):
    __jobs = []
    __id_updater = None
    def __init__(self):
        super(JobsManager,self).__init__(spacing=6)

        self.progress = gtk.ProgressBar()
        self.progress.set_text("")
        self.progress.set_fraction(0)
        self.progress.set_pulse_step(0.1)
        self.progress.set_ellipsize(pango.ELLIPSIZE_START)
        self.progress.set_size_request(-1,22)
        
        self.label = gtk.Label("0 operation(s) pending")

        self.__btn_size = gtk.ICON_SIZE_BUTTON
        size = (28,28)
        
        self.__paused = False

        self.__btn_pause= gtk.Button()
        self.__btn_pause.connect("clicked",self.pause)
        image = gtk.image_new_from_stock(gtk.STOCK_MEDIA_PAUSE,self.__btn_size)
        self.__btn_pause.add(image)
        self.__btn_pause.set_relief(gtk.RELIEF_NONE)
        self.__btn_pause.set_size_request(*size)
 
        btn_cancel= gtk.Button()
        btn_cancel.connect("clicked",self.stop)
        
        image = gtk.image_new_from_stock(gtk.STOCK_MEDIA_STOP,self.__btn_size)
        btn_cancel.add(image)
        btn_cancel.set_relief(gtk.RELIEF_NONE)
        btn_cancel.set_size_request(*size)
        
        self.pack_start(self.label,False,False)
        self.pack_start(self.progress,True,True)
        self.pack_start(self.__btn_pause,False,False)
        self.pack_start(btn_cancel,False,False)
        self.show_all()
        self.set_no_show_all(True)
        self.hide()
        
        self.label.hide_all()
        

    def add(self,job):
        id = job.connect("end",self.__job_end)
        self.__jobs.append((job,id))
        if len(self.__jobs) == 1:
            try: gobject.source_remove(self.__id_updater)
            except: pass
            self.__id_updater = gobject.timeout_add(500,self.__update)
            self.__jobs[0][0].start()
            if self.__paused:
                self.pause(self.__btn_pause)
            self.__update()

    def __job_end(self,ajob):
        gobject.idle_add(self.__job_end_cb,ajob)

    def __job_end_cb(self,ajob):
        job, id = self.__jobs.pop(0)
        job.disconnect(id)
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

    def pause(self,btn):
        if self.__jobs:
            if not self.__paused:
                btn.child.set_from_stock(gtk.STOCK_MEDIA_PLAY,self.__btn_size)
                self.__jobs[0][0].pause()
                self.__paused = True
            else:
                btn.child.set_from_stock(gtk.STOCK_MEDIA_PAUSE,self.__btn_size)
                self.__jobs[0][0].unpause()
                self.__paused = False

    def stop(self,*args):
        if self.__jobs:
            if self.__paused:
                self.pause(self.__btn_pause)
                
            self.__jobs[0][0].stop()

    def __update(self):
        if len(self.__jobs)-1 > 0 :
            self.label.set_text("%d "% (len(self.__jobs)-1) + "operation(s) pending")
            self.label.show_all()
        else:
            self.label.hide_all()

        if self.__jobs:
            percent, message, pulse = self.__jobs[0][0].get_info()
            if pulse:
                """self.progress.set_pulse_step(0.1)"""
                self.progress.pulse()
            else:
                self.progress.set_fraction(percent)
            self.progress.set_text(message)
            self.show()
            return True
        else:
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
        info = ( self.__percent , self.__message, self.__pulse)
        self.__cond.release()
        return info
    
    def job(self):
        """ job to execute must yield  text, percent and pulse on each iteration """
        pass

    def __run(self):
        for text, percent, pulse in self.job():
            self.__cond.acquire()
            while self.__pause:
                self.__cond.wait()
            self.__message = text
            self.__percent = percent
            self.__pulse = pulse
            if self.__stop:
                self.__cond.release()
                break
            self.__cond.release()

        self.__cond.acquire()
        self.on_stop()
        self.__is_alive = False
        self.emit("end")
        self.__cond.release()
