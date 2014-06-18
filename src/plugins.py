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

import imp
import inspect
import os
import shutil
import sys
import tarfile
import traceback


import xdg_support
from logger import Logger
from nls import _
from config import config
import utils

class InvalidPluginError(Exception):
    def __str__(self):
        return str(self.args[0])

class PluginsManager(Logger):
    
    def __init__(self, dmusic, load=True):
        
        self.plugindirs = xdg_support.get_plugin_dirs()
        for plugin_dir in self.plugindirs:
            try:
                os.makedirs(plugin_dir)
            except:
                pass

        self.plugindirs = [ x for x in self.plugindirs if os.path.exists(x) ]
        self.loaded_plugins = {}

        self.dmusic = dmusic
        self.enabled_plugins = {}

        self.load = load

        if self.load:
            self.load_enabled()

    def __find_plugin(self, plugin_name):
        if not plugin_name:
            return None
        for plugin_dir in self.plugindirs:
            path = os.path.join(plugin_dir, plugin_name)
            if os.path.exists(path):
                return path
        return None

    def load_plugin(self, plugin_name, reload=False):
        if not reload and plugin_name in self.loaded_plugins:
            return self.loaded_plugins[plugin_name]

        path = self.__find_plugin(plugin_name)
        if path is None:
            return False
        sys.path.insert(0, path)
        plugin = imp.load_source(plugin_name, os.path.join(path,'__init__.py'))
        sys.path = sys.path[1:]
        self.loaded_plugins[plugin_name] = plugin
        return plugin

    def install_plugin(self, path):
        try:
            tar = tarfile.open(path, "r:*") #transparently supports gz, bz2
        except (tarfile.ReadError, OSError):
            raise InvalidPluginError(
                _('Plugin archive is not in the correct format.'))

        #ensure the paths in the archive are sane
        mems = tar.getmembers()
        base = os.path.basename(path).split('.')[0]
        if os.path.isdir(os.path.join(self.plugindirs[0], base)):
            raise InvalidPluginError(
                _('A plugin with the name \"%s\" is already installed.') % base)

        for m in mems:
            if not m.name.startswith(base):
                raise InvalidPluginError(
                    _('Plugin archive contains an unsafe path.'))

        tar.extractall(self.plugindirs[0])

    def uninstall_plugin(self, plugin_name):
        self.disable_plugin(plugin_name)
        for plugin_dir in self.plugindirs:
            try:
                shutil.rmtree(self.__find_plugin(plugin_name))
                return True
            except:
                pass
        return False

    def enable_plugin(self, plugin_name):
        try:
            plugin = self.load_plugin(plugin_name)
            if not plugin: raise Exception("Error loading plugin")
            plugin.enable(self.dmusic)
            self.enabled_plugins[plugin_name] = plugin
            self.logdebug("Loaded plugin %s", plugin_name)
            self.save_enabled()
        except Exception, e:
            traceback.print_exc()
            self.logwarn("Unable to enable plugin %s", plugin_name)
            raise e

    def disable_plugin(self, plugin_name):
        try:
            plugin = self.enabled_plugins[plugin_name]
            del self.enabled_plugins[plugin_name]
            plugin.disable(self.dmusic)
            self.logdebug("Unloaded plugin %s", plugin_name)
            self.save_enabled()
        except Exception, e:
            traceback.print_exc()
            self.logwarn("Unable to fully disable plugin %s", plugin_name)
            raise e

    def list_installed_plugins(self):
        pluginlist = []
        for plugin_dir in self.plugindirs:
            if os.path.exists(plugin_dir):
                for plugin_file in os.listdir(plugin_dir):
                    if plugin_file not in pluginlist and \
                            os.path.isdir(os.path.join(plugin_dir, plugin_file)):
                        pluginlist.append(plugin_file)
        return pluginlist

    def list_available_plugins(self):
        pass

    def list_updateable_plugins(self):
        pass

    def get_plugin_info(self, pluginname):
        path = os.path.join(self.__find_plugin(pluginname), 'PLUGININFO')
        infodict = utils.parser_plugin_infos(path)
        return infodict

    def get_plugin_default_preferences(self, plugin_name):
        """
            Returns the default preferences for a plugin
        """
        preflist = {}
        path = self.__find_plugin(plugin_name)
        plugin = imp.load_source(plugin_name, os.path.join(path,'__init__.py'))
        try:
            preferences_pane = plugin.get_preferences_pane()
            for c in dir(preferences_pane):
                attr = getattr(preferences_pane, c)
                if inspect.isclass(attr):
                    try:
                        preflist[attr.name] = attr.default
                    except AttributeError:
                        pass
        except AttributeError:
            pass
        return preflist

    def save_enabled(self):
        if self.load:
            config.set("plugins", "enabled", ",".join(self.enabled_plugins.keys()))

    def load_enabled(self):
        to_enable = config.get("plugins", "enabled", "mpris2").replace("\n", ",").split(",")
        if to_enable:
            for plugin in to_enable:
                try:
                    if plugin:
                        self.enable_plugin(plugin)
                except:
                    pass
        self.load = True        
