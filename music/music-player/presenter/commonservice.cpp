/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author: Zhang Hao<zhanghao@uniontech.com>
*
* Maintainer: Zhang Hao <zhanghao@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "commonservice.h"
#include <QDebug>
#include "util/pinyinsearch.h"
#include "util/global.h"

CommonService *CommonService::instance = nullptr;
CommonService *CommonService::getInstance()
{
    if (nullptr == instance) {
        instance = new CommonService();
    }
    return instance;
}

void CommonService::setListPageSwitchType(ListPageSwitchType lpst)
{
    listPageSwitchType = lpst;
}

CommonService::CommonService()
{

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

