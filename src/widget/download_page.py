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
from dtk.ui.new_treeview import TreeView

from ui.download_item import TaskItem

from helper import event_manager

class TaskView(TreeView):
    
    def __init__(self):
        TreeView.__init__(self, enable_drag_drop=False, enable_multiple_select=False)
        event_manager.add_callback("download-images", self.on_download_images)        
        
    def get_images(self):    
        return map(lambda item: item.image_object, self.get_items())
    
    def has_image(self, image):
        return image in self.get_images() or os.path.exists(image.get_save_path())
    
    def on_download_images(self, name, obj, data):
        self.add_images(data)
        
    def emit_task_number(self):    
        event_manager.emit("downloading-tasks-number", len(self.get_items()))
        
    def add_images(self, images):    
        filter_images = filter(lambda image: not self.has_image(image), images)
        if filter_images:
            items = []
            for image in filter_images:
                items.append(TaskItem(image, self.on_download_finish))
            self.add_items(items)
            self.emit_task_number()
            
            
    def on_download_finish(self, item):        
        self.delete_items([item])
        self.emit_task_number()
        event_manager.emit("download-image-finish", item.image_object.get_save_path())
        
    def draw_mask(self, cr, x, y, w, h):
        cr.set_source_rgb(1, 1, 1)
        cr.rectangle(x, y, w, h)
        cr.fill()
        

class TaskPage(gtk.VBox):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        
        self.task_view = TaskView()
        self.task_view.set_size_request(650, 450)
        self.add(self.task_view)
