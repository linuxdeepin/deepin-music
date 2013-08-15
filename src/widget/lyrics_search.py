#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011~2012 Deepin, Inc.
#               2011~2012 Hou Shaohui
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
from dtk.ui.button import Button
from dtk.ui.entry import InputEntry
from dtk.ui.utils import get_content_size
from dtk.ui.dialog import DialogBox, DIALOG_MASK_MULTIPLE_PAGE
from dtk.ui.threads import post_gui
from dtk.ui.label import Label

from dtk.ui.treeview import TreeView, TreeItem
from widget.ui_utils import render_item_text, draw_single_mask, draw_alpha_mask
from lrc_download import TTPlayer, DUOMI, TTPod
from cover_query import poster
from helper import Dispatcher
from constant import DEFAULT_FONT_SIZE
from lrc_manager import LrcManager
from player import Player
from config import config
from nls import _
import utils


class SearchUI(DialogBox):
    def __init__(self):
        DialogBox.__init__(
            self, _("Lyrics search"), 460, 300, DIALOG_MASK_MULTIPLE_PAGE, close_callback=self.hide_all, 
            modal=False, window_hint=None, skip_taskbar_hint=False, window_pos=gtk.WIN_POS_CENTER)
        
        self.artist_entry = InputEntry()
        self.artist_entry.set_size(130, 23)
        self.title_entry = InputEntry()
        self.title_entry.set_size(130, 23)
        artist_label = Label(_("Artist:"))
        title_label = Label(_("Title:"))
        right_align = gtk.Alignment()
        right_align.set(0, 0, 0, 1)
        
        self.search_button = Button(_("Search"))
        self.search_button.connect("clicked", self.search_lyric_cb)
        self.process_id = 0
        
        info_box = gtk.HBox(spacing=25)
        
        control_box = gtk.HBox(spacing=5)
        title_box = gtk.HBox(spacing=5)        
        title_box.pack_start(title_label, False, False)
        title_box.pack_start(self.title_entry)
        artist_box = gtk.HBox(spacing=5)
        artist_box.pack_start(artist_label, False, False)
        artist_box.pack_start(self.artist_entry)
        control_box.pack_start(title_box, False, False)
        control_box.pack_start(artist_box, False, False)
        
        info_box.pack_start(control_box, False, False)
        info_box.pack_start(self.search_button, False, False)
        
        sort_items = [ lambda items, reverse : self.sort_by_key(items, reverse, "title"),
                       lambda items, reverse : self.sort_by_key(items, reverse, "artist")]
        self.result_view = TreeView()
        self.result_view.set_expand_column(0)
        self.result_view.connect("double-click-item", self.double_click_cb)
        self.result_view.set_column_titles([_("Title"), _("Artist")], sort_items)
        self.result_view.draw_mask = self.draw_view_mask
        
        self.prompt_label = Label("")
        download_button = Button(_("Download"))
        download_button.connect("clicked", self.download_lyric_cb)
        cancel_button = Button(_("Close"))
        cancel_button.connect("clicked", lambda w: self.hide_all())
        
        info_box_align = gtk.Alignment()
        info_box_align.set_padding(5, 0, 5, 0)
        info_box_align.add(info_box)
        
        self.body_box.set_spacing(5)
        self.body_box.pack_start(info_box_align, False, False)
        self.body_box.pack_start(self.result_view, True, True)
        self.left_button_box.set_buttons([self.prompt_label])
        self.right_button_box.set_buttons([download_button, cancel_button])
        self.lrc_manager = LrcManager()
        
    def show_window(self):    
        DialogBox.show_window(self)
        self.present()
                
    def draw_view_mask(self, cr, x, y, width, height):            
        draw_alpha_mask(cr, x, y, width, height, "layoutMiddle")
        
    def sort_by_key(self, items, sort_reverse, sort_key):    
        return sorted(items, reverse=sort_reverse, key=lambda item: getattr(item, sort_key))
        
    def double_click_cb(self, widget, item, colume, x, y):   
        self.download_lyric_cb(widget)
        
    def search_engine(self, artist, title, pid):    
        
        ting_result = poster.query_lrc_info(artist, title)
        self.render_lyrics(ting_result, pid=pid)
        
        ttplayer_result = TTPlayer().request(artist, title)
        self.render_lyrics(ttplayer_result, pid=pid)
        
        ttpod_result = TTPod().request(artist, title)        
        self.render_lyrics(ttpod_result, pid=pid)
        
        duomi_result = DUOMI().request(artist, title)
        self.render_lyrics(duomi_result, pid=pid, last=True)
        
        
    def search_lyric_cb(self, widget):
        self.result_view.clear()
        artist = self.artist_entry.entry.get_text()
        title = self.title_entry.entry.get_text()
        # widget.set_sensitive(False)
        self.prompt_label.set_text(_("Now searching..."))
        if artist == "" and title == "":
            self.prompt_label.set_text(_("Not found!"))
            return
        self.process_id += 1
        utils.ThreadLoad(self.search_engine, artist, title, self.process_id).start()
        
    @post_gui
    def render_lyrics(self, result, last=False, pid=1):
        '''docs'''
        if pid != self.process_id:
            return
        
        if result != None:
            try:
                items = [SearchItem(each_info) for each_info in result]
            except:    
                pass
            else:
                self.result_view.add_items(items)

            self.prompt_label.set_text(_("%d lyrics found") % len(self.result_view.get_items()))
        else:    
            if last:
                if len(self.result_view.get_items()) > 0:
                    self.prompt_label.set_text(_("%d lyrics found") % len(self.result_view.get_items()))
                else:
                    self.prompt_label.set_text(_("Not found!"))
                    

    def download_lyric_cb(self, widget):
        select_items = self.result_view.select_rows
        save_filepath = self.lrc_manager.get_lrc_filepath(Player.song)
        if len(select_items) > 0:
            self.prompt_label.set_text(_("Downloading lyrics..."))
            item = self.result_view.get_items()[select_items[0]]
            url = item.get_url()
            net_encode = item.get_netcode()
            utils.ThreadRun(utils.download, self.render_download, [url, save_filepath, net_encode]).start()
            
    @post_gui        
    def render_download(self, result):
        if result:
            Dispatcher.reload_lrc(Player.song)
            self.prompt_label.set_text("%s %s" % (_("File save to"), config.get("lyrics", "save_lrc_path")))
        else:    
            self.prompt_label.set_text(_("Download failed."))
        
        
class SearchItem(TreeItem):        

    def __init__(self, lrc_list):
        TreeItem.__init__(self)
        
        self.update(lrc_list)
        
        self.is_highlight = False
        self.column_index = 0
        self.default_height = 26
    
    def emit_redraw_request(self):
        if self.redraw_request_callback:
            self.redraw_request_callback(self)
            
    def update(self, lrc_list):    
        self.__url = lrc_list[2]
        self.title  = utils.xmlescape(lrc_list[1])
        self.artist = utils.xmlescape(lrc_list[0])
        
        if len(lrc_list) > 3:
            self.netcode = lrc_list[3]
        else:    
            self.netcode = "utf-8"
        
        # Calculate item size.
        self.title_padding_x = 10
        self.title_padding_y = 5
        (self.title_width, self.title_height) = get_content_size(self.title, DEFAULT_FONT_SIZE)
        
        self.artist_padding_x = 10
        self.artist_padding_y = 5
        (self.artist_width, self.artist_height) = get_content_size(self.artist, DEFAULT_FONT_SIZE)
        
    def render_title(self, cr, rect):
        '''Render title.'''
        
        if self.is_select:    
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHighlight")
        elif self.is_hover:
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHover")
            
        rect.x += self.title_padding_x
        rect.width -= self.title_padding_x * 2
        render_item_text(cr, self.title, rect, self.is_select, self.is_highlight)
    
    def render_artist(self, cr, rect):
        '''Render artist.'''
        if self.is_select:    
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHighlight")
        elif self.is_hover:
            draw_single_mask(cr, rect.x, rect.y, rect.width, rect.height, "globalItemHover")
        
        rect.x += self.artist_padding_x
        rect.width -= self.artist_padding_x * 2
        render_item_text(cr, self.artist, rect, self.is_select, self.is_highlight)
        
        
    def get_column_widths(self):
        '''Get sizes.'''
        return (260, 160)
    
    def get_height(self):
        return self.default_height
    
    def get_column_renders(self):
        '''Get render callbacks.'''
        return (self.render_title, self.render_artist)
    
    def unselect(self):
        self.is_select = False
        self.emit_redraw_request()
    
    def select(self):    
        self.is_select = True
        self.emit_redraw_request()
        
    def unhover(self, column, offset_x, offset_y):
        self.is_hover = False
        self.emit_redraw_request()
    
    def hover(self, column, offset_x, offset_y):
        self.is_hover = True
        self.emit_redraw_request()
    
    def get_url(self):
        return self.__url
    
    def get_netcode(self):
        return self.netcode
