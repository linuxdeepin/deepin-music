
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
from dtk.ui.draw import draw_pixbuf
from dtk.ui.utils import container_remove_all
from dtk.ui.timeline import Timeline, CURVE_SINE
from dtk.ui.theme import ui_theme
from dtk.ui.box import ImageBox
from dtk.ui.button import ToggleButton, ImageButton, MinButton, CloseButton
import dtk.ui.tooltip as Tooltip

from helper import Dispatcher
from config import config
from widget.skin import app_theme
from widget.information import PlayInfo
from widget.ui_utils import set_widget_gravity, set_widget_vcenter, switch_tab, is_in_rect
from widget.ui import QuellButton 
from player import Player
from widget.timer import VolumeSlider
from utils import get_main_window

class MiniWindow(Window):
    
    def __init__(self):
        Window.__init__(self,
                        window_type=gtk.WINDOW_POPUP,
                        shape_frame_function=self.shape_mini_frame,
                        expose_frame_function=self.expose_mini_frame)
        
        self.set_property("skip-pager-hint", True)
        self.set_property("skip-taskbar-hint", True)
        
        self.body_box = gtk.VBox()
        
        self.control_box = gtk.HBox()
        self.action_box = gtk.HBox()
        self.event_box = gtk.HBox()  
        self.info_box = gtk.HBox()              

        # Build info box
        self.logo_padding_left = 10
        self.logo_padding_right = 10
        logo_box = self.create_logo_box()
        logo_box_align = set_widget_gravity(logo_box, (0.5, 0.5, 0, 0),
                                            (0, 0, self.logo_padding_left,
                                             self.logo_padding_right))
        self.playinfo = PlayInfo(239, None, 0, "#000000")
        self.info_box.pack_start(logo_box_align, False, False)
        self.info_box.pack_start(self.playinfo, False, False)
        
        # Build control box
        self.lyrics_button = self.create_lyrics_button()
        self.signal_auto = False        
        if config.getboolean("lyrics", "status"):
            self.lyrics_button.set_active(True)
        self.signal_auto = True    
        
        
        self.lyrics_padding_left = 10
        self.lyrics_padding_right = 8
        lyrics_button_align = set_widget_gravity(self.lyrics_button, (0.5, 0.5, 0, 0),
                                                 (0, 0, self.lyrics_padding_left,
                                                  self.lyrics_padding_right))
        
        self.previous_button = self.create_button("previous")
        self.next_button = self.create_button("next")
        self.playpause_button = self.create_playpause_button()
        
        # swap played status handler
        self.__id_signal_play = self.playpause_button.connect("toggled", self.on_player_playpause)        
        Player.connect("played", self.__swap_play_status, True)
        Player.connect("paused", self.__swap_play_status, False)
        Player.connect("stopped", self.__swap_play_status, False)
        Player.connect("play-end", self.__swap_play_status, False)
        
        self.volume_slider = VolumeSlider(auto_hide=False, mini_mode=True)
        self.volume_padding_left = 8
        volume_slider_align = set_widget_gravity(self.volume_slider, (0.5, 0.5, 0, 0),
                                                 (0, 0, self.volume_padding_left, 0))
        
        self.action_box.pack_start(lyrics_button_align, False, False)
        self.action_box.pack_start(set_widget_vcenter(self.previous_button), False, False)
        self.action_box.pack_start(set_widget_vcenter(self.playpause_button), False, True)
        self.action_box.pack_start(set_widget_vcenter(self.next_button), False, False)
        self.action_box.pack_start(volume_slider_align, False, False)
        
        # Build event box.
        quell_button = QuellButton()
        quell_button.connect("clicked", self.on_quell_button_clicked)
        
        min_button = MinButton()
        min_button.connect("clicked", lambda w: self.min_window())
        close_button = CloseButton()
        close_button.connect("clicked", self.try_to_quit)
        self.event_box.pack_start(quell_button, False, False)
        self.event_box.pack_start(min_button, False, False)
        self.event_box.pack_start(close_button, False, False)
        event_box_align = set_widget_gravity(self.event_box, paddings=(0, 0, 8, 0))
        
        self.control_box.pack_start(self.action_box, False, True)
        self.control_box.pack_end(event_box_align, False, True)
        
        self.connect("configure-event", self.on_configure_event)
        self.connect("enter-notify-event", self.on_enter_notify_event)
        self.connect("leave-notify-event", self.on_leave_notify_event)
        self.connect("button-press-event", self.on_button_press_event)
        self.connect("motion-notify-event", self.on_motion_notify_event)
        self.connect("button-release-event", self.on_button_release_event)
        
        Dispatcher.connect("close-lyrics", lambda w : self.lyrics_button.set_active(False))
        Dispatcher.connect("show-lyrics", lambda w: self.lyrics_button.set_active(True))
        
        if config.get("mini", "x") == "-1":
            self.set_position(gtk.WIN_POS_CENTER)
        else:    
            self.move(int(config.get("mini","x")),int(config.get("mini","y")))
            
        # pixbufs    
        self.info_pixbuf = None
        self.control_pixbuf = None

        self.body_box.add(self.info_box)    
        self.window_frame.add(self.body_box)
        self.set_size_request(305, 40)
        
        
        # animation params.
        self.active_alpha = 1.0
        self.target_alpha = 0.0
        self.in_animation = False
        self.animation_time = 1000
        self.animation_timeout_id = None
        self.draw_animation = False
        self.active_draw_func = None
        self.target_draw_func = None
        
        # drag params.
        self.drag_move = False
        self.old_x = self.old_y = self.mouse_x = self.mouse_y = 0
        
    def on_quell_button_clicked(self, widget):    
        main_window = get_main_window()
        main_window.change_app_mode("normal")
        switch_tab(self.body_box, self.info_box)
        
    def try_to_quit(self, widget):
        self.hide_all()
        main_window = get_main_window()
        main_window.quit()
        
    def create_lyrics_button(self):    
        toggle_button = ToggleButton(
            app_theme.get_pixbuf("mini/lyrics_inactive_normal.png"),
            app_theme.get_pixbuf("mini/lyrics_active_normal.png"),
            app_theme.get_pixbuf("mini/lyrics_inactive_hover.png"),
            app_theme.get_pixbuf("mini/lyrics_active_hover.png"),
            app_theme.get_pixbuf("mini/lyrics_inactive_press.png"),
            app_theme.get_pixbuf("mini/lyrics_active_press.png"),
            ) 
        
        toggle_button.connect("toggled", self.change_lyrics_status)
        return toggle_button
    
    def change_lyrics_status(self, widget):    
        if self.signal_auto:
            if widget.get_active():
                Dispatcher.show_lyrics()
            else:    
                Dispatcher.close_lyrics()
    
    def create_button(self, name, tip_msg=None):
        button = ImageButton(
            app_theme.get_pixbuf("mini/%s_normal.png" % name),
            app_theme.get_pixbuf("mini/%s_hover.png" % name),
            app_theme.get_pixbuf("mini/%s_press.png" % name),
            )
        
        button.connect("clicked", self.player_control, name)

        if tip_msg:
            Tooltip.text(button, tip_msg)
        return button
    
    def create_logo_box(self):
        return ImageBox(app_theme.get_pixbuf("mini/logo.png"))
    
    def player_control(self, button, name):
        if name == "next":
            getattr(Player, name)(True)
        else:    
            getattr(Player, name)()
            
    def is_in_window(self):        
        root_window = gtk.gdk.get_default_root_window()
        r_x, r_y = root_window.get_pointer()[:2]
        o_x, o_y = self.get_position()
        rect = self.allocation
        rect.x = o_x + 1
        rect.y = o_y + 1
        rect.width -= 2
        rect.height -= 2
        return is_in_rect((r_x, r_y), rect)
    
    def adjust_move_coordinate(self, widget, x, y):
        x = max(x, 0)
        y = max(y, 0)
        screen = widget.get_screen()
        w, h = widget.get_size()
        screen_w, screen_h = screen.get_width(), screen.get_height()
        
        if x + w > screen_w:
            x = screen_w - w
           
        if y + h > screen_h:    
            y = screen_h - h
        return (int(x), int(y))
    
    
    def create_playpause_button(self):
        play_normal_pixbuf = app_theme.get_pixbuf("mini/play_normal.png")
        pause_normal_pixbuf = app_theme.get_pixbuf("mini/pause_normal.png")
        play_hover_pixbuf = app_theme.get_pixbuf("mini/play_hover.png")
        pause_hover_pixbuf = app_theme.get_pixbuf("mini/pause_hover.png")
        play_press_pixbuf = app_theme.get_pixbuf("mini/play_press.png")
        pause_press_pixbuf = app_theme.get_pixbuf("mini/pause_press.png")
        playpause_button = ToggleButton(play_normal_pixbuf, pause_normal_pixbuf,
                     play_hover_pixbuf, pause_hover_pixbuf,
                     play_press_pixbuf, pause_press_pixbuf)
        return playpause_button    
    
    def on_player_playpause(self, widget):    
        if Player.song:
            Player.playpause()
            
    def __swap_play_status(self, obj, active):    
        self.playpause_button.handler_block(self.__id_signal_play)
        self.playpause_button.set_active(active)
        self.playpause_button.handler_unblock(self.__id_signal_play)
            
    def on_configure_event(self, widget, event):        
        if widget.get_property("visible"):
            config.set("mini","x","%d" % event.x)
            config.set("mini","y","%d" % event.y)
            
    def on_enter_notify_event(self, widget, event):        
        if self.drag_move: return
        
        childs = self.body_box.get_children()
        if len(childs) > 0:
            child = childs[0]
            if child != self.control_box:
                self.draw_animation = True
                container_remove_all(self.body_box)
                self.active_draw_func = self.draw_info
                self.target_draw_func = self.draw_control
                self.start_animation(self.control_box)
        else:
            self.draw_animation = True
            container_remove_all(self.body_box)
            self.active_draw_func = self.draw_info
            self.target_draw_func = self.draw_control
            self.start_animation(self.control_box)
        
    def on_leave_notify_event(self, widget, event):    
        if self.drag_move: return
        
        if not self.is_in_window():
            self.draw_animation = True
            container_remove_all(self.body_box)
            self.active_draw_func = self.draw_control
            self.target_draw_func = self.draw_info
            self.start_animation(self.info_box)
            
    def on_button_press_event(self, widget, event):        
        if event.button == 1:
            self.old_x, self.old_y = widget.get_position()
            self.mouse_x, self.mouse_y = event.x_root, event.y_root
            self.drag_move = True
            
    def on_motion_notify_event(self, widget, event):        
        if self.drag_move:
            x = int(self.old_x + (event.x_root - self.mouse_x))
            y = int(self.old_y + (event.y_root - self.mouse_y))
            widget.move(*self.adjust_move_coordinate(widget, x, y))
            
    def on_button_release_event(self, widget, event):        
        self.drag_move = False
            
    def toggle_visible(self, bring_to_front=False):        
        if self.get_property("visible"):
            if self.is_active():
                if not bring_to_front:
                    self.hide_all()
            else:    
                self.present()
        else:        
            self.show_from_tray()
            
    def toggle_window(self):        
        if self.get_property("visible"):
            self.hide_to_tray()
        else:    
            self.show_from_tray()

    def show_from_tray(self):        
        if config.get("mini", "x") != "-1":
            self.move(int(config.get("mini", "x")), int(config.get("mini", "y")))
        self.show_all()    
        
    def hide_to_tray(self):    
        self.hide_all()
        
    def get_widget_pixbuf(self, widget=None):    
        if widget is None:
            widget = self
        drawable = widget.window
        x, y, width, height = widget.allocation
        pixbuf = gtk.gdk.Pixbuf(gtk.gdk.COLORSPACE_RGB, False, 8, width, height)
        pixbuf.get_from_drawable(drawable, drawable.get_colormap(), x, y, 0, 0, width, height)
        return pixbuf
    
    def shape_mini_frame(self, widget, event):    
        pass
    
    def draw_info(self, cr, allocation, alpha):
        rect = gtk.gdk.Rectangle(allocation.x, allocation.y, allocation.width, allocation.height)
        cr.push_group()
        
        # Draw logo.
        rect.x += self.logo_padding_left
        logo_pixbuf = app_theme.get_pixbuf("mini/logo.png").get_pixbuf()
        icon_y = rect.y + (rect.height - logo_pixbuf.get_height()) / 2
        draw_pixbuf(cr, logo_pixbuf, rect.x, icon_y)
        
        # Draw playinfo.
        left_width = logo_pixbuf.get_width() + self.logo_padding_right
        rect.x += left_width
        rect.width -= left_width * 2
        self.playinfo.draw_content(cr, rect)
        
        # set source to paint with alpha.
        cr.pop_group_to_source()
        cr.paint_with_alpha(alpha)
    
    def draw_control(self, cr, allocation, alpha):
        rect = gtk.gdk.Rectangle(allocation.x, allocation.y, allocation.width, allocation.height)
        cr.push_group()
        
        # Draw lyrics button
        rect.x += self.lyrics_padding_left
        enable_lyrics = config.getboolean('lyrics', 'status')
        if enable_lyrics:
            lyrics_pixbuf = app_theme.get_pixbuf('mini/lyrics_active_normal.png').get_pixbuf()
        else:
            lyrics_pixbuf = app_theme.get_pixbuf('mini/lyrics_inactive_normal.png').get_pixbuf()
        icon_y = rect.y + (rect.height - lyrics_pixbuf.get_height()) / 2
        draw_pixbuf(cr, lyrics_pixbuf, rect.x, icon_y)
        
        # Draw previous button.
        rect.x += lyrics_pixbuf.get_width() + self.lyrics_padding_right
        previous_pixbuf = app_theme.get_pixbuf('mini/previous_normal.png').get_pixbuf()
        icon_y = rect.y + (rect.height - previous_pixbuf.get_height()) / 2
        draw_pixbuf(cr, previous_pixbuf, rect.x, icon_y)
        
        # Draw playpause button.
        rect.x += previous_pixbuf.get_width()
        is_played = config.getboolean('player', 'play')
        if is_played:
            playpause_pixbuf = app_theme.get_pixbuf('mini/pause_normal.png').get_pixbuf()
        else:
            playpause_pixbuf = app_theme.get_pixbuf('mini/play_normal.png').get_pixbuf()
        icon_y = rect.y + (rect.height - playpause_pixbuf.get_height()) / 2
        draw_pixbuf(cr, playpause_pixbuf, rect.x, icon_y)
        
        # Draw next button.
        rect.x += playpause_pixbuf.get_width()
        next_pixbuf = app_theme.get_pixbuf('mini/next_normal.png').get_pixbuf()
        icon_y = rect.y + (rect.height - next_pixbuf.get_height()) / 2
        draw_pixbuf(cr, next_pixbuf, rect.x, icon_y)
        
        # Draw volume button.
        (v_w, v_h,) = self.volume_slider.volume_button.get_size()
        v_y = rect.y + (rect.height - v_h) / 2
        rect.x += self.volume_padding_left + next_pixbuf.get_width()
        volume_button_rect = gtk.gdk.Rectangle(rect.x, v_y, v_w, v_h)
        self.volume_slider.volume_button.draw_volume(cr, volume_button_rect)
        
        # Draw event box, draw close button.
        close_pixbuf = ui_theme.get_pixbuf('button/window_close_normal.png').get_pixbuf()
        event_box_x = rect.width - close_pixbuf.get_width() + 4
        draw_pixbuf(cr, close_pixbuf, event_box_x, rect.y)
        
        # Draw min button.
        min_pixbuf = ui_theme.get_pixbuf('button/window_min_normal.png').get_pixbuf()
        event_box_x -= min_pixbuf.get_width()
        draw_pixbuf(cr, min_pixbuf, event_box_x, rect.y)
        
        # draw quell button.
        quell_pixbuf = app_theme.get_pixbuf('mode/quell_normal.png').get_pixbuf()
        event_box_x -= quell_pixbuf.get_width()
        draw_pixbuf(cr, quell_pixbuf, event_box_x, rect.y)
        
        cr.pop_group_to_source()
        cr.paint_with_alpha(alpha)
        
    def expose_mini_frame(self, widget, event):
        cr  = widget.window.cairo_create()
        rect = widget.allocation
        cr.set_source_rgba(1,1,1, 0.8)
        cr.rectangle(rect.x, rect.y, rect.width, rect.height)
        cr.fill()
        
        if self.draw_animation:
            if self.active_draw_func:
                self.active_draw_func(cr, rect, self.active_alpha)
            if self.target_draw_func:    
                self.target_draw_func(cr, rect, self.target_alpha)
        
    def start_animation(self, widget):    
        if not self.in_animation:
            self.in_animation = False
            try:
                self.timeline.stop()
            except:    
                pass
            self.timeline = Timeline(self.animation_time, CURVE_SINE)
            self.timeline.connect("update", self.update_animation)
            self.timeline.connect("completed", lambda source: self.completed_animation(source, widget))
            self.timeline.run()
    
    def update_animation(self, source, status):
        self.active_alpha = 1.0 - status
        self.target_alpha = status
        self.queue_draw()
    
    def completed_animation(self, source, widget):    
        self.draw_animation = False        
        self.active_alpha = 1.0
        self.target_alpha = 0.0
        self.in_animation = False
        self.queue_draw()
        switch_tab(self.body_box, widget)
        
    def sync_volume(self):    
        self.volume_slider.load_volume_config()
