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
import cairo

from dtk.ui.window import Window
from dtk.ui.label import Label
from dtk.ui.button import CheckButton, RadioButton
from dtk.ui.spin import SpinBox
from dtk.ui.line import draw_vlinear
from dtk.ui.utils import get_content_size, move_window
from dtk.ui.scalebar import HScalebar
from dtk.ui.entry import TextEntry
from dtk.ui.treeview import TreeView
from dtk.ui.button import Button
from dtk.ui.combo import ComboBox, ComboBoxItem
from dtk.ui.scrolled_window import ScrolledWindow

from utils import color_hex_to_cairo
from widget.skin import app_theme
from widget.ui_utils import get_font_families, switch_tab, create_separator_box, create_right_align
from render_lyrics import RenderContextNew
from config import config


class GeneralSetting(gtk.VBox):
    
    def __init__(self):
        super(GeneralSetting, self).__init__()
        self.set_spacing(30)
        self.pack_start(self.create_start_box(), False, True)
        self.pack_start(self.create_close_box(), False, True)
        self.pack_start(self.create_play_box(), False, True)
        self.pack_start(self.create_link_box(), False, True)
        
        # Load.
        self.load_status()
        
    def load_status(self):    
        self.open_lyrics_check_button.set_active(config.getboolean("lyrics", "status"))
        self.auto_play_check_button.set_active(config.getboolean("player", "play_on_startup"))
        
        close_to_tray = config.getboolean("setting", "close_to_tray")
        if close_to_tray:
            self.tray_radio_button.set_active(True)
        else:    
            self.tray_radio_button.set_active(True)
        
        self.fade_check_button.set_active(config.getboolean("player", "crossfade"))
        self.album_check_button.set_active(config.getboolean("player", "crossfade_gapless_album"))
        
        
        new_value = int(float(config.get("player", "crossfade_time")) * 100)
        self.fade_spin.set_value(new_value)
        
        
    def create_start_box(self):    
        main_table = gtk.Table(3, 2)
        main_table.set_row_spacings(10)
        start_title_label = Label("启动时")
        start_title_label.set_size_request(350, 12)
        label_align = gtk.Alignment()
        label_align.set_padding(20, 0, 0, 0)
        label_align.add(start_title_label)
        
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
        
        main_table.attach(label_align, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, gtk.FILL)
        main_table.attach(auto_play_hbox, 0, 1, 2, 3)
        main_table.attach(open_lyrics_hbox, 1, 2, 2, 3, yoptions=gtk.EXPAND)
        return main_table
    
    def create_close_box(self):
        main_table = gtk.Table(3, 2)
        main_table.set_row_spacings(10)
        close_title_label = Label("关闭时")
        close_title_label.set_size_request(350, 12)
        
        # mini_check_button

        self.tray_radio_button = RadioButton("最小化至托盘")        
        self.quit_radio_button = RadioButton("退出程序")
        
        main_table.attach(close_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(self.tray_radio_button, 0, 1, 2, 3, yoptions=gtk.FILL)
        main_table.attach(self.quit_radio_button, 1, 2, 2, 3)
        
        return main_table
        
    
    def create_play_box(self):
        main_table = gtk.Table(4, 2)
        main_table.set_row_spacings(10)
        
        play_title_label = Label("播放时")
        play_title_label.set_size_request(350, 12)
        
        fade_check_hbox = gtk.HBox()
        self.fade_check_button = CheckButton("启用淡入淡出功能")        
        fade_check_hbox.pack_start(self.fade_check_button, False, False)
        fade_check_hbox.pack_start(create_right_align(), True, True)
        self.album_check_button = CheckButton("相同专辑间无缝切换")
        
        fade_label = Label("淡入淡出:")
        fade_label.set_size_request(60, 12)
        self.fade_spin = SpinBox(300, 1, 1000, 100)
        millisecond_lablel = Label(" 毫秒")        
        millisecond_lablel.set_size_request(50, 12)
        spin_hbox = gtk.HBox()
        spin_hbox.pack_start(fade_label, False, False)
        spin_hbox.pack_start(self.fade_spin, False, False)
        spin_hbox.pack_start(millisecond_lablel, False, False)
        
        main_table.attach(play_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(fade_check_hbox, 0, 1, 2, 3, yoptions=gtk.FILL)
        main_table.attach(self.album_check_button, 1, 2, 2, 3, yoptions=gtk.FILL)
        main_table.attach(spin_hbox, 0, 1, 3, 4, yoptions=gtk.FILL, xpadding=8)
        
        return main_table

    def create_link_box(self):    
        
        main_table = gtk.Table(3, 2, True)
        main_table.set_row_spacings(3)
        
        link_title_label = Label("关联")
        link_title_label.set_size_request(350, 12)
        
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
        self.main_table.set_row_spacings(10)
        self.pack_start(self.main_table, False, True)
        self.create_hotkey_box()
        
    def create_hotkey_box(self):    
        hotkey_title_label = Label("热键设置")
        hotkey_title_label.set_size_request(350, 12)
        label_align = gtk.Alignment()
        label_align.set_padding(20, 0, 0, 0)
        label_align.add(hotkey_title_label)
        self.main_table.attach(label_align, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
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
        self.main_table.attach(hotkey_entry, 1, 2, top_attach, bottom_attach, xoptions=gtk.FILL)
        
class DesktopLyricsSetting(gtk.VBox):
    
    def __init__(self):
        super(DesktopLyricsSetting, self).__init__()
        self.set_spacing(20)
        self.pack_start(self.create_lyrics_dir_table(), False, True)
        self.pack_start(self.create_style_table(), False, True)
        self.render_lyrics = RenderContextNew()
        
        self.preview = gtk.EventBox()
        self.preview.set_visible_window(False)
        self.preview.set_size_request(-1, 135)
        preview_align = gtk.Alignment()
        preview_align.set(0.0, 0.0, 1.0, 1.0)
        preview_align.set_padding(0, 10, 5, 5)
        preview_align.add(self.preview)
        
        self.preview.connect("expose-event", self.draw_lyrics)
        self.pack_start(preview_align, False, True)
        self.load_status()
        
    def get_render_color(self, active=False):        
        if active:
            return [color_hex_to_cairo(config.get("lyrics", "active_color_upper")),
                    color_hex_to_cairo(config.get("lyrics", "active_color_middle")),
                    color_hex_to_cairo(config.get("lyrics", "active_color_bottom"))]
        else:
            return [color_hex_to_cairo(config.get("lyrics", "inactive_color_upper")),
                    color_hex_to_cairo(config.get("lyrics", "inactive_color_middle")),
                    color_hex_to_cairo(config.get("lyrics", "inactive_color_bottom"))]
        
    def draw_lyric_surface(self, lyrics, active=False):
        if not lyrics:
            return None
        self.render_lyrics.set_linear_color(self.get_render_color(active))
        width, height = self.render_lyrics.get_pixel_size(lyrics)
        surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, int(width), int(height))
        cr = cairo.Context(surface)
        cr.set_source_rgba(1.0, 1.0, 1.0, 0.0)
        cr.set_operator(cairo.OPERATOR_SOURCE)
        cr.paint()
        self.render_lyrics.paint_text(cr, lyrics, 0, 0)
        return surface
    
    def draw_lyrics(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        font_height = self.render_lyrics.get_font_height()
        xpos = rect.x + 5
        ypos = rect.y + 5
        cr.save()
        cr.rectangle(xpos, ypos, rect.width, rect.height)
        cr.clip()
        cr.set_operator(cairo.OPERATOR_OVER)
        
        # draw_active
        cr.save()
        cr.rectangle(xpos, ypos, rect.width * 0.5, rect.height)
        cr.clip()
        active_surface = self.draw_lyric_surface("深度音乐播放器", True)
        if active_surface:
            cr.set_source_surface(active_surface, xpos, ypos)
            cr.paint()
        cr.restore()    
        
        # draw_inactive
        cr.save()
        cr.rectangle(xpos + rect.width * 0.5, ypos, rect.width*0.5, rect.height)
        cr.clip()
        inactive_surface = self.draw_lyric_surface("深度音乐播放器")
        if inactive_surface:
            cr.set_source_surface(inactive_surface, xpos, ypos)
            cr.paint()
        cr.restore()    
        
        cr.restore()

        return False
        
    def load_status(self):    
        self.dir_entry.set_text(config.get("lyrics", "save_lrc_path"))
        self.dir_entry.set_size_request(300, 12)
        
    def create_lyrics_dir_table(self):    
        main_table = gtk.Table(3, 2)
        main_table.set_row_spacings(5)
        
        dir_title_label = Label("歌词保存目录")
        dir_title_label.set_size_request(300, 12)
        label_align = gtk.Alignment()
        label_align.set_padding(20, 0, 0, 0)
        label_align.add(dir_title_label)
        
        self.dir_entry = TextEntry("~/.lyrics")
        self.dir_entry.set_size(290, 23)
        
        modify_button = Button("修改目录")
        hbox = gtk.HBox(spacing=5)
        hbox.pack_start(self.dir_entry, False, False)
        hbox.pack_start(modify_button, False, False)
        
        main_table.attach(label_align, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(hbox, 0, 2, 2, 3, xpadding=18, xoptions=gtk.FILL)
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
        
        outline_fuzzy_box = gtk.HBox(spacing=80)
        outline_fuzzy_box.pack_start(outline_hbox, False, False)
        outline_fuzzy_box.pack_start(fuzzy_hbox, False, False)
        
        main_table.attach(style_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(font_name_hbox, 0, 1, 2, 3, xpadding=20, xoptions=gtk.FILL)
        main_table.attach(font_size_hbox, 1, 2, 2, 3, xoptions=gtk.FILL)
        main_table.attach(line_number_hbox, 0, 1, 3, 4, xpadding=20, xoptions=gtk.FILL)
        main_table.attach(align_hbox, 1, 2, 3, 4, xoptions=gtk.FILL)
        main_table.attach(outline_fuzzy_box, 0, 2, 4, 5, xpadding=20, xoptions=gtk.FILL)
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
        self.set_size_request(575, 495)
        titlebar = gtk.EventBox()
        titlebar.set_visible_window(False)
        titlebar.set_size_request(-1, 32)
        titlebar.connect("button-press-event", lambda w, e: move_window(w, e, self))
        statusbar = gtk.EventBox()
        statusbar.set_visible_window(False)
        statusbar.set_size_request(-1, 50)
        
        button_align = gtk.Alignment()
        button_align.set(1.0, 0.5, 0, 0)
        button_align.set_padding(10, 10, 5, 10)
        button_box = gtk.HBox()
        
        button_align.add(button_box)
        save_button = Button("保存")
        save_button.connect("clicked", self.click_save_button)
        cancel_button = Button("取消")
        cancel_button.connect("clicked", self.click_cancel_button)
        
        button_box.pack_start(save_button, False, True, 5)
        button_box.pack_start(cancel_button, False, True, 5)
        
        statusbar.add(button_align)
        
        main_align = gtk.Alignment()
        main_align.set(0.0, 0.0, 1.0, 1.0)
        main_align.set_padding(0, 0, 2, 2)
        self.main_box = gtk.VBox(spacing=5)
        main_align.add(self.main_box)
        self.window_frame.pack_start(titlebar, False, True)
        self.window_frame.pack_start(main_align, True, True)
        self.window_frame.pack_start(statusbar, False, True)
        self.main_box.connect("expose-event", self.expose_mask_cb)
        
        # Category bar
        self.category_bar = TreeView(font_x_padding=20)
        self.general_category_item = CategoryItem("常规设置", GeneralSetting())
        self.category_bar.add_item(None, self.general_category_item)
        self.category_bar.add_item(None, CategoryItem("热键设置", HotKeySetting()))
        lyrics_node = self.category_bar.add_item(None, CategoryItem("歌词设置"))
        self.category_bar.add_item(lyrics_node, CategoryItem("桌面歌词", DesktopLyricsSetting()))
        self.category_bar.add_item(lyrics_node, CategoryItem("窗口歌词"))
        self.category_bar.add_item(None, CategoryItem("关于我们"))
        self.category_bar.connect("single-click-item", self.category_single_click_cb)
        
        category_align = gtk.Alignment()
        category_align.set_padding(11, 0, 0, 0)

        category_scrolled_window = ScrolledWindow()
        category_scrolled_window.add_child(self.category_bar)
        category_scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_NEVER)
        category_scrolled_window.set_size_request(132, 516)
        category_align.add(category_scrolled_window)
        
        # Pack widget.
        left_box = gtk.VBox()
        self.right_box = gtk.VBox()
        left_box.add(category_align)
        self.right_box.add(self.general_category_item.get_allocated_widget())
        right_align = gtk.Alignment()
        right_align.set_padding(0, 0, 10, 0)
        right_align.add(self.right_box)

        body_box = gtk.HBox()
        body_box.pack_start(left_box, False, True)
        body_box.pack_start(right_align, False, True)
        self.main_box.add(body_box)
        
    def expose_mask_cb(self, widget, event):    
        cr = widget.window.cairo_create()
        rect = widget.allocation
        draw_vlinear(cr, rect.x, rect.y, 132, rect.height, app_theme.get_shadow_color("settingMaskLeft").get_color_info())
        draw_vlinear(cr, rect.x + 132, rect.y, rect.width - 132, rect.height, app_theme.get_shadow_color("settingMaskRight").get_color_info())
        return False
    
    def category_single_click_cb(self, widget, item):
        if item.get_allocated_widget():
            switch_tab(self.right_box, item.get_allocated_widget())
            
    def click_save_button(self, widget):        
        self.destroy()
        
    def click_cancel_button(self, widget):    
        self.destroy()
    
class CategoryItem(object):    
    
    def __init__(self, item_title, allocated_widget=None, has_arrow=True, item_left_image=None):
        self.item_title = item_title
        self.allocated_widget = allocated_widget
        self.has_arrow= has_arrow
        self.item_left_image = item_left_image
        self.item_id = None
        
    def get_title(self):    
        return self.item_title
    
    def get_has_arrow(self):
        return self.has_arrow
    
    def set_item_id(self, new_id):
        self.item_id = new_id
        
    def get_item_id(self):    
        return self.item_id
    
    def get_allocated_widget(self):
        return self.allocated_widget
