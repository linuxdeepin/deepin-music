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

import gst
import gtk
from collections import OrderedDict

from dtk.ui.scalebar import VScalebar
from dtk.ui.dialog import DialogBox, DIALOG_MASK_SINGLE_PAGE
from dtk.ui.box import ImageBox
from dtk.ui.button import Button
from dtk.ui.menu import Menu
from dtk.ui.combo import ComboBox
from dtk.ui.utils import get_widget_root_coordinate

from widget.skin import app_theme
from config import config
from player import Player
from nls import _

class PreampScalebar(gtk.VBox):
    
    def __init__(self):
        super(PreampScalebar, self).__init__()
        increase_one = ImageBox(app_theme.get_pixbuf("equalizer/1.png"))
        increase_two = ImageBox(app_theme.get_pixbuf("equalizer/2.png"))
        increase_db = ImageBox(app_theme.get_pixbuf("equalizer/DB.png"))
        zero_zero = ImageBox(app_theme.get_pixbuf("equalizer/0.png"))
        zero_db = ImageBox(app_theme.get_pixbuf("equalizer/DB.png"))
        preamp_image = ImageBox(app_theme.get_pixbuf("equalizer/preamp.png"))
        scale_image = ImageBox(app_theme.get_pixbuf("equalizer/scale.png"))
        increase_image = ImageBox(app_theme.get_pixbuf("equalizer/+.png"))
        decrease_image = ImageBox(app_theme.get_pixbuf("equalizer/-.png"))
        one_image = ImageBox(app_theme.get_pixbuf("equalizer/1.png"))
        two_image = ImageBox(app_theme.get_pixbuf("equalizer/2.png"))
        db_image = ImageBox(app_theme.get_pixbuf("equalizer/DB.png"))
        blank_image = ImageBox(app_theme.get_pixbuf("equalizer/blank.png"))
        blank_image1 = ImageBox(app_theme.get_pixbuf("equalizer/blank.png"))
        
        self.scalebar = VScalebar(
            app_theme.get_pixbuf("vscalebar/upper_fg.png"),
            app_theme.get_pixbuf("vscalebar/upper_bg.png"),
            app_theme.get_pixbuf("vscalebar/middle_fg.png"),
            app_theme.get_pixbuf("vscalebar/middle_bg.png"),
            app_theme.get_pixbuf("vscalebar/bottom_fg.png"),
            app_theme.get_pixbuf("vscalebar/bottom_bg.png"),
            app_theme.get_pixbuf("vscalebar/point.png"),
            )
        self.scalebar.set_value(50)
        preamp_align = gtk.Alignment()
        preamp_align.set_padding(8, 8, 0, 0)
        preamp_align.add(scale_image)
        preamp_hbox = gtk.HBox()
        preamp_hbox.pack_start(self.scalebar, False, False)
        preamp_hbox.pack_start(preamp_align, False, False)
        
        increase_box = gtk.HBox(spacing=1)
        increase_box.pack_start(increase_image, False, False)
        increase_box.pack_start(increase_one, False, False)
        increase_box.pack_start(increase_two, False, False)
        increase_box.pack_start(increase_db, False, False)
        increase_align = gtk.Alignment()
        increase_align.set_padding(6, 0, 0, 0)
        increase_align.set(1.0, 0, 0, 0)
        increase_align.add(increase_box)
        
        zero_box = gtk.HBox(spacing=1)
        zero_box.pack_start(blank_image, False, False)
        zero_box.pack_start(blank_image1, False, False)
        zero_box.pack_start(zero_zero, False, False)
        zero_box.pack_start(zero_db, False, False)
        zero_align = gtk.Alignment()
        zero_align.set(0.5, 0.5, 0, 0)
        zero_align.add(zero_box)
        
        decrease_box = gtk.HBox(spacing=1)
        decrease_box.pack_start(decrease_image, False, False)
        decrease_box.pack_start(one_image, False, False)
        decrease_box.pack_start(two_image, False, False)
        decrease_box.pack_start(db_image, False, False)
        decrease_align = gtk.Alignment()
        decrease_align.set(0, 1, 0, 0)
        decrease_align.set_padding(0, 6, 0, 0)
        decrease_align.add(decrease_box)
        
        right_box = gtk.VBox()
        right_box.pack_start(increase_align)
        right_box.pack_start(zero_align)
        right_box.pack_start(decrease_align)
        upper_box = gtk.HBox(spacing=3)        
        upper_box.pack_start(preamp_hbox)
        upper_box.pack_start(right_box)
        bottom_align = gtk.Alignment()
        bottom_align.set(0, 0, 1.0, 0)
        bottom_align.add(preamp_image)
        
        self.pack_start(upper_box, False, False)
        self.pack_start(bottom_align, False,False)


class SlipperScalebar(gtk.VBox):
    def __init__(self, num_string="29"):
        gtk.VBox.__init__(self)
        scale_image = ImageBox(app_theme.get_pixbuf("equalizer/scale.png"))
        self.scalebar = VScalebar(
            app_theme.get_pixbuf("vscalebar/upper_fg.png"),
            app_theme.get_pixbuf("vscalebar/upper_bg.png"),
            app_theme.get_pixbuf("vscalebar/middle_fg.png"),
            app_theme.get_pixbuf("vscalebar/middle_bg.png"),
            app_theme.get_pixbuf("vscalebar/bottom_fg.png"),
            app_theme.get_pixbuf("vscalebar/bottom_bg.png"),
            app_theme.get_pixbuf("vscalebar/point.png"),
            )
        self.scalebar.set_value(50)
        self.scalebar.set_has_point(False)
        preamp_align = gtk.Alignment()
        preamp_align.set_padding(8, 8, 0, 0)
        preamp_align.add(scale_image)
        
        hear_box = gtk.HBox()
        hear_box.pack_start(self.scalebar, False, False)
        hear_box.pack_start(preamp_align, False, False)
        
        num_box = gtk.HBox(spacing=1)
        for i in num_string:
            num_box.pack_start(ImageBox(app_theme.get_pixbuf("equalizer/%s.png" % i)))
        num_align = gtk.Alignment()    
        num_align.set(0, 0, 0.8, 0.2)
        num_align.add(num_box)
        self.pack_start(hear_box, False, False)
        self.pack_start(num_align, False, False)
        
MANDATORY = OrderedDict()
MANDATORY_CUSTOM = "Custom"

MANDATORY[MANDATORY_CUSTOM] = "1.0:0.0:0.0:0.0:0.0:0.0:0.0:0.0:0.0:0.0"
MANDATORY["Default"] = "0:0:0:0:0:0:0:0:0:0"
MANDATORY["Classical"] = "0.0:0.0:0.0:0.0:0.0:0.0:-7.2:-7.2:-7.2:-9.6"
MANDATORY["Club"] = "0.0:0.0:8:5.6:5.6:5.6:3.2:0.0:0.0:0.0"
MANDATORY["Dance"] = "9.6:7.2:2.4:0.0:0.0:-5.6:-7.2:-7.2:0.0:0.0"
MANDATORY["Full Bass"] = "-8:9.6:9.6:5.6:1.6:-4:-8:-10.4:-11.2:-11.2"
MANDATORY["Full Bass and Treble"] = "7.2:5.6:0.0:-7.2:-4.8:1.6:8:11.2:12:12"
MANDATORY["Full Treble"] = "-9.6:-9.6:-9.6:-4:2.4:11.2:11.3:11.8:12:12"
MANDATORY["Laptop Speakers/Headphones"] = "4.8:11.2:5.6:-3.2:-2.4:1.6:4.8:9.6:12:11.8"
MANDATORY["Large Hall"] = "10.4:10.4:5.6:5.6:0.0:-4.8:-4.8:-4.8:0.0:0.0"
MANDATORY["Live"] = "-4.8:0.0:4:5.6:5.6:5.6:4:2.4:2.4:2.4"
MANDATORY["Party"] = "7.2:7.2:0.0:0.0:0.0:0.0:0.0:0.0:7.2:7.2"
MANDATORY["Pop"] = "-1.6:4.8:7.2:8:5.6:0.0:-2.4:-2.4:-1.6:-1.6"
MANDATORY["Reggae"] = "0.0:0.0:0.0:-5.6:0.0:6.4:6.4:0.0:0.0:0.0"
MANDATORY["Rock"] = "8:4.8:-5.6:-8:-3.2:4:8.8:11.2:11.2:11.2"
MANDATORY["Ska"] = "-2.4:-4.8:-4:0.0:4:5.6:8.8:9.6:11.2:9.6"
MANDATORY["Soft"] = "4.8:1.6:0.0:-2.4:0.0:4:8:9.6:11.2:12"
MANDATORY["Soft Rock"] = "4:4:2.4:0.0:-4:-5.6:-3.2:0.0:2.4:8.8"
MANDATORY["Techno"] = "8:5.6:0.0:-5.6:-4.8:0.0:8:9.6:9.6:8.8"

mandatory_i18n = OrderedDict()
mandatory_i18n["Custom"]  = _("Custom")
mandatory_i18n["Default"] = _("Default")
mandatory_i18n["Classical"] = _("Classical")
mandatory_i18n["Club"] = _("Club")
mandatory_i18n["Dance"] = _("Dance")
mandatory_i18n["Full Bass"] = _("Full Bass")
mandatory_i18n["Full Bass and Treble"] = _("Full Bass and Treble")
mandatory_i18n["Full Treble"] = _("Full Treble")
mandatory_i18n["Laptop Speakers/Headphones"] = _("Headphones")
mandatory_i18n["Large Hall"] = _("Large Hall")
mandatory_i18n["Live"] = _("Live")
mandatory_i18n["Party"] = _("Party")
mandatory_i18n["Pop"] = _("Pop")
# mandatory_i18n["Reggae"] = "Reggae"
mandatory_i18n["Rock"] = _("Rock")
mandatory_i18n["Ska"] = _("Ska")
mandatory_i18n["Soft"] = _("Soft")
mandatory_i18n["Soft Rock"] = _("Soft Rock")
mandatory_i18n["Techno"] = _("Electronic")

class EqualizerWindow(DialogBox):        
    def __init__(self):
        super(EqualizerWindow, self).__init__(
            _("Equalizer"), 372, 168, DIALOG_MASK_SINGLE_PAGE, close_callback=self.hide_all, 
            modal=False, window_hint=None, skip_taskbar_hint=False)
        
        self.manual_flag = False
        
        try:
            pre_value = float(config.get("equalizer", "preamp"))
        except:    
            pre_value = 0.6
            
        pre_adjust = gtk.Adjustment(value=pre_value, lower= -10 , upper=10, step_incr=0.1, page_incr=1, page_size=0)
        self.preamp_scale = PreampScalebar()
        self.preamp_scale.scalebar.set_adjustment(pre_adjust)
        pre_adjust.connect("value-changed", self.preamp_change)
        
        self.connect("configure-event", self.equalizer_configure_event)
        control_box = gtk.HBox(spacing=10)
        control_box.pack_start(self.preamp_scale, False, False)
        self.__scales = {}
        
        for i, label in enumerate(["32", "64", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"]):
            slipper_scale = SlipperScalebar(label)
            try:
                value = float(config.get("equalizer", "equalizer-band%s" % str(i)))
            except:    
                value = 0.0
                
            adjust = gtk.Adjustment(value=value, lower=-12, upper=12, step_incr=0.1, page_incr=1, page_size=0)
            adjust.changed_id = adjust.connect("value-changed", self.__on_adjust_change, i)
            slipper_scale.scalebar.set_adjustment(adjust)
            self.__scales[i] = slipper_scale.scalebar
            control_box.pack_start(slipper_scale, False, False)
        
        try:
            self.__equalizer = gst.element_factory_make("equalizer-10bands")
        except gst.PluginNotFoundError:    
            self.__equalizer = None
            self.logerror("Equalizer support requires gstreamer-plugins-bad (>= 0.10.5)")
        else:    
            Player.bin.xfade_add_filter(self.__equalizer)
            for i in range(0, 10):
                try:
                    value = float(config.get("equalizer", "equalizer-band%s"  % str(i)))
                except:    
                    value = 0

                self.__equalizer.set_property("band" + str(i), float(value))    
                
            Player.bin.connect("tee-removed", self.__on_remove)    
            config.connect("config-changed", self.__on_config_change)
            
        self.active_button = Button(_("Close"))
        self.active_button.connect("clicked", self.hide_win)
        self.reset_button = Button(_("Reset"))
        self.reset_button.connect("clicked", self.set_default_value)
        self.predefine_button = Button(_("Predefined"))
        self.predefine_button.connect("clicked", self.show_predefine)
        
        self.predefine_combo_box = ComboBox([(value, key) for key, value in mandatory_i18n.items()],
                                            250, select_index=self.get_selected_index())
        self.predefine_combo_box.connect("item-selected", self.set_predefine_value)
            
        control_box_align = gtk.Alignment()
        control_box_align.set(0.0, 0.0, 1.0, 1.0)
        control_box_align.set_padding(10, 20, 15, 15)
        control_box_align.add(control_box)
        
        main_align = gtk.Alignment()
        main_align.set(0.0, 0.0, 1.0, 1.0)
        main_align.set_padding(0, 5, 2, 2)
        main_box = gtk.VBox(spacing=5)
        main_align.add(main_box)
        
        main_box.add(control_box_align)
        
        self.body_box.pack_start(main_align, True, True)
        self.left_button_box.set_buttons([self.predefine_combo_box])
        self.right_button_box.set_buttons([self.reset_button, self.active_button])
        
    def set_default_value(self, widget):    
        self.predefine_combo_box.set_select_index(1)
        self.__change("Default")
        self.preamp_scale.scalebar.set_value(0.6)
        
    def db_to_percent(self, dB):    
        return 10 ** (dB / 10)
    
    def preamp_change(self, adjust):
        
        config.set("equalizer", "preamp", str(adjust.get_value()))
        Player.volume = self.db_to_percent(adjust.get_value())
        
    def __on_remove(self, bin, tee, element):    
        if element != self.__equalizer:
            return
        self.__equalizer.set_state(gst.STATE_NULL)
        self.__equalizer = None
        
    def __on_config_change(self, dispacher, section, option, value):    
        if section == "equalizer" and option.find("equalizer-band") == 0:
            band_name = option.replace("equalizer-", "")
            self.__equalizer.set_property(band_name, float(value))
            
            
    def equalizer_configure_event(self, widget, event):        
        if widget.get_property("visible"):
            if widget.get_resizable():
                config.set("equalizer","width","%d"%event.width)
                config.set("equalizer","height","%d"%event.height)
            config.set("equalizer","x","%d"%event.x)
            config.set("equalizer","y","%d"%event.y)
        
    def hide_win(self, widget):    
        self.hide_all()
        
    def __select_name(self):
        self.menu_dict = OrderedDict()
        for  name in MANDATORY.keys():
            self.menu_dict[name] = [None, name, self.__change, name]
        
        values = []
        for i in range(0, 10):
            try:
                value = int(float(config.get("equalizer", "equalizer-band%s" % str(i))))
            except: value = 0    
            values.append(str(value))
        values = ":".join(values)    
        
        self.has_tick = False
        for name, value in MANDATORY.iteritems():
            value = value.split(":")            
            value = ":".join([str(int(float(v))) for v in value])

            if value == values:
                self.menu_dict[name][0] = (app_theme.get_pixbuf("menu/tick.png"),
                                           app_theme.get_pixbuf("menu/tick_press.png"))
                self.has_tick = True
        if not self.has_tick:        
            self.menu_dict[MANDATORY_CUSTOM][0] = (app_theme.get_pixbuf("menu/tick.png"),
                                                   app_theme.get_pixbuf("menu/tick_press.png"))
    def get_selected_index(self):        
        values = []
        for i in range(0, 10):
            try:
                value = int(float(config.get("equalizer", "equalizer-band%s" % str(i))))
            except: value = 0    
            values.append(str(value))
        values = ":".join(values)    
        
        for index, value in enumerate(MANDATORY.values()):
            value = value.split(":")            
            value = ":".join([str(int(float(v))) for v in value])

            if value == values:
                return index
        return 0    
    
    def set_predefine_value(self, widget, label, allocated_data, index):
        self.__change(allocated_data)
        
    def show_predefine(self, widget):    
        self.__select_name()
        menu_items = self.menu_dict.values()
        menu_items.insert(2, None)
        Menu(menu_items, True).show(get_widget_root_coordinate(widget))
    
    def __on_adjust_change(self, adjust, i):    
        if not self.manual_flag:
            self.predefine_combo_box.set_select_index(0)
        config.set("equalizer", "equalizer-band%s" % str(i), str(adjust.get_value()))
        self.manual_flag = False        
        
    def __save(self, *args):    
        text = self.predefine_button.get_label()
        
        if text in MANDATORY.keys():
            return
        values = []
        for i in range(0, 10):
            try:
                value = float(config.get("equalizer", "equalizer-band%s" % str(i)))
            except:    
                value = 0.0
            values.append(str(value))    
            
    def __change(self, name):        
        if name == MANDATORY_CUSTOM:
            self.predefine_combo_box.set_select_index(0)
            return True
        
        self.manual_flag = True
        if name in MANDATORY.keys():
            values = MANDATORY[name].split(":")

            for i, value in enumerate(values):    
                adj = self.__scales[i].get_adjustment()
                adj.handler_block(adj.changed_id)
                self.__scales[i].set_value(float(value))
                config.set("equalizer", "equalizer-band%s" % str(i), str(float(value)))
                adj.handler_unblock(adj.changed_id)
        return True        

    def run(self):
        if config.get("equalizer", "x") == "-1":
            self.set_position(gtk.WIN_POS_CENTER)
        else:    
            self.move(int(config.get("equalizer","x")),int(config.get("equalizer","y")))
        self.show_all()
