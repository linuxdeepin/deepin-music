/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "searchmetalist.h"

#include <thememanager.h>

class SearchMetaListPrivate
{
public:
    SearchMetaListPrivate(SearchMetaList *parent) : q_ptr(parent) {}

    SearchMetaList *q_ptr;
    Q_DECLARE_PUBLIC(SearchMetaList)
};

SearchMetaList::SearchMetaList(QWidget *parent) : QListWidget(parent), d_ptr(new SearchMetaListPrivate(this))
{
    setObjectName("SearchMetaList");
    setSelectionMode(QListView::SingleSelection);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ThemeManager::instance()->regisetrWidget(this);
}

SearchMetaList::~SearchMetaList()
{

}
