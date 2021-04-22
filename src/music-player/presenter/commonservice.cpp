/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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

#include "commonservice.h"
#include <QDebug>
#include "util/pinyinsearch.h"
#include "util/global.h"

#include <DGuiApplicationHelper>

bool CommonService::isTabletEnvironment()
{
    return m_isTabletEnvironment;
}

void CommonService::setIsTabletEnvironment(bool isTablet)
{
    m_isTabletEnvironment = isTablet;
}

void CommonService::setListPageSwitchType(ListPageSwitchType lpst)
{
    listPageSwitchType = lpst;
}

CommonService::CommonService()
{
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 5, 0, 0))
    m_isTabletEnvironment = Dtk::Gui::DGuiApplicationHelper::isTabletEnvironment();
#else
    m_isTabletEnvironment = false;
#endif
}

ListPageSwitchType CommonService::getListPageSwitchType() const
{
    return listPageSwitchType;
}

bool CommonService::containsStr(QString searchText, QString text)
{
    //filter
    text = QString(text).remove("\r").remove("\n");
    bool chineseFlag = false;
    for (auto ch : searchText) {
        if (DMusic::PinyinSearch::isChinese(ch)) {
            chineseFlag = true;
            break;
        }
    }
    if (chineseFlag) {
        return text.contains(searchText);
    } else {
        auto curTextList = DMusic::PinyinSearch::simpleChineseSplit(text);
        QString curTextListStr = "";
        if (!curTextList.isEmpty()) {
            for (auto mText : curTextList) {
                if (mText.contains(searchText, Qt::CaseInsensitive)) {
                    return true;
                }
                curTextListStr += mText;
            }
//            curTextListStr = QString(curTextListStr.remove(" "));
            if (curTextListStr.contains(searchText, Qt::CaseInsensitive)) {
                return true;
            }
        }
        return text.contains(searchText, Qt::CaseInsensitive);
    }
}

void CommonService::setSelectModel(TabletSelectMode model)
{
    if (m_select != model) {
        m_select = model;
        emit signalSelectMode(m_select);
    }
}

void CommonService::setIsHScreen(bool state)
{
    m_isHScreen = state;
    emit signalHScreen(m_isHScreen);
}

bool CommonService::isHScreen()
{
    return m_isHScreen;
}

void CommonService::setCurrentWidgetPosY(int posY)
{
    qDebug() << __FUNCTION__ << "--------" << posY;
    m_currentWidgetPosY = posY;
}

int CommonService::getCurrentWidgetPosY()
{
    return m_currentWidgetPosY;
}

