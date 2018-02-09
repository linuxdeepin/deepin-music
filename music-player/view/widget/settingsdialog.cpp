/*
 * Copyright (C) 2017 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <DSettings>

void GenerateSettingTranslate()
{
    auto base_close_quit_action_text0 = QObject::tr("Minimize to system tray");
    auto base_close_quit_action_text1 = QObject::tr("Exit Deepin Music");
    auto base_play_auto_play_text = QObject::tr("Autoplay");
    auto base_play_fade_in_out_text = QObject::tr("Enable fade");
    auto base_play_remember_progress_text = QObject::tr("Remember playback position");
    auto group_base_close_name = QObject::tr("Close Main Window");
    auto group_base_name = QObject::tr("Basic");
    auto group_base_play_name = QObject::tr("Play");
    auto group_shortcuts_name = QObject::tr("Shortcuts");
    auto reset_button_name = QObject::tr("Restore Defaults");
    auto shortcuts_all_next_name = QObject::tr("Next");
    auto shortcuts_all_play_pause_name = QObject::tr("Play/Pause");
    auto shortcuts_all_previous_name = QObject::tr("Previous");
    auto shortcuts_all_volume_down_name = QObject::tr("Volume down");
    auto shortcuts_all_volume_up_name = QObject::tr("Volume up");
}
