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
from collections import OrderedDict
from dtk.ui.new_treeview import TreeView
from dtk.ui.paned import HPaned

from widget.webcast_item import CategroyTreeItem
from widget.skin import app_theme
from widget.combo import TextPrompt
from widget.webcast_view import WebcastIconView, MultiDragWebcastView
from widget.ui_utils import draw_alpha_mask, switch_tab, set_widget_gravity
from webcast_library import WebcastDB, WebcastQuery
from widget.ui import BackButton
from helper import SignalContainer, Dispatcher
from nls import _


class WebcastsBrowser(gtk.VBox, SignalContainer):
    
    def __init__(self):
        gtk.VBox.__init__(self)
        SignalContainer.__init__(self)

        # load data.
        self.__load_webcast_query()
        
        # Init webcastbar.
        self.__init_webcastbar()
        
        # Init iconview.
        self.region_view = self.get_icon_view("region_en")
        self.region_view_sw = self.region_view.get_scrolled_window()
        self.genre_view = self.get_icon_view("region")
        self.genre_view_sw = self.genre_view.get_scrolled_window()
        
        self.page_box = gtk.VBox()
        self.page_box.add(self.region_view_sw)
        
        # webcasts view
        self.webcast_view = self.get_webcast_view()
        self.webcast_view_sw = self.webcast_view.get_scrolled_window()
        
        # init listview page.
        self.init_listview_page()
        
        body_paned = HPaned(handle_color=app_theme.get_color("panedHandler"))
        body_paned.add1(self.webcastbar)
        body_paned.add2(self.page_box)
        self.add(body_paned)
        
    def __load_webcast_query(self):    
        self.__db_query = WebcastQuery()        
        if WebcastDB.isloaded():
            self.__on_db_loaded()
        else:    
            WebcastDB.connect("loaded", lambda obj: self.__on_db_loaded())
            
    def __on_db_loaded(self):        
        self.autoconnect(self.__db_query, "added", self.__on_added_songs)
        self.autoconnect(self.__db_query, "removed", self.__on_removed_songs)
        self.autoconnect(self.__db_query, "update-songs", self.__on_update_songs)
        self.autoconnect(self.__db_query, "full-update", self.__on_full_update)
        self.__db_query.set_query()
        
    def __on_added_songs(self, db_query, songs):    
        self.reload_flag = True
    
    def __on_removed_songs(self, db_query, songs):
        self.reload_flag = True
    
    def __on_update_songs(self, db_query, songs):
        self.reload_flag = True
    
    def __on_full_update(self, db_query):
        self.load_view_data()
        
    def __init_webcastbar(self):    
        self.source_data = OrderedDict()
        self.source_data["region"] = _("国内广播")
        self.source_data["genre"] = _("流派广播")        
        # self.source_data["foreign"]  = _("国外广播")
        # self.source_data["network"] = _("网络广播")
        # self.source_data["music"]  = _("音乐广播")
        # self.source_data["finance"] = _("财经广播")
        # self.source_data["sports"] = _("体育广播")
        self.webcastbar = TreeView(enable_drag_drop=False, enable_multiple_select=False)
        items = []
        items.append(CategroyTreeItem(self.source_data["region"], lambda : switch_tab(self.page_box, self.region_view_sw)))
        items.append(CategroyTreeItem(self.source_data["genre"], lambda : switch_tab(self.page_box, self.genre_view_sw)))
        self.webcastbar.add_items(items)
        self.webcastbar.select_items([self.webcastbar.visible_items[0]])
        self.webcastbar.set_size_request(121, -1)
        self.webcastbar.draw_mask = self.on_webcastbar_draw_mask        
        
    def on_webcastbar_draw_mask(self, cr, x, y, w, h):    
        draw_alpha_mask(cr, x, y, w, h ,"layoutRight")
        
    def init_listview_page(self):    
        self.listview_page = gtk.VBox()
        self.text_prompt = TextPrompt("Default")
        prompt_align = set_widget_gravity(self.text_prompt, 
                                          paddings=(10, 10, 0, 0))
        prompt_box = gtk.HBox()
        back_button = BackButton()
        back_button_align = set_widget_gravity(back_button, gravity=(0.5, 0.5, 0, 0),
                                               paddings=(0, 0, 10, 5))
        prompt_box.pack_start(back_button_align, False, True)
        prompt_box.pack_start(prompt_align, False, False)
        
        self.listview_page.pack_start(prompt_box, False, True)
        self.listview_page.pack_start(self.webcast_view_sw, True, True)
        
    def switch_to_listview(self, category, title):    
        self.text_prompt.set_text(title)
        self.webcast_view.clear()
        
        songs = self.__db_query.get_songs(category, title)
        self.webcast_view.add_webcasts(songs)
        switch_tab(self.page_box, self.listview_page)
        
    def load_view_data(self):    
        region_child_datas = self.__db_query.get_info("region_en")[0]
        genre_child_datas = self.__db_query.get_info("region")[0]
        
        self.region_view.add_webcast_items(region_child_datas)
        self.genre_view.add_webcast_items(genre_child_datas)
        
    def get_icon_view(self, category):
        icon_view = WebcastIconView(category)
        icon_view.connect("single-click-item", self.on_iconview_single_click_item)
        return icon_view
    
    
    def on_iconview_single_click_item(self, widget, item, x, y):
        category = widget.category
        title = item.title
        self.switch_to_listview(category, title)
    
        
    def get_webcast_view(self):    
        return MultiDragWebcastView()
        
