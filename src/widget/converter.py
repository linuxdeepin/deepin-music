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

import gobject
import threading

from dtk.ui.listview import ListView

from transcoder import Transcoder

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
        if not self.has_active_job:
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