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
from dtk.ui.box import BackgroundBox
from deepin_utils.file import get_parent_dir
from dtk.ui.button import CheckButton, RadioButton
from dtk.ui.spin import SpinBox
from dtk.ui.entry import InputEntry, ShortcutKeyEntry

from dtk.ui.treeview import TreeView
from dtk.ui.button import Button
from dtk.ui.dialog import DialogBox, DIALOG_MASK_MULTIPLE_PAGE
from dtk.ui.color_selection import ColorButton
from dtk.ui.combo import ComboBox


from utils import color_hex_to_cairo
from widget.ui_utils import (get_font_families, switch_tab,
                             create_separator_box, create_right_align,
                             draw_alpha_mask, set_widget_resize)
from widget.dialog import WinDir
from widget.prefer_item import NormalItem, ExpandItem
from widget.skin import app_theme
from widget.plugins_view import PluginsManager
from render_lyrics import RenderContextNew
from constant import PREDEFINE_COLORS, PROGRAM_VERSION, PROGRAM_NAME_LONG
from config import config
from nls import _

TABLE_ROW_SPACING = 25
CONTENT_ROW_SPACING = 8

class GeneralSetting(gtk.VBox):

    def __init__(self):
        super(GeneralSetting, self).__init__()
        self.set_spacing(TABLE_ROW_SPACING)
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
        main_table.set_row_spacings(CONTENT_ROW_SPACING)
        start_title_label = Label(_("On starting the player"))
        start_title_label.set_size_request(350, 12)
        label_align = gtk.Alignment()
        label_align.set_padding(20, 0, 0, 0)
        label_align.add(start_title_label)

        # splash check_button
        self.splash_check_button = CheckButton(_("Display splash-screen"))
        splash_hbox = gtk.HBox()
        splash_hbox.pack_start(self.splash_check_button, False, False)
        splash_hbox.pack_start(create_right_align(), True, True)

        # open_lyrics_check_button.
        open_lyrics_hbox = gtk.HBox()
        self.open_lyrics_check_button = CheckButton(_("Show lyrics"))
        open_lyrics_hbox.pack_start(self.open_lyrics_check_button, False, False)


        # startup_check_button.
        self.auto_play_check_button = CheckButton(_("Start playback automatically"))
        auto_play_hbox = gtk.HBox()
        auto_play_hbox.pack_start(self.auto_play_check_button, False, False)
        auto_play_hbox.pack_start(create_right_align(), True, True)

        # resume last check_button.
        self.resume_last_check_button = CheckButton(_("Resume playback"))
        resume_hbox = gtk.HBox()
        resume_hbox.pack_start(self.resume_last_check_button, False, False)

        main_table.attach(label_align, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, gtk.FILL)
        #main_table.attach(splash_hbox, 0, 1, 2, 3)
        main_table.attach(open_lyrics_hbox, 0, 1, 2, 3)
        main_table.attach(auto_play_hbox, 1, 2, 2, 3, yoptions=gtk.EXPAND)
        main_table.attach(resume_hbox, 0, 1, 3, 4)
        return main_table

    def create_close_box(self):
        main_table = gtk.Table(3, 2)
        main_table.set_row_spacings(CONTENT_ROW_SPACING)
        close_title_label = Label(_("When pressing the close button"))
        close_title_label.set_size_request(350, 12)

        # mini_check_button

        self.tray_radio_button = RadioButton(_("Minimize to tray"))
        self.quit_radio_button = RadioButton(_("Quit"))

        main_table.attach(close_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(self.tray_radio_button, 0, 1, 2, 3, yoptions=gtk.FILL)
        main_table.attach(self.quit_radio_button, 1, 2, 2, 3)

        return main_table

    def create_play_box(self):
        main_table = gtk.Table(5, 2)
        main_table.set_row_spacings(CONTENT_ROW_SPACING)

        play_title_label = Label(_("Playback settings"))
        play_title_label.set_size_request(350, 12)

        fade_check_hbox = gtk.HBox()
        self.fade_check_button = CheckButton(_("Enable crossfade"))
        fade_check_hbox.pack_start(self.fade_check_button, False, False)
        fade_check_hbox.pack_start(create_right_align(), True, True)
        self.album_check_button = CheckButton(_("Seamless switching between the same album"))
        album_check_hbox = gtk.HBox()
        album_check_hbox.pack_start(self.album_check_button, False, False)
        album_check_hbox.pack_start(create_right_align(), True, True)

        fade_label = Label("%s" % _("Crossfade"))
        self.fade_spin = SpinBox(200, 0, 900, 100)
        millisecond_lablel = Label(_(" ms"))
        millisecond_lablel.set_size_request(50, 12)
        spin_hbox = gtk.HBox(spacing=3)
        spin_hbox.pack_start(fade_label, False, False)
        spin_hbox.pack_start(self.fade_spin, False, False)
        spin_hbox.pack_start(millisecond_lablel, False, False)

        main_table.attach(play_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(fade_check_hbox, 0, 2, 2, 3, yoptions=gtk.FILL)
        main_table.attach(album_check_hbox, 0, 2, 3, 4, yoptions=gtk.FILL)
        main_table.attach(spin_hbox, 0, 1, 4, 5, yoptions=gtk.FILL, xpadding=8)
        return main_table

    def create_lyrics_dir_table(self):
        main_table = gtk.Table(3, 2)
        main_table.set_row_spacings(CONTENT_ROW_SPACING)

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
        self.main_table.set_row_spacings(CONTENT_ROW_SPACING)
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

        # for each_entry in self.control_entrys:
        #     each_entry.connect("wait-key-input", lambda w, d: self.pause_global_keys())

        if not config.getboolean("globalkey", "enable"):
            for each_entry in self.control_entrys:
                each_entry.set_sensitive(False)

    # def pause_global_keys(self):
    #     global_hotkeys.stop_bind()

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

    def change_lyrics_status_key(self, widget, value):
        config.set("globalkey", "toggle_lyrics_status", value)

    def change_lyrics_lock_key(self, widget, value):
        config.set("globalkey", "toggle_lyrics_lock", value)

    def change_playpause_key(self, widget, value):
        config.set("globalkey", "playpause", value)

    def change_previous_key(self, widget, value):
        config.set("globalkey", "previous", value)

    def change_next_key(self, widget, value):
        config.set("globalkey", "next", value)

    def change_increase_vol_key(self, widget, value):
        config.set("globalkey", "increase_vol", value)

    def change_decrease_vol_key(self, widget, value):
        config.set("globalkey", "decrease_vol", value)

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
        self.using_check_button = CheckButton(_("Enable hotkeys"))

        using_hbox.pack_start(self.using_check_button, False, False)
        using_hbox.pack_start(create_right_align(), False, True)
        self.main_table.attach(using_hbox, 0, 2, 2, 3, yoptions=gtk.FILL)

        self.toggle_window_entry = self.create_combo_entry(3, 4, _("Minimize/Show window"),
                                                           config.get("globalkey", "toggle_window"))
        self.toggle_lyrics_status_entry = self.create_combo_entry(4, 5, _("Show/Hide lyrics"),
                                                                  config.get("globalkey", "toggle_lyrics_status"))
        self.toggle_lyrics_lock_entry = self.create_combo_entry(5, 6, _("Lock/Unlock lyrics"),
                                                                config.get("globalkey", "toggle_lyrics_lock"))
        self.playpause_entry = self.create_combo_entry(6, 7, _("Play/Pause"), config.get("globalkey", "playpause"))
        self.previous_entry = self.create_combo_entry(7, 8, _("Previous"), config.get("globalkey", "previous"))
        self.next_entry = self.create_combo_entry(8, 9, _("Next"), config.get("globalkey", "next"))
        self.increase_vol_entry = self.create_combo_entry(9, 10, _("Volume Up"), config.get("globalkey", "increase_vol"))
        self.decrease_vol_entry = self.create_combo_entry(10, 11, _("Volume Down"), config.get("globalkey", "decrease_vol"))

        # Button.
        default_button = Button(_("Reset"))
        default_button.connect("clicked", self.restore_to_default)
        button_hbox = gtk.HBox()
        button_hbox.pack_start(create_right_align(), True, True)
        button_hbox.pack_start(default_button, False, False)
        self.main_table.attach(button_hbox, 0, 2, 11, 12, xpadding=10)

    def restore_to_default(self, widget):
        self.toggle_window_entry.set_shortcut_key("Alt + W")
        self.change_toggle_window_key(None, "Alt + W")

        self.toggle_lyrics_status_entry.set_shortcut_key("Alt + H")
        self.change_lyrics_status_key(None, "Alt + H")

        self.toggle_lyrics_lock_entry.set_shortcut_key("Alt + U")
        self.change_lyrics_lock_key(None, "Alt + U")

        self.playpause_entry.set_shortcut_key("Alt + P")
        self.change_playpause_key(None, "Alt + P")

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
        hotkey_entry = ShortcutKeyEntry()
        hotkey_entry.set_shortcut_key(None if hotkey_content == "None" else hotkey_content)
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

        config.connect("config-changed", self.on_config_changed)


    def on_config_changed(self, obj, selection, option, value):
        if selection == "lyrics" and option == "line_count":
            value = int(value) - 1
            index = self.line_number_combo_box.get_select_index()
            if index != value:
                self.line_number_combo_box.set_select_index(value)

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
        # font_height = self.render_lyrics.get_font_height()
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
        active_surface = self.draw_lyric_surface(PROGRAM_NAME_LONG, True)
        if active_surface:
            cr.set_source_surface(active_surface, xpos, ypos)
            cr.paint()
        cr.restore()

        # draw_inactive
        cr.save()
        cr.rectangle(xpos + rect.width * 0.5, ypos, rect.width*0.5, rect.height)
        cr.clip()
        inactive_surface = self.draw_lyric_surface(PROGRAM_NAME_LONG)
        if inactive_surface:
            cr.set_source_surface(inactive_surface, xpos, ypos)
            cr.paint()
        cr.restore()

        cr.restore()

        return False

    def create_single_line_box(self):
        single_align_items = OrderedDict()
        single_align_items["left"] = _("Left")
        single_align_items["centered"] = _("Centered")
        single_align_items["right"] = _("Right")
        try:
            single_index = single_align_items.keys().index(config.get("lyrics", "single_line_align"))
        except:
            single_index = 0
        self.single_align_combo_box = ComboBox([(value, key) for key, value in single_align_items.items()],
                                               select_index=single_index)

    def create_double_line_box(self):
        double_align_items = OrderedDict()
        double_align_items["left"] = _("Left")
        double_align_items["centered"] = _("Centered")
        double_align_items["right"] = _("Right")
        double_align_items["justified"] = _("Justified")
        try:
            align_index = double_align_items.keys().index(config.get("lyrics", "double_line_align"))
        except:
            align_index = 0

        self.double_align_combo_box = ComboBox([(value, key) for key, value in double_align_items.items()],
                                               select_index=align_index)

    def create_predefine_box(self):
        predefine_color_items = OrderedDict()
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

        predefine_color_label = Label("%s:" % _("Color scheme"))
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

        font_type_label = Label("%s:" % _("Style"))
        return set_widget_resize(font_type_label, self.font_type_combo_box)

    def create_style_table(self):
        main_table = gtk.Table(12, 2)
        main_table.set_row_spacings(CONTENT_ROW_SPACING)
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

        font_name_hbox, self.font_name_combo_box = self.create_combo_widget(_("Font"),
                                                                            [(font_name, None) for font_name in font_families],
                                                                            font_item_index)
        font_type_hbox = self.create_font_type_box()
        font_size = int(config.get("lyrics", "font_size", 30))
        font_size_hbox, self.font_size_spin = self.create_combo_spin(_("Size"), font_size, 16, 70, 1)

        line_number = config.getint("lyrics", "line_count")
        line_number_hbox, self.line_number_combo_box = self.create_combo_widget(_("Lines"),
                                                    [(_("Single"), 1), (_("Double"), 2)], select_index=line_number - 1)

        self.line_align_hbox = gtk.HBox()
        line_align_label = Label("%s:" % _("Alignment"))

        if line_number == 2:
            self.line_align_hbox.add(self.double_align_combo_box)
        else:
            self.line_align_hbox.add(self.single_align_combo_box)

        part_align_hbox = set_widget_resize(line_align_label, self.line_align_hbox)

        outline_hbox, self.outline_spin = self.create_combo_spin(_("Outline"),
                                                                 int(config.get("lyrics", "outline_width", "3")), 0, 8, 1)

        # blur_color_button.
        blur_color_label = Label("%s:" % _("Stroke"))
        self.blur_color_button = ColorButton(config.get("lyrics", "blur_color", "#000000"))
        blur_color_hbox = set_widget_resize(blur_color_label, self.blur_color_button)

        predefine_color_hbox = self.create_predefine_box()
        inactive_color_label = Label("%s:" % _("Unplayed"))
        self.inactive_upper_color_button = ColorButton(config.get("lyrics", "inactive_color_upper"))
        self.inactive_middle_color_button = ColorButton(config.get("lyrics", "inactive_color_middle"))
        self.inactive_bottom_color_button = ColorButton(config.get("lyrics", "inactive_color_bottom"))

        inactive_color_subbox = gtk.HBox(spacing=10)
        inactive_color_subbox.pack_start(self.inactive_upper_color_button, False, False)
        inactive_color_subbox.pack_start(self.inactive_middle_color_button, False, False)
        inactive_color_subbox.pack_start(self.inactive_bottom_color_button, False, False)
        inactive_color_box = set_widget_resize(inactive_color_label, inactive_color_subbox, sizes2=(160, 22))

        active_color_label = Label("%s:" % _("Played"))
        self.active_upper_color_button = ColorButton(config.get("lyrics", "active_color_upper"))
        self.active_middle_color_button = ColorButton(config.get("lyrics", "active_color_middle"))
        self.active_bottom_color_button = ColorButton(config.get("lyrics", "active_color_bottom"))

        active_color_subbox = gtk.HBox(spacing=10)
        active_color_subbox.pack_start(self.active_upper_color_button, False, False)
        active_color_subbox.pack_start(self.active_middle_color_button, False, False)
        active_color_subbox.pack_start(self.active_bottom_color_button, False, False)
        active_color_box = set_widget_resize(active_color_label, active_color_subbox, sizes2=(160, 22))

        main_table.attach(style_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(font_name_hbox, 0, 2, 2, 3)
        main_table.attach(font_type_hbox, 0, 2, 3, 4)
        main_table.attach(font_size_hbox, 0, 2, 4, 5)
        main_table.attach(line_number_hbox, 0, 2, 5, 6)
        main_table.attach(part_align_hbox, 0, 2, 6, 7)
        main_table.attach(outline_hbox, 0, 2, 7, 8)
        main_table.attach(blur_color_hbox, 0, 2, 8, 9)
        main_table.attach(predefine_color_hbox, 0, 2, 9, 10)
        main_table.attach(inactive_color_box, 0, 2, 10, 11)
        main_table.attach(active_color_box, 0, 2, 11, 12)
        return main_table

    def create_combo_widget(self, label_content, items, select_index=0):
        label = Label("%s:" % label_content)
        if len(items) > 10:
            height = 200
            max_width = 300
        else:
            height = None
            max_width = None
        combo_box = ComboBox(items, height, select_index=select_index, max_width=max_width)
        hbox = set_widget_resize(label, combo_box)
        return hbox, combo_box

    def create_combo_spin(self, label_content, init_value, low, upper, step):
        label = Label("%s:" % label_content)
        spinbox = SpinBox(init_value, low, upper, step)

        hbox = set_widget_resize(label, spinbox)
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

        font_type_label = Label("%s:" % _("Style"))
        return set_widget_resize(font_type_label, self.font_type_combo_box)

    def create_style_table(self):
        main_table = gtk.Table(8, 2)
        main_table.set_row_spacings(CONTENT_ROW_SPACING)
        style_title_label = Label(_("Lyrics style"))
        # font_name
        font_families = get_font_families()
        font_name = config.get("scroll_lyrics", "font_name")
        try:
            font_item_index = font_families.index(font_name)
        except:
            font_item_index = 0

        font_name_hbox, self.font_name_combo_box = self.create_combo_widget(_("Font"),
                                                                            [(font_name, None) for font_name in font_families],
                                                                            font_item_index)
        font_type_hbox = self.create_font_type_box()

        font_size = int(config.get("scroll_lyrics", "font_size", 10))
        font_size_hbox, self.font_size_spin = self.create_combo_spin(_("Size"), font_size, 5, 30, 1)

        # alignment.
        line_align_index = int(config.get("scroll_lyrics", "line_align", 1))
        line_align_hbox, self.line_align_combo_box = self.create_combo_widget(_("Alignment"),
                                                [(value, index) for index, value
                                                 in enumerate([_("Left"), _("Centered"), _("Right")])],
                                                                               line_align_index)

        # scroll mode.
        scroll_mode_index = int(config.get("scroll_lyrics", "scroll_mode", 0))
        scroll_mode_hbox, self.scroll_mode_combo_box = self.create_combo_widget(_("Rolling"),
                                                 [(value, index) for index, value in enumerate([_("Always"), _("By line")])],
                                                                                scroll_mode_index)


        inactive_color_label = Label("%s:" % _("Unplayed"))
        active_color_label = Label("%s:" % _("Played"))
        self.inactive_color_button = ColorButton(config.get("scroll_lyrics", "inactive_color"))
        self.active_color_button = ColorButton(config.get("scroll_lyrics", "active_color"))

        color_hbox = gtk.HBox(spacing=5)
        color_hbox.pack_start(self.inactive_color_button, False, False)
        color_hbox.pack_start(active_color_label, False, False)
        color_hbox.pack_start(self.active_color_button, False, False)

        main_table.attach(style_title_label, 0, 2, 0, 1, yoptions=gtk.FILL, xpadding=8)
        main_table.attach(create_separator_box(), 0, 2, 1, 2, yoptions=gtk.FILL)
        main_table.attach(font_name_hbox, 0, 2, 2, 3)
        main_table.attach(font_type_hbox, 0, 2, 3, 4)
        main_table.attach(font_size_hbox, 0, 2, 4, 5)
        main_table.attach(line_align_hbox, 0, 2, 5, 6)
        main_table.attach(scroll_mode_hbox, 0, 2, 6, 7)
        main_table.attach(set_widget_resize(inactive_color_label, color_hbox), 0, 2, 7, 8)
        return main_table

    def create_combo_widget(self, label_content, items, select_index=0):
        label = Label("%s:" % label_content)
        if len(items) > 10:
            height = 200
            max_width = 300
        else:
            height = None
            max_width = None
        combo_box = ComboBox(items, height, select_index=select_index, max_width=max_width)
        hbox = set_widget_resize(label, combo_box)
        return hbox, combo_box

    def create_combo_spin(self, label_content, init_value, low, upper, step):
        label = Label("%s:" % label_content)
        spinbox = SpinBox(init_value, low, upper, step)
        hbox = set_widget_resize(label, spinbox)
        return hbox, spinbox

class AboutBox(gtk.VBox):

    def __init__(self):
        gtk.VBox.__init__(self)
        main_box = gtk.VBox(spacing=15)
        logo_image = gtk.image_new_from_pixbuf(gtk.gdk.pixbuf_new_from_file(os.path.join(get_parent_dir(__file__, 3), "image", "logo16.png")))
        light_color = app_theme.get_color("labelText")
        logo_name = Label(PROGRAM_NAME_LONG, text_size=10)
        logo_box = gtk.HBox(spacing=2)
        logo_box.pack_start(logo_image, False, False)
        logo_box.pack_start(logo_name, False, False)

        version_label = Label("%s" % _("Version"))
        version_content = Label(PROGRAM_VERSION, light_color)
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

        describe = _("Deepin Music is a music application designed for Linux "
                     "users. It's characterized with lyrics searching, desktop "
                     "lyrics display,album cover downloading, resume playing, "
                     "music management and skin selection.\n"
                     "\n"
                     "Deepin Music is free software licensed under GNU GPLv3.")

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
        super(PreferenceDialog, self).__init__(_("Preferences"), 575, 495,
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
        self.plugins_manager = PluginsManager()

        # Category bar
        self.category_bar = TreeView(enable_drag_drop=False, enable_multiple_select=False)
        self.category_bar.set_expand_column(0)
        self.category_bar.draw_mask = self.draw_treeview_mask
        self.category_bar.set_size_request(132, 516)
        self.category_bar.connect("single-click-item", self.on_categorybar_single_click)

        # Init catagory bar.
        self.__init_category_bar()

        category_box = gtk.VBox()
        background_box = BackgroundBox()
        background_box.set_size_request(132, 11)
        background_box.draw_mask = self.draw_treeview_mask
        category_box.pack_start(background_box, False, False)

        category_bar_align = gtk.Alignment()
        category_bar_align.set(0, 0, 1, 1,)
        category_bar_align.set_padding(0, 1, 0, 0)
        category_bar_align.add(self.category_bar)
        category_box.pack_start(category_bar_align, True, True)

        # Pack widget.
        left_box = gtk.VBox()
        self.right_box = gtk.VBox()
        left_box.add(category_box)
        self.right_box.add(self.general_setting)
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

    def __init_category_bar(self):
        general_normal_item = NormalItem(_("General"), self.general_setting)
        hotkey_normal_item = NormalItem(_("Hotkeys"), self.hotkey_setting)
        self.lyrics_expand_item = ExpandItem(_("Lyrics"), None)
        self.lyrics_expand_item.add_childs([(_("Desktop"), self.desktop_lyrics_setting),
                                       (_("Window"), self.scroll_lyrics_setting)])
        self.plugins_expand_item = NormalItem(_("Plugins"), self.plugins_manager)
        about_normal_item = NormalItem(_("About"), AboutBox())

        items = [general_normal_item,
                 hotkey_normal_item,
                 self.lyrics_expand_item,
                 self.plugins_expand_item,
                 about_normal_item]
        self.category_bar.add_items(items)
        self.category_bar.select_items([general_normal_item])

    def switch_lyrics_page(self, index=3):
        self.lyrics_expand_item.try_to_expand()
        show_item = self.category_bar.visible_items[index]
        self.category_bar.select_items([show_item])
        self.on_categorybar_single_click(self.category_bar, show_item, None, None, None)

    def show_scroll_lyrics_page(self):
        self.switch_lyrics_page(4)
        self.show_all()

    def show_desktop_lyrics_page(self):
        self.switch_lyrics_page(3)
        self.show_all()

    def draw_treeview_mask(self, cr, x, y, width, height):
        draw_alpha_mask(cr, x, y, width, height, ("#FFFFFF", 0.9))

    def on_categorybar_single_click(self, widget, item, column, x, y):
        if item.allocate_widget:
            switch_tab(self.right_box, item.allocate_widget)

    def load_plugins(self):
        self.plugins_manager.flush_plugins()

