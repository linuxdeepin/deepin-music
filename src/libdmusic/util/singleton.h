// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <memory>

namespace DMusic {

template <class T>
class DSingleton
{
public:
    static inline T *getInstance()
    {
        static T instance;
        return &instance;
    }

protected:
    DSingleton(void) {}
    ~DSingleton(void) {}
    DSingleton(const DSingleton &) = delete;
    DSingleton &operator= (const DSingleton &) = delete;
};

}
