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

from osd_window import lrc_window
from lrcparser import lrcparser
from player import Player
import gtk
import gobject

class ToolWindow(object):
    
    def __init__(self, filename):
        self.lrc_file = filename
        lrcparser.set_lrc_file(self.lrc_file)
        self.half = False
        self.full = False

        
    def parser_current_lrc(self):    
        current_pos = Player.get_position() * 100
        self.c_start, self.c_total, self.c_lrc = lrcparser.get_current_content(current_pos)

    def parser_next_lrc(self):    
        current_pos = Player.get_position() * 100
        self.n_start, self.n_total, self.n_lrc = lrcparser.get_next_content(current_pos)
        
    def parser_process(self):    
        self.parser_current_lrc()
        current_pos = Player.get_position() * 100
        if current_pos >= self.c_start + (self.c_total) / 2:
            self.half = True
        elif current_pos >= self.c_start + self.c_total:    
            self.full = True
        else:    
            self.half = False
            self.full = False

        self.c_process =  (current_pos - self.c_start) / float(self.c_total)
        print self.c_process
    
    def init_lrc(self):
        self.parser_current_lrc()
        self.parser_next_lrc()
        lrc_window.set_lyric(0, self.c_lrc)
        lrc_window.set_lyric(1, self.n_lrc)
        self.current_line = 0
        
    def get_next_line(self):    
        if self.current_line == 0:
            next_line = 1
        else:    
            next_line = 0
            
        return next_line    
    
    def update_active(self):
        self.parser_process()
        self.parser_next_lrc()
        format_p  = "%0.1f" % self.c_process
        lrc_window.set_percentage(self.current_line, self.c_process)
        if self.half:
            lrc_window.set_lyric(self.get_next_line(), self.n_lrc)
        elif self.full:
            self.parser_current_lrc()
            self.current_line = self.get_next_line()
            self.parser_process()
            lrc_window.set_percentage(self.current_line, self.c_process)
        return True    

        
    def run(self):        
        self.init_lrc()
        gobject.timeout_add(50, self.update_active)                
        # lrc_window.window.connect("expose-event", self.update_active)        
        lrc_window.window.show_all()

        
if __name__ == "__main__":        
    lrc_win = ToolWindow('/home/vicious/.lyrics/王麟-伤不起.lrc')    
    lrc_win.run()
            
            
            
            
        
        
    
    
    
    
        
        

        
        
        