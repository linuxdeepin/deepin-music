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
import cairo
from collections import OrderedDict

from dtk.ui.label import Label
from dtk.ui.box import BackgroundBox, ImageBox
from dtk.ui.button import CheckButton, RadioButton
from dtk.ui.spin import SpinBox
from dtk.ui.entry import InputEntry, ShortcutKeyEntry
from dtk.ui.treeview import TreeView
from dtk.ui.button import Button
from dtk.ui.dialog import DialogBox, DIALOG_MASK_MULTIPLE_PAGE
from dtk.ui.color_selection import ColorButton
from dtk.ui.combo import ComboBox
from dtk.ui.scrolled_window import ScrolledWindow

from utils import color_hex_to_cairo
from widget.ui_utils import (get_font_families, switch_tab,
                             create_separator_box, create_right_align,
                             draw_alpha_mask)
from widget.dialog import WinDir
from widget.global_keys import global_hotkeys
from widget.skin import app_theme
from render_lyrics import RenderContextNew
from constant import PREDEFINE_COLORS
from config import config
from nls import _

class GeneralSetting(gtk.VBox):
    
    def __init__(self):
        super(GeneralSetting, self).__init__()
        self.set_spacing(30)
        self.pack_start(self.create_start_box(), False, True)
        self.pack_start(self.create_close_box(), False, True)
        self.pack_start(self.create_play_box(), False, True)
        self.pack_start(self.create_lyrics_dir_table(), False, False)
        
        # Load.
        self.load_status()
        
        # Signals.
        self.splash_check_button.connect("toggled", self.update_splash_status)
        self.open_lyrics_check_button.connect("toggled", self.update_lyrics_status)
        self.auto_play_check_button.connect("toggled", self.update_auto_play_status)
        self.resume_last_check_button.connect("toggled", self.update_resume_status)
        self.tray_radio_button.connect("toggled", self.update_close_to_tray_status)
        self.fade_check_button.connect("toggled", self.update_fade_status)
        self.album_check_button.connect("toggled", self.update_album_status)
        self.fade_spin.connect("value-changed", self.update_fade_time_status)
        
    def load_status(self):    
        self.splash_check_button.set_active(config.getboolean("setting", "use_splash"))
        self.open_lyrics_check_button.set_active(config.getboolean("lyrics", "status"))
        self.auto_play_check_button.set_active(config.getboolean("player", "play_on_startup"))
        self.resume_last_check_button.set_active(config.getboolean("player", "resume_last_progress"))
        
        close_to_tray = config.getboolean("setting", "close_to_tray")
        if close_to_tray:
            self.tray_radio_button.set_active(True)
        else:    
            self.quit_radio_button.set_active(True)
        
        self.fade_check_button.set_active(config.getboolean("player", "crossfade"))
        self.album_check_button.set_active(config.getboolean("player", "crossfade_gapless_album"))
        
        new_value = int(float(config.get("player", "crossfade_time")) * 100)
        self.fade_spin.set_value(new_value)
        
    def update_splash_status(self, widget):    
        if widget.get_active():
            config.set("setting", "use_splash", "true")
        else:    
            config.set("setting", "use_splash", "false")
            
    def update_lyrics_status(self, widget):        
        if widget.get_active():    
            config.set("lyrics", "status", "true")
        else:    
            config.set("lyrics", "status", "false")
            
    def update_auto_play_status(self, widget):       
        if widget.get_active():    
            config.set("player", "play_on_startup", "true")            
        else:    
            config.set("player", "play_on_startup", "false")
            
    def update_resume_status(self, widget):        
        if widget.get_active():
            config.set("player", "resume_last_progress", "true")
        else:    
            config.set("player", "resume_last_progress", "false")
            
    def update_close_to_tray_status(self, widget):        
        if widget.get_active():   
            config.set("setting", "close_to_tray", "true")
        else:    
            config.set("setting", "close_to_tray", "false")
            
    def update_fade_status(self, widget):        
        if widget.get_active():
            config.set("player", "crossfade", "true")
            self.fade_spin.set_sensitive(True)
        else:    
            config.set("player", "crossfade", "false")
            self.fade_spin.set_sensitive(False)
            
    def update_album_status(self, widget):        
        if self.album_check_button.get_active():    
            config.set("player", "crossfade_gapless_album", "true")
        else:    
            config.set("player", "crossfade_gapless_album", "false")
            
    def update_fade_time_status(self, widget, value):        
        new_value = value / 100.0
        config.set("player", "crossfade_time", str(new_value))        
            
    def create_start_box(self):    
        main_table = gtk.Table(4, 2)
        main_table.set_row_spacings(10)
        start_title_label = Label(_("Starting"))
        start_title_label.set_size_request(350, 12)
        label_align = gtk.Alignment()
        label_align.set_padding(20, 0, 0, 0)
        label_align.add(start_title_label)
        
        # splash check_button
        self.splash_check_button = CheckButton(_("Display splash"))
        splash_hbox = gtk.HBox()
        splash_hbox.pack_start(self.splash_check_button, False, False)
        splash_hbox.pack_start(create_right_align(), True, True)        
        
        # open_lyrics_check_button.
        open_lyrics_hbox = gtk.HBox()
        self.open_lyrics_check_button = CheckButton(_("Lyrics on"))
        open_lyrics_hbox.pack_start(self.open_lyrics_check_button, False, False)

        
        # startup_check_button.
        self.auto_play_check_button = CheckButton(_("Auto play"))
        auto_play_hbox = gtk.HBox()
        auto_play_hbox.pack_start(self.auto_play_check_button, False, False)
        auto_play_hbox.pack_start(create_right_align(), True, True)                
        
        # resume last check_button.
        self.resume_last_check_button = CheckButton(_("Resume last played"))
        resume_hbox = gtk.HBox()
        resume_hbox.pack_start(self.resume_last_check_button, False, False)
        
        main_table.attach(label_align, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, gtk.FILL)
        main_table.attach(splash_hbox, 0, 1, 2, 3)
        main_table.attach(open_lyrics_hbox, 1, 2, 2, 3, yoptions=gtk.EXPAND)
        main_table.attach(auto_play_hbox, 0, 1, 3, 4)
        main_table.attach(resume_hbox, 1, 2, 3, 4, yoptions=gtk.EXPAND)
        return main_table
    
    def create_close_box(self):
        main_table = gtk.Table(3, 2)
        main_table.set_row_spacings(10)
        close_title_label = Label(_("Closing"))
        close_title_label.set_size_request(350, 12)
        
        # mini_check_button

        self.tray_radio_button = RadioButton(_("Minimize to trayicon"))        
        self.quit_radio_button = RadioButton(_("Quit"))
        
        main_table.attach(close_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(self.tray_radio_button, 0, 1, 2, 3, yoptions=gtk.FILL)
        main_table.attach(self.quit_radio_button, 1, 2, 2, 3)
        
        return main_table
    
    def create_play_box(self):
        main_table = gtk.Table(4, 2)
        main_table.set_row_spacings(10)
        
        play_title_label = Label(_("Playing"))
        play_title_label.set_size_request(350, 12)
        
        fade_check_hbox = gtk.HBox()
        self.fade_check_button = CheckButton(_("Crossfade"))        
        fade_check_hbox.pack_start(self.fade_check_button, False, False)
        fade_check_hbox.pack_start(create_right_align(), True, True)
        self.album_check_button = CheckButton(_("Crossfade gapless album"))
        
        fade_label = Label(_("Fade timeout:"))
        self.fade_spin = SpinBox(300, 1, 1000, 100)
        millisecond_lablel = Label(_(" ms"))        
        millisecond_lablel.set_size_request(50, 12)
        spin_hbox = gtk.HBox(spacing=3)
        spin_hbox.pack_start(fade_label, False, False)
        spin_hbox.pack_start(self.fade_spin, False, False)
        spin_hbox.pack_start(millisecond_lablel, False, False)
        
        main_table.attach(play_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(fade_check_hbox, 0, 1, 2, 3, yoptions=gtk.FILL)
        main_table.attach(self.album_check_button, 1, 2, 2, 3, yoptions=gtk.FILL)
        main_table.attach(spin_hbox, 0, 1, 3, 4, yoptions=gtk.FILL, xpadding=8)
        return main_table

    def create_lyrics_dir_table(self):    
        main_table = gtk.Table(3, 2)
        main_table.set_row_spacings(8)
        
        dir_title_label = Label(_("Lyrics directory"))
        dir_title_label.set_size_request(200, 12)
        label_align = gtk.Alignment()
        label_align.set_padding(0, 0, 0, 0)
        label_align.add(dir_title_label)
        
        self.dir_entry = InputEntry()
        self.dir_entry.set_text(os.path.expanduser(config.get("lyrics", "save_lrc_path", "~/.lyrics")))
        self.dir_entry.set_editable(False)        
        self.dir_entry.set_size(250, 25)
        
        modify_button = Button(_("Change"))
        modify_button.connect("clicked", self.change_lyrics_save_dir)
        hbox = gtk.HBox(spacing=5)
        hbox.pack_start(self.dir_entry, False, False)
        hbox.pack_start(modify_button, False, False)
        
        main_table.attach(label_align, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(hbox, 0, 2, 2, 3, xpadding=10, xoptions=gtk.FILL)
        return main_table
    
    def change_lyrics_save_dir(self, widget):
        local_dir = WinDir(False).run()
        if local_dir:
            config.set("lyrics", "save_lrc_path", local_dir)
            self.dir_entry.set_editable(True)        
            self.dir_entry.set_text(os.path.expanduser(local_dir))
            self.dir_entry.set_editable(False)
    
class HotKeySetting(gtk.VBox):        
    
    def __init__(self):
        super(HotKeySetting, self).__init__()
        self.main_table = gtk.Table(12, 2)        
        self.main_table.set_row_spacings(10)
        self.pack_start(self.main_table, False, True)
        self.create_hotkey_box()
        
        self.control_entrys = [
            self.toggle_window_entry,
            self.toggle_lyrics_status_entry,
            self.toggle_lyrics_lock_entry,
            self.playpause_entry,
            self.previous_entry,
            self.next_entry,
            self.increase_vol_entry,
            self.decrease_vol_entry]
        
        self.using_check_button.connect("toggled", self.change_enbale_status)
        self.using_check_button.set_active(config.getboolean("globalkey", "enable"))        
        self.toggle_window_entry.connect("shortcut-key-change", self.change_toggle_window_key)
        self.toggle_lyrics_status_entry.connect("shortcut-key-change", self.change_lyrics_status_key)
        self.toggle_lyrics_lock_entry.connect("shortcut-key-change", self.change_lyrics_lock_key)
        self.playpause_entry.connect("shortcut-key-change", self.change_playpause_key)
        self.previous_entry.connect("shortcut-key-change", self.change_previous_key)
        self.next_entry.connect("shortcut-key-change", self.change_next_key)
        self.increase_vol_entry.connect("shortcut-key-change", self.change_increase_vol_key)
        self.decrease_vol_entry.connect("shortcut-key-change", self.change_decrease_vol_key)
        
        for each_entry in self.control_entrys:
            each_entry.connect("wait-key-input", lambda w, d: self.pause_global_keys())
        
        if not config.getboolean("globalkey", "enable"):
            for each_entry in self.control_entrys:
                each_entry.set_sensitive(False)
                
    def pause_global_keys(self):            
        global_hotkeys.pause()
                
    def change_enbale_status(self, widget):    
        if widget.get_active():
            config.set("globalkey", "enable", "true")
            for each_entry in self.control_entrys:
                each_entry.set_sensitive(True)
        else:    
            config.set("globalkey", "enable", "false")
            for each_entry in self.control_entrys:
                each_entry.set_sensitive(False)
            
    def change_toggle_window_key(self, widget, value):    
        config.set("globalkey", "toggle_window", value)
        global_hotkeys.play()
        
    def change_lyrics_status_key(self, widget, value):    
        config.set("globalkey", "toggle_lyrics_status", value) 
        global_hotkeys.play()
       
    def change_lyrics_lock_key(self, widget, value):    
        config.set("globalkey", "toggle_lyrics_lock", value) 
        global_hotkeys.play()
       
    def change_playpause_key(self, widget, value):    
        config.set("globalkey", "playpause", value)
        global_hotkeys.play()
        
    def change_previous_key(self, widget, value):    
        config.set("globalkey", "previous", value)
        global_hotkeys.play()
        
    def change_next_key(self, widget, value):    
        config.set("globalkey", "next", value)
        global_hotkeys.play() 
       
    def change_increase_vol_key(self, widget, value):    
        config.set("globalkey", "increase_vol", value)
        global_hotkeys.play()
        
    def change_decrease_vol_key(self, widget, value):    
        config.set("globalkey", "decrease_vol", value)
        global_hotkeys.play()
        
    def create_hotkey_box(self):    
        hotkey_title_label = Label(_("Hotkeys"))
        hotkey_title_label.set_size_request(350, 12)
        label_align = gtk.Alignment()
        label_align.set_padding(20, 0, 0, 0)
        label_align.add(hotkey_title_label)
        self.main_table.attach(label_align, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        self.main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        
        # using check button.
        using_hbox = gtk.HBox()
        self.using_check_button = CheckButton(_("Activate hotkeys"))

        using_hbox.pack_start(self.using_check_button, False, False)
        using_hbox.pack_start(create_right_align(), False, True)
        self.main_table.attach(using_hbox, 0, 2, 2, 3, yoptions=gtk.FILL)
        
        self.toggle_window_entry = self.create_combo_entry(3, 4, _("Minimized/Show window"), 
                                                           config.get("globalkey", "toggle_window"))
        self.toggle_lyrics_status_entry = self.create_combo_entry(4, 5, _("Lyrics on/off"),
                                                                  config.get("globalkey", "toggle_lyrics_status"))
        self.toggle_lyrics_lock_entry = self.create_combo_entry(5, 6, _("Lock/unlock lyrics"),
                                                                config.get("globalkey", "toggle_lyrics_lock"))
        self.playpause_entry = self.create_combo_entry(6, 7, _("Play/Pause"), config.get("globalkey", "playpause"))
        self.next_entry = self.create_combo_entry(7, 8, _("Previous"), config.get("globalkey", "next"))
        self.previous_entry = self.create_combo_entry(8, 9, _("Next"), config.get("globalkey", "previous"))
        self.increase_vol_entry = self.create_combo_entry(9, 10, _("Increase volume"), config.get("globalkey", "increase_vol"))
        self.decrease_vol_entry = self.create_combo_entry(10, 11, _("Decrease volume"), config.get("globalkey", "decrease_vol"))
        
        # Button.
        default_button = Button(_("Default"))
        default_button.connect("clicked", self.restore_to_default)
        button_hbox = gtk.HBox()
        button_hbox.pack_start(create_right_align(), True, True)
        button_hbox.pack_start(default_button, False, False)
        self.main_table.attach(button_hbox, 0, 2, 11, 12, xpadding=10)
        
    def restore_to_default(self, widget):    
        self.toggle_window_entry.set_shortcut_key("Ctrl + Alt + W")        
        self.change_toggle_window_key(None, "Ctrl + Alt + w")
        
        self.toggle_lyrics_status_entry.set_shortcut_key("Ctrl + Alt + H")
        self.change_lyrics_status_key(None, "Ctrl + Alt + H")
        
        self.toggle_lyrics_lock_entry.set_shortcut_key("Ctrl + Alt + D")
        self.change_lyrics_lock_key(None, "Ctrl + Alt + D")
        
        self.playpause_entry.set_shortcut_key("Alt + F5")
        self.change_playpause_key(None, "Alt + F5")        
        
        self.previous_entry.set_shortcut_key("Alt + Left")
        self.change_previous_key(None, "Alt + Left")
        
        self.next_entry.set_shortcut_key("Alt + Right")
        self.change_next_key(None, "Alt + Right")
        
        self.increase_vol_entry.set_shortcut_key("Alt + Up")
        self.change_increase_vol_key(None, "Alt + Up")
        
        self.decrease_vol_entry.set_shortcut_key("Alt + Down")
        self.change_decrease_vol_key(None, "Alt + Down")
        
    def create_combo_entry(self, top_attach, bottom_attach, label_content, hotkey_content):    
        combo_hbox = gtk.HBox(spacing=5)
        combo_hbox.pack_start(create_right_align(), True, True)
        
        # single_hotkey_label
        hotkey_label = Label(label_content)
        combo_hbox.pack_start(hotkey_label, False, False)
        
        # Hotkey entry.
        hotkey_entry = ShortcutKeyEntry(hotkey_content)
        hotkey_entry.set_size(170, 24)
        hotkey_entry_align = gtk.Alignment()
        hotkey_entry_align.set_padding(0, 0, 0, 50)
        hotkey_entry_align.add(hotkey_entry)
        combo_hbox.pack_start(hotkey_entry_align, False, False)
        
        self.main_table.attach(combo_hbox, 0, 2, top_attach, bottom_attach, xpadding=5)
        # self.main_table.attach(combo_hbox, 0, 1, top_attach, bottom_attach, xpadding=5)
        # self.main_table.attach(hotkey_entry, 1, 2, top_attach, bottom_attach, xoptions=gtk.FILL)
        
        return hotkey_entry
        
class DesktopLyricsSetting(gtk.VBox):
    
    def __init__(self):
        super(DesktopLyricsSetting, self).__init__()
        self.render_lyrics = RenderContextNew()        
        main_align = gtk.Alignment()
        main_align.set(1, 1, 0, 0)
        main_align.set_padding(20, 0, 0, 0)
        main_align.add(self.create_style_table())
        self.pack_start(main_align, False, False)
        self.preview = gtk.EventBox()
        self.preview.set_visible_window(False)
        self.preview.set_size_request(-1, 135)
        preview_align = gtk.Alignment()
        preview_align.set(0.0, 0.0, 1.0, 1.0)
        preview_align.set_padding(0, 10, 5, 5)
        preview_align.add(self.preview)
        # self.pack_start(preview_align, False, True)
        
        # Signals.
        self.preview.connect("expose-event", self.draw_lyrics)        
        self.font_name_combo_box.connect("item-selected", self.update_preview_font_name)
        self.font_type_combo_box.connect("item-selected", self.update_preview_font_type)
        self.font_size_spin.connect("value-changed", self.update_preview_font_size)
        self.line_number_combo_box.connect("item-selected", self.update_preview_line_number)
        self.single_align_combo_box.connect("item-selected", self.update_preview_single_align)
        self.double_align_combo_box.connect("item-selected", self.update_preview_double_align)
        self.outline_spin.connect("value-changed", self.update_preview_outline_width)
        self.blur_color_button.connect("color-select", self.update_blur_color)
        self.predefine_color_combo_box.connect("item-selected", self.update_preview_predefine_color)
        self.inactive_upper_color_button.connect("color-select", self.update_inactive_upper_color)
        self.inactive_middle_color_button.connect("color-select", self.update_inactive_middle_color)
        self.inactive_bottom_color_button.connect("color-select", self.update_inactive_bottom_color)
        self.active_upper_color_button.connect("color-select", self.update_active_upper_color)
        self.active_middle_color_button.connect("color-select", self.update_active_middle_color)
        self.active_bottom_color_button.connect("color-select", self.update_active_bottom_color)
        
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
    
    def update_preview_font_name(self, widget, label, allocated_data, index):
        config.set("lyrics", "font_name", label)
        
    def update_preview_font_type(self, widget, label, allocated_data, index):
        config.set("lyrics", "font_type", allocated_data)
        
    def update_preview_font_size(self, widget, value):    
        config.set("lyrics", "font_size", str(value))
        
    def update_preview_line_number(self, widget, label, allocated_data, index):    
        if allocated_data == 1:
            switch_tab(self.line_align_hbox, self.single_align_combo_box)
        else:    
            switch_tab(self.line_align_hbox, self.double_align_combo_box)
            
        config.set("lyrics", "line_count", str(allocated_data))
        
    def update_preview_single_align(self, widget, label, allocated_data, index):    
        config.set("lyrics", "single_line_align", allocated_data)
        
    def update_preview_double_align(self, widget, label, allocated_data, index):    
        config.set("lyrics", "double_line_align", allocated_data)
        
    def update_preview_outline_width(self, widget, value):    
        config.set("lyrics", "outline_width", str(value))
        
    def update_blur_color(self, widget, value):    
        config.set("lyrics", "blur_color", str(value))
        
    def update_preview_predefine_color(self, widget, label, allocated_data, index):    
        values = PREDEFINE_COLORS[allocated_data]
        config.set("lyrics", "predefine_color", allocated_data)
        config.set("lyrics", "inactive_color_upper", values[0])
        config.set("lyrics", "inactive_color_middle", values[1])
        config.set("lyrics", "inactive_color_bottom", values[2])
        config.set("lyrics", "active_color_upper", values[3])
        config.set("lyrics", "active_color_middle", values[4])
        config.set("lyrics", "active_color_bottom", values[5])
        
        self.inactive_upper_color_button.set_color(values[0])
        self.inactive_middle_color_button.set_color(values[1])
        self.inactive_bottom_color_button.set_color(values[2])
        self.active_upper_color_button.set_color(values[3])
        self.active_middle_color_button.set_color(values[4])
        self.active_bottom_color_button.set_color(values[5])
        
    def update_inactive_upper_color(self, widget, color):    
        config.set("lyrics", "inactive_color_upper", color)
        
    def update_inactive_middle_color(self, widget, color):    
        config.set("lyrics", "inactive_color_middle", color)
        
    def update_inactive_bottom_color(self, widget, color):    
        config.set("lyrics", "inactive_color_bottom", color)
        
    def update_active_upper_color(self, widget, color):    
        config.set("lyrics", "active_color_upper", color)
        
    def update_active_middle_color(self, widget, color):    
        config.set("lyrics", "active_color_middle", color)
        
    def update_active_bottom_color(self, widget, color):    
        config.set("lyrics", "active_color_bottom", color)
        
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
        active_surface = self.draw_lyric_surface(_("Deepin Music Player"), True)
        if active_surface:
            cr.set_source_surface(active_surface, xpos, ypos)
            cr.paint()
        cr.restore()    
        
        # draw_inactive
        cr.save()
        cr.rectangle(xpos + rect.width * 0.5, ypos, rect.width*0.5, rect.height)
        cr.clip()
        inactive_surface = self.draw_lyric_surface(_("Deepin Music Player"))
        if inactive_surface:
            cr.set_source_surface(inactive_surface, xpos, ypos)
            cr.paint()
        cr.restore()    
        
        cr.restore()

        return False
        
    def create_single_line_box(self):
        single_align_items = OrderedDict()
        single_align_items["left"] = _("Left-aligned")
        single_align_items["centered"] = _("Centered")
        single_align_items["right"] = _("Right-aligned")
        try:
            single_index = single_align_items.keys().index(config.get("lyrics", "single_line_align"))
        except:    
            single_index = 0
        self.single_align_combo_box = ComboBox([(value, key) for key, value in single_align_items.items()],
                                               select_index=single_index)
        
    def create_double_line_box(self):    
        double_align_items = OrderedDict()
        double_align_items["left"] = _("Left-aligned")
        double_align_items["centered"] = _("Centered")
        double_align_items["right"] = _("Right-aligned")
        double_align_items["justified"] = _("Justified")
        try:
            align_index = double_align_items.keys().index(config.get("lyrics", "double_line_align"))
        except:    
            align_index = 0
        
        self.double_align_combo_box = ComboBox([(value, key) for key, value in double_align_items.items()],
                                               select_index=align_index)
        
    def create_predefine_box(self):    
        predefine_color_items = OrderedDict()
        predefine_color_items["default"] = _("Default")
        predefine_color_items["vitality_yellow"] = _("Vitality yellow")
        predefine_color_items["fresh_green"]  = _("Fresh green")
        predefine_color_items["playful_pink"] = _("Playful pink")
        predefine_color_items["cool_blue"] = _("Cool blue")
        save_predefine_color = config.get("lyrics", "predefine_color", "default")
        
        try:
            predefine_color_index = predefine_color_items.keys().index(save_predefine_color)
        except:    
            predefine_color_index = 0
            
        self.predefine_color_combo_box = ComboBox(
            [(value, key) for key, value in predefine_color_items.items()],
            select_index=predefine_color_index)    
        
        predefine_color_label = Label(_("Color scheme:"))
        predefine_color_hbox = gtk.HBox(spacing=5)
        predefine_color_hbox.pack_start(predefine_color_label, False, False)
        predefine_color_hbox.pack_start(self.predefine_color_combo_box, False, False)
        return predefine_color_hbox
    
    def create_font_type_box(self):
        font_type_items = OrderedDict()
        font_type_items["Regular"] = _("Regular")
        font_type_items["Italic"]  = _("Italic")
        font_type_items["Bold"]    = _("Bold")
        font_type_items["Bold Italic"] = _("Bold Italic")
        
        
        try:
            font_type_index = font_type_items.keys().index(config.get("lyrics", "font_type", "Regular"))
        except:    
            font_type_index = 0
        self.font_type_combo_box = ComboBox([(value, key) for key, value in font_type_items.items()],
                                            select_index=font_type_index)    
        
        font_type_label = Label(_("Style:"))
        font_type_hbox = gtk.HBox(spacing=5)
        font_type_hbox.pack_start(font_type_label, False, False)
        font_type_hbox.pack_start(self.font_type_combo_box, False, False)
        return font_type_hbox
    
    def create_style_table(self):
        main_table = gtk.Table(9, 2)
        main_table.set_row_spacings(10)
        self.create_single_line_box()
        self.create_double_line_box()
        style_title_label = Label(_("Lyrics style"))
        
        # font_name
        font_families = get_font_families()
        font_name = config.get("lyrics", "font_name")
        try:
            font_item_index = font_families.index(font_name)
        except:    
            font_item_index = 0
            
        font_name_hbox, self.font_name_combo_box = self.create_combo_widget(_("Font:"),
                                                                            [(font_name, None) for font_name in font_families],
                                                                            font_item_index)
        font_type_hbox = self.create_font_type_box()
        font_size = int(config.get("lyrics", "font_size", 30))
        font_size_hbox, self.font_size_spin = self.create_combo_spin(_("Size:"), font_size, 16, 70, 1)
        
        line_number = config.getint("lyrics", "line_count")
        line_number_hbox, self.line_number_combo_box = self.create_combo_widget(_("Lines:"),
                                                    [(name, index + 1) for index, name in enumerate([_("Single"), _("Double")])],
                                                                                line_number - 1)
        
        self.line_align_hbox = gtk.HBox()
        part_align_hbox = gtk.HBox(spacing=5) 
        line_align_label = Label(_("Alignment:"))
        
        if line_number == 2:
            self.line_align_hbox.add(self.double_align_combo_box)
        else:    
            self.line_align_hbox.add(self.single_align_combo_box)
            
        part_align_hbox.pack_start(line_align_label, False, False)    
        part_align_hbox.pack_start(self.line_align_hbox, False, False)    
        
        outline_hbox, self.outline_spin = self.create_combo_spin(_("Outline:"), 
                                                                 int(config.get("lyrics", "outline_width", "3")), 0, 8, 1)
        
        # blur_color_button.
        blur_color_hbox = gtk.HBox(spacing=5)
        blur_color_label = Label(_("Stroke:"))
        self.blur_color_button = ColorButton(config.get("lyrics", "blur_color", "#000000"))
        blur_color_hbox.pack_start(blur_color_label, False, False)
        blur_color_hbox.pack_start(self.blur_color_button, False, False)
        
        predefine_color_hbox = self.create_predefine_box()
        inactive_color_box = gtk.HBox(spacing=10)
        inactive_color_label = Label(_("Coming:"))
        self.inactive_upper_color_button = ColorButton(config.get("lyrics", "inactive_color_upper"))
        self.inactive_middle_color_button = ColorButton(config.get("lyrics", "inactive_color_middle"))
        self.inactive_bottom_color_button = ColorButton(config.get("lyrics", "inactive_color_bottom"))
        inactive_color_box.pack_start(inactive_color_label, False, False)
        inactive_color_box.pack_start(self.inactive_upper_color_button, False, False)
        inactive_color_box.pack_start(self.inactive_middle_color_button, False, False)
        inactive_color_box.pack_start(self.inactive_bottom_color_button, False, False)
        
        active_color_box = gtk.HBox(spacing=10)
        active_color_label = Label(_("Played:"))
        self.active_upper_color_button = ColorButton(config.get("lyrics", "active_color_upper"))
        self.active_middle_color_button = ColorButton(config.get("lyrics", "active_color_middle"))
        self.active_bottom_color_button = ColorButton(config.get("lyrics", "active_color_bottom"))
        active_color_box.pack_start(active_color_label, False, False)
        active_color_box.pack_start(self.active_upper_color_button, False, False)
        active_color_box.pack_start(self.active_middle_color_button, False, False)
        active_color_box.pack_start(self.active_bottom_color_button, False, False)
        
        main_table.attach(style_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(font_name_hbox, 0, 2, 2, 3, xpadding=20, xoptions=gtk.FILL)
        main_table.attach(font_type_hbox, 0, 1, 3, 4, xpadding=20)
        main_table.attach(font_size_hbox, 1, 2, 3, 4)
        main_table.attach(line_number_hbox, 0, 1, 4, 5, xpadding=20)
        main_table.attach(part_align_hbox, 1, 2, 4, 5)
        main_table.attach(outline_hbox, 0, 1, 5, 6, xpadding=20)
        main_table.attach(blur_color_hbox, 1, 2, 5, 6)
        main_table.attach(predefine_color_hbox, 0, 2, 6, 7, xpadding=20, xoptions=gtk.FILL)
        main_table.attach(inactive_color_box, 0, 2, 7, 8, xpadding=35, xoptions=gtk.FILL)
        main_table.attach(active_color_box, 0, 2, 8, 9, xpadding=35, xoptions=gtk.FILL)
        return main_table
    
    def create_combo_widget(self, label_content, items, select_index=0):
        label = Label(label_content)
        if len(items) > 10:
            height = 200
            max_width = 300
        else:    
            height = 0
            max_width = None
        combo_box = ComboBox(items, height, select_index, max_width)
        hbox = gtk.HBox(spacing=5)
        hbox.pack_start(label, False, False)
        hbox.pack_start(combo_box, False, False)
        return hbox, combo_box
    
    def create_combo_spin(self, label_content, init_value, low, upper, step):
        label = Label(label_content)
        spinbox = SpinBox(init_value, low, upper, step)
        
        hbox = gtk.HBox(spacing=5)
        hbox.pack_start(label, False, False)
        hbox.pack_start(spinbox, False, False)
        return hbox, spinbox
    
class ScrollLyricsSetting(gtk.VBox):
    def __init__(self):
        gtk.VBox.__init__(self)
        main_align = gtk.Alignment()
        main_align.set(1, 1, 0, 0)
        main_align.set_padding(20, 0, 0, 0)
        main_align.add(self.create_style_table())
        self.pack_start(main_align, False, False)
        
        # Signals.
        self.font_name_combo_box.connect("item-selected", self.update_scroll_font_name)
        self.font_type_combo_box.connect("item-selected", self.update_scroll_font_type)
        self.font_size_spin.connect("value-changed", self.update_scroll_font_size)
        self.line_align_combo_box.connect("item-selected", self.update_scroll_line_align)
        self.scroll_mode_combo_box.connect("item-selected", self.update_scroll_mode)
        self.inactive_color_button.connect("color-select", self.update_scroll_inative_color)
        self.active_color_button.connect("color-select", self.update_scroll_active_color)
        
    def update_scroll_font_name(self, widget, label, allocated_data, index):    
        config.set("scroll_lyrics", "font_name", label)
        
    def update_scroll_font_type(self, widget, label, allocated_data, index):    
        config.set("scroll_lyrics", "font_type", allocated_data)
        
    def update_scroll_font_size(self, widget, value):
        config.set("scroll_lyrics", "font_size", str(value))
        
    def update_scroll_line_align(self, widget, label, allocated_data, index):    
        config.set("scroll_lyrics", "line_align", str(allocated_data))
        
    def update_scroll_mode(self, widget, label, allocated_data, index):    
        config.set("scroll_lyrics", "scroll_mode", str(allocated_data))
        
    def update_scroll_inative_color(self, widget, color):    
        config.set("scroll_lyrics", "inactive_color", color)
        
    def update_scroll_active_color(self, widget, color):    
        config.set("scroll_lyrics", "active_color", color)
        
    def create_font_type_box(self):
        font_type_items = OrderedDict()
        font_type_items["Regular"] = _("Regular")
        font_type_items["Italic"]  = _("Italic")
        font_type_items["Bold"]    = _("Bold")
        font_type_items["Bold Italic"] = _("Bold Italic")
        try:
            font_type_index = font_type_items.keys().index(config.get("scroll_lyrics", "font_type", "Regular"))
        except:    
            font_type_index = 0
        self.font_type_combo_box = ComboBox([(value, key) for key, value in font_type_items.items()],
                                            select_index=font_type_index)    
        
        font_type_label = Label(_("Style:"))
        font_type_hbox = gtk.HBox(spacing=5)
        font_type_hbox.pack_start(font_type_label, False, False)
        font_type_hbox.pack_start(self.font_type_combo_box, False, False)
        return font_type_hbox
    
    def create_style_table(self):
        main_table = gtk.Table(5, 2)
        main_table.set_row_spacings(10)
        style_title_label = Label(_("Lyrics style"))
        # font_name
        font_families = get_font_families()
        font_name = config.get("scroll_lyrics", "font_name")
        try:
            font_item_index = font_families.index(font_name)
        except:    
            font_item_index = 0
            
        font_name_hbox, self.font_name_combo_box = self.create_combo_widget(_("Font:"),
                                                                            [(font_name, None) for font_name in font_families],
                                                                            font_item_index)
        font_type_hbox = self.create_font_type_box()
        
        font_size = int(config.get("scroll_lyrics", "font_size", 10))
        font_size_hbox, self.font_size_spin = self.create_combo_spin(_("Size:"), font_size, 5, 30, 1)
        
        # alignment.
        line_align_index = int(config.get("scroll_lyrics", "line_align", 1))
        line_align_hbox, self.line_align_combo_box = self.create_combo_widget(_("Alignment:"), 
                                                [(value, index) for index, value in enumerate([_("Left-aligned"), _("Centered"), _("Right-aligned")])],
                                                                               line_align_index)
        
        # scroll mode.
        scroll_mode_index = int(config.get("scroll_lyrics", "scroll_mode", 0))
        scroll_mode_hbox, self.scroll_mode_combo_box = self.create_combo_widget(_("Rolling:"),
                                                 [(value, index) for index, value in enumerate([_("Always"), _("By line")])],
                                                                                scroll_mode_index) 
        
        
        inactive_color_label = Label(_("Coming:"))
        active_color_label = Label(_("Played:"))
        self.inactive_color_button = ColorButton(config.get("scroll_lyrics", "inactive_color"))
        self.active_color_button = ColorButton(config.get("scroll_lyrics", "active_color"))
        
        inactive_color_hbox = gtk.HBox(spacing=5)
        inactive_color_hbox.pack_start(inactive_color_label, False, False)
        inactive_color_hbox.pack_start(self.inactive_color_button, False, False)
        
        active_color_hbox = gtk.HBox(spacing=5)
        active_color_hbox.pack_start(active_color_label, False, False)
        active_color_hbox.pack_start(self.active_color_button, False, False)
        
        main_table.attach(style_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(font_name_hbox, 0, 2, 2, 3, xpadding=20, xoptions=gtk.FILL)
        main_table.attach(font_type_hbox, 0, 1, 3, 4, xpadding=20)
        main_table.attach(font_size_hbox, 1, 2, 3, 4)
        main_table.attach(line_align_hbox, 0, 1, 4, 5, xpadding=20)
        main_table.attach(scroll_mode_hbox, 1, 2, 4, 5)
        main_table.attach(inactive_color_hbox, 0, 1, 5, 6, xpadding=20)
        main_table.attach(active_color_hbox, 1, 2, 5, 6)
        return main_table
        
    def create_combo_widget(self, label_content, items, select_index=0):
        label = Label(label_content)
        if len(items) > 10:
            height = 200
            max_width = 300
        else:    
            height = 0
            max_width = None
        combo_box = ComboBox(items, height, select_index, max_width)
        hbox = gtk.HBox(spacing=5)
        hbox.pack_start(label, False, False)
        hbox.pack_start(combo_box, False, False)
        return hbox, combo_box
    
    def create_combo_spin(self, label_content, init_value, low, upper, step):
        label = Label(label_content)
        spinbox = SpinBox(init_value, low, upper, step)
        hbox = gtk.HBox(spacing=5)
        hbox.pack_start(label, False, False)
        hbox.pack_start(spinbox, False, False)
        return hbox, spinbox
    
class AboutBox(gtk.VBox):    
    
    def __init__(self):
        gtk.VBox.__init__(self)
        main_box = gtk.VBox(spacing=15)
        logo_image = ImageBox(app_theme.get_pixbuf("skin/logo1.png"))
        light_color = app_theme.get_color("labelText")
        logo_name = Label(_("Deepin Music Player"), text_size=10)
        logo_box = gtk.HBox(spacing=2)
        logo_box.pack_start(logo_image, False, False)
        logo_box.pack_start(logo_name, False, False)
        
        version_label = Label(_("Version:"))
        version_content = Label("V1.0", light_color)
        # publish_label = Label(_("Release date:"))
        # publish_content = Label("2012.07.12", light_color)
        info_box = gtk.HBox(spacing=5)
        info_box.pack_start(version_label, False, False)
        info_box.pack_start(version_content, False, False)
        # info_box.pack_start(publish_label, False, False)
        # info_box.pack_start(publish_content, False, False)
        
        title_box = gtk.HBox()
        title_box.pack_start(logo_box, False, False)
        title_box.pack_start(create_right_align(), True, True)
        title_box.pack_start(info_box, False, False)
        
        describe = _("        Deepin Music Player is a music application designed for Linux users.It features lyrics searching and downloading, desktop lyrics display,album cover downloading, resume playing, music management and skin selection.\n\nDeepin Music Player is free software licensed under GNU GPLv3.")
        
        describe_label = Label(describe, enable_select=False, wrap_width=400, text_size=10)
        main_box.pack_start(title_box, False, False)
        main_box.pack_start(create_separator_box(), False, True)
        main_box.pack_start(describe_label, False, False)
        
        main_align = gtk.Alignment()
        main_align.set_padding(25, 0, 0, 0)
        main_align.set(0, 0, 1, 1)
        main_align.add(main_box)
        
        self.add(main_align)
        
    
class PreferenceDialog(DialogBox):
    
    def __init__(self):
        super(PreferenceDialog, self).__init__(_("Preference"), 575, 495, 
                                               mask_type=DIALOG_MASK_MULTIPLE_PAGE,
                                               close_callback=self.hide_all)
        
        self.set_position(gtk.WIN_POS_CENTER)
        
        self.main_box = gtk.VBox()
        close_button = Button(_("Close"))
        close_button.connect("clicked", lambda w: self.hide_all())
        
        # Init widget.
        self.general_setting = GeneralSetting()
        self.hotkey_setting = HotKeySetting()
        self.desktop_lyrics_setting = DesktopLyricsSetting()
        self.scroll_lyrics_setting = ScrollLyricsSetting()
        
        # Category bar
        self.category_bar = TreeView(font_x_padding=20)
        self.category_bar.draw_mask = self.draw_treeview_mask
        self.general_category_item = CategoryItem(_("General"), self.general_setting)
        self.category_bar.add_item(None, self.general_category_item)
        self.category_bar.add_item(None, CategoryItem(_("Hotkeys"), self.hotkey_setting))
        lyrics_node = self.category_bar.add_item(None, CategoryItem(_("Lyrics")))
        self.category_bar.add_item(lyrics_node, CategoryItem(_("Desktop"), self.desktop_lyrics_setting))
        self.category_bar.add_item(lyrics_node, CategoryItem(_("Window"), self.scroll_lyrics_setting))
        self.category_bar.add_item(None, CategoryItem(_("About us"), AboutBox()))
        self.category_bar.connect("single-click-item", self.category_single_click_cb)
        self.category_bar.set_highlight_index(0)
        
        category_box = gtk.VBox()
        background_box = BackgroundBox()
        background_box.set_size_request(132, 11)
        background_box.draw_mask = self.draw_treeview_mask
        category_box.pack_start(background_box, False, False)
        
        category_scrolled_window = ScrolledWindow()
        category_scrolled_window.add_child(self.category_bar)
        category_scrolled_window.set_policy(gtk.POLICY_NEVER, gtk.POLICY_NEVER)
        category_scrolled_window.set_size_request(132, 516)
        
        category_scrolled_window_align = gtk.Alignment()
        category_scrolled_window_align.set(0, 0, 1, 1,)
        category_scrolled_window_align.set_padding(0, 1, 0, 0)
        category_scrolled_window_align.add(category_scrolled_window)
        
        category_box.pack_start(category_scrolled_window_align, True, True)
        
        # Pack widget.
        left_box = gtk.VBox()
        self.right_box = gtk.VBox()
        left_box.add(category_box)
        self.right_box.add(self.general_category_item.get_allocated_widget())
        right_align = gtk.Alignment()
        right_align.set_padding(0, 0, 10, 0)
        right_align.add(self.right_box)

        body_box = gtk.HBox()
        body_box.pack_start(left_box, False, False)
        body_box.pack_start(right_align, False, False)
        self.main_box.add(body_box)
        
        # DialogBox code.
        self.body_box.pack_start(self.main_box, True, True)
        self.right_button_box.set_buttons([close_button])        
        
    def switch_lyrics_page(self, index=3):
        self.category_bar.tree_list[2].show_child_items_bool = True
        self.category_bar.sort()
        self.category_bar.queue_draw()
        self.category_bar.set_highlight_index(index)
        highlight_item  = self.category_bar.get_highlight_item()
        self.category_single_click_cb(None, highlight_item)
        
    def show_scroll_lyrics_page(self):
        self.switch_lyrics_page(4)
        self.show_all()
        
    def show_desktop_lyrics_page(self):    
        self.switch_lyrics_page(3)
        self.show_all()
    
    def draw_treeview_mask(self, cr, x, y, width, height):
        draw_alpha_mask(cr, x, y, width, height, ("#FFFFFF", 0.9))
    
    def category_single_click_cb(self, widget, item):
        if item.get_allocated_widget():
            switch_tab(self.right_box, item.get_allocated_widget())
            
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
