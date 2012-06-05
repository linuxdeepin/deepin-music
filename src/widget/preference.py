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

from dtk.ui.window import Window
from dtk.ui.label import Label
from dtk.ui.line import HSeparator
from dtk.ui.button import CheckButton, RadioButton
from dtk.ui.spin import SpinBox
from dtk.ui.line import draw_vlinear
from dtk.ui.utils import get_content_size
from dtk.ui.scalebar import HScalebar
from dtk.ui.entry import TextEntry
from dtk.ui.button import Button
from dtk.ui.combo import ComboBox, ComboBoxItem

from widget.skin import app_theme
from widget.ui_utils import get_font_families
from config import config

def create_separator_box(padding_x=0, padding_y=0):    
    separator_box = HSeparator(
        app_theme.get_shadow_color("hSeparator").get_color_info(),
        padding_x, padding_y)
    return separator_box

def create_right_align():    
    align = gtk.Alignment()
    align.set(0, 0, 0, 1)
    return align


class GeneralSetting(gtk.VBox):
    
    def __init__(self):
        super(GeneralSetting, self).__init__()
        self.set_spacing(20)
        self.pack_start(self.create_start_box(), False, True)
        self.pack_start(self.create_close_box(), False, True)
        self.pack_start(self.create_play_box(), False, True)
        self.pack_start(self.create_link_box(), False, True)
        
        # Load.
        self.load_widget_status()
        
    def load_widget_status(self):    
        self.open_lyrics_check_button.set_active(config.getboolean("lyrics", "status"))
        self.auto_play_check_button.set_active(config.getboolean("player", "play_on_startup"))
        
    def create_start_box(self):    
        main_table = gtk.Table(3, 2, True)
        main_table.set_row_spacings(3)
        start_title_label = Label("启动时")
        start_title_label.set_size_request(400, 12)
        
        # startup_check_button.
        self.auto_play_check_button = CheckButton("启动时自动播放")
        auto_play_hbox = gtk.HBox()
        auto_play_hbox.pack_start(self.auto_play_check_button, False, False)
        auto_play_hbox.pack_start(create_right_align(), True, True)
        
        # open_lyrics_check_button.
        open_lyrics_hbox = gtk.HBox()
        self.open_lyrics_check_button = CheckButton("打开桌面歌词")
        open_lyrics_hbox.pack_start(self.open_lyrics_check_button, False, False)
        open_lyrics_hbox.pack_start(create_right_align(), True, True)
        
        
        main_table.attach(start_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, gtk.FILL)
        main_table.attach(auto_play_hbox, 0, 1, 2, 3)
        main_table.attach(open_lyrics_hbox, 1, 2, 2, 3, yoptions=gtk.EXPAND)
        return main_table
        
    
    def create_close_box(self):
        main_table = gtk.Table(3, 2, True)
        main_table.set_row_spacings(3)
        close_title_label = Label("关闭时")
        close_title_label.set_size_request(400, 12)
        
        # mini_check_button

        mini_radio_button = RadioButton("最小化至托盘")        
        quit_radio_button = RadioButton("退出程序")
        
        main_table.attach(close_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(mini_radio_button, 0, 1, 2, 3, yoptions=gtk.FILL)
        main_table.attach(quit_radio_button, 1, 2, 2, 3)
        
        return main_table
        
    
    def create_play_box(self):
        main_table = gtk.Table(4, 2, True)
        main_table.set_row_spacings(3)
        
        play_title_label = Label("播放时")
        play_title_label.set_size_request(400, 12)
        
        fade_check_hbox = gtk.HBox()
        fade_check_button = CheckButton("启用淡入淡出功能")        
        fade_check_hbox.pack_start(fade_check_button, False, False)
        fade_check_hbox.pack_start(create_right_align(), True, True)
        album_check_button = CheckButton("相同专辑间无缝切换")
        
        fade_label = Label("淡入淡出:")
        fade_label.set_size_request(60, 12)
        fade_spin = SpinBox(300, 1, 1000, 100)
        millisecond_lablel = Label(" 毫秒")        
        millisecond_lablel.set_size_request(50, 12)
        spin_hbox = gtk.HBox()
        spin_hbox.pack_start(fade_label, False, False)
        spin_hbox.pack_start(fade_spin, False, False)
        spin_hbox.pack_start(millisecond_lablel, False, False)
        
        main_table.attach(play_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(fade_check_hbox, 0, 1, 2, 3, yoptions=gtk.FILL)
        main_table.attach(album_check_button, 1, 2, 2, 3, yoptions=gtk.FILL)
        main_table.attach(spin_hbox, 0, 1, 3, 4, yoptions=gtk.FILL, xpadding=8)
        
        return main_table

    def create_link_box(self):    
        
        main_table = gtk.Table(3, 2, True)
        main_table.set_row_spacings(3)
        
        link_title_label = Label("关联")
        link_title_label.set_size_request(400, 12)
        
        link_check_box = gtk.HBox()
        link_check_button = CheckButton("将深度音乐设置为默认播放器")
        link_check_box.pack_start(link_check_button, False, False)
        link_check_box.pack_start(create_right_align(), True, True)
        
        main_table.attach(link_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(link_check_box, 0, 2, 2, 3, yoptions=gtk.FILL)
        
        return main_table
    
    
class HotKeySetting(gtk.VBox):        
    
    def __init__(self):
        super(HotKeySetting, self).__init__()
        self.main_table = gtk.Table(12, 2)        
        self.main_table.set_row_spacings(5)
        self.pack_start(self.main_table, False, True)
        self.create_hotkey_box()
        
    def create_hotkey_box(self):    
        hotkey_title_label = Label("热键设置")
        hotkey_title_label.set_size_request(400, 12)
        self.main_table.attach(hotkey_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        self.main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        
        # using check button.
        using_hbox = gtk.HBox()
        using_check_button = CheckButton("启用快捷键")
        using_hbox.pack_start(using_check_button, False, False)
        using_hbox.pack_start(create_right_align(), False, True)
        self.main_table.attach(using_hbox, 0, 2, 2, 3, yoptions=gtk.FILL)
        
        self.create_combo_entry(3, 4, "最小化/正常模式", "Ctrl + Alt + Q")
        self.create_combo_entry(4, 5, "显示/隐藏桌面歌词", "Ctrl + Alt + D")
        self.create_combo_entry(5, 6, "锁定/解锁桌面歌词", "Ctrl + Alt + E")
        self.create_combo_entry(6, 7, "播放/暂停", "Ctrl + Alt + F5")
        self.create_combo_entry(7, 8, "上一首", "Ctrl + Alt + Left")
        self.create_combo_entry(8, 9, "下一首", "Ctrl + Alt + Right")
        self.create_combo_entry(9, 10, "增大音量", "Ctrl + Alt + Up")
        self.create_combo_entry(10, 11, "减小音量", "Ctrl + Alt + Down")
        
        # Button.
        default_button = Button("恢复默认")
        button_hbox = gtk.HBox()
        button_hbox.pack_start(create_right_align(), True, True)
        button_hbox.pack_start(default_button, False, False)
        
        self.main_table.attach(button_hbox, 0, 2, 11, 12)
        
        
    def create_combo_entry(self, top_attach, bottom_attach, label_content, hotkey_content):    
        combo_hbox = gtk.HBox()
        combo_hbox.pack_start(create_right_align(), True, True)
        
        # single_hotkey_label
        width, _ = get_content_size(label_content, 10)
        hotkey_label = Label(label_content)
        hotkey_label.set_size_request(width, 20)
        combo_hbox.pack_start(hotkey_label, False, False)
        
        # Hotkey entry.
        hotkey_entry = TextEntry(hotkey_content)
        hotkey_entry.set_size(170, 23)
        
        self.main_table.attach(combo_hbox, 0, 1, top_attach, bottom_attach, xpadding=5)
        self.main_table.attach(hotkey_entry, 1, 2, top_attach, bottom_attach)
        
class DesktopLyricsSetting(gtk.VBox):
    
    def __init__(self):
        super(DesktopLyricsSetting, self).__init__()
        self.set_spacing(20)
        self.pack_start(self.create_lyrics_dir_table(), False, True)
        self.pack_start(self.create_style_table(), False, True)
        
    def create_lyrics_dir_table(self):    
        main_table = gtk.Table(3, 2)
        main_table.set_row_spacings(5)
        
        dir_title_label = Label("歌词保存目录")
        dir_title_label.set_size_request(40, 12)
        dir_entry = TextEntry("~/.lyrics")
        dir_entry.set_size(300, 23)
        
        modify_button = Button("修改目录")
        
        main_table.attach(dir_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(dir_entry, 0, 1, 2, 3, xpadding=8)
        main_table.attach(modify_button, 1, 2, 2, 3)
        
        return main_table
    
    def create_style_table(self):
        main_table = gtk.Table(5, 2)
        main_table.set_row_spacings(10)
        
        style_title_label = Label("歌词样式")
        
        font_name_hbox = self.create_combo_widget("字体:",
                                                  [ComboBoxItem(font_name) for font_name in get_font_families()[:10]]
                                                  )
        font_size_hbox = self.create_combo_widget("字号:", 
                                                  [ComboBoxItem(str(num)) for num in range(20, 40, 2)],
                                                  default_width=50)
        
        line_number_hbox = self.create_combo_widget("行数:",
                                                    [ComboBoxItem(name) for name in ["单行", "双行"]])
        
        align_hbox = self.create_combo_widget("对齐:",
                                              [ComboBoxItem(name) for name in ["左对齐", "居中对齐", "右对齐"]],
                                              1)
        
        outline_hbox = self.create_combo_widget("轮廓:",
                                                [ComboBoxItem(str(num)) for num in range(1, 10)],
                                                default_width=50)
        
        fuzzy_slipper = HScalebar(
            app_theme.get_pixbuf("slipper/left_fg.png"),
            app_theme.get_pixbuf("slipper/left_bg.png"),
            app_theme.get_pixbuf("slipper/middle_fg.png"),
            app_theme.get_pixbuf("slipper/middle_bg.png"),
            app_theme.get_pixbuf("slipper/right_fg.png"),
            app_theme.get_pixbuf("slipper/right_bg.png"),
            app_theme.get_pixbuf("slipper/point.png"),
            )
        
        fuzzy_slipper.set_size_request(177, -1)
        fuzzy_label = Label("模糊:")
        fuzzy_label.set_size_request(35, -1)
        fuzzy_hbox = gtk.HBox()
        slipper_align = gtk.Alignment()
        slipper_align.set(0.5, 0.5, 0.0, 0.0)
        slipper_align.set_padding(6, 0, 0, 0)
        slipper_align.add(fuzzy_slipper)
        fuzzy_hbox.pack_start(fuzzy_label, False, False)
        fuzzy_hbox.pack_start(slipper_align, False, False)
        
        main_table.attach(style_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(font_name_hbox, 0, 1, 2, 3, xpadding=20)
        main_table.attach(font_size_hbox, 1, 2, 2, 3,)
        main_table.attach(line_number_hbox, 0, 1, 3, 4, xpadding=20)
        main_table.attach(align_hbox, 1, 2, 3, 4)
        main_table.attach(outline_hbox, 0, 1, 4, 5, xpadding=20)
        main_table.attach(fuzzy_hbox, 1, 2, 4, 5)
        
        return main_table
    
    def create_combo_widget(self, label_content, items, index=0, default_width=100):
        label = Label(label_content)
        label.set_size_request(30, 12)
        
        combo_box = ComboBox(items, default_width)
        combo_box.set_select_index(index)
        
        hbox = gtk.HBox(spacing=5)
        hbox.pack_start(label, False, False)
        hbox.pack_start(combo_box, False, True)
        
        return hbox
        

class PreferenceDialog(Window):
    
    def __init__(self):
        super(PreferenceDialog, self).__init__()
        
        self.set_position(gtk.WIN_POS_CENTER)
        self.set_size_request(540, 500)
        titlebar = gtk.EventBox()
        titlebar.set_visible_window(False)
        titlebar.set_size_request(-1, 32)
        
        statusbar = gtk.EventBox()
        statusbar.set_visible_window(False)
        statusbar.set_size_request(-1, 32)
        
        main_align = gtk.Alignment()
        main_align.set(0.0, 0.0, 1.0, 1.0)
        main_align.set_padding(5, 10, 10, 10)
        self.main_box = gtk.VBox(spacing=5)
        main_align.add(self.main_box)
        self.window_frame.pack_start(titlebar, False, True)
        self.window_frame.pack_start(main_align, True, True)
        self.window_frame.pack_start(statusbar, False, True)
        self.main_box.connect("expose-event", self.expose_mask_cb)
        
        
    def expose_mask_cb(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_vlinear(cr, rect.x, rect.y, rect.width, rect.height, app_theme.get_shadow_color("linearBackground").get_color_info(), 4)
        return False

    def create_general_setting(self):    
        pass
