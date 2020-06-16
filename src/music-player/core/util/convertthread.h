/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
*
* Author:     huangjie<huangjie@uniontech.com>
* Maintainer: huangjie <huangjie@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef CONVERTTHREAD_H
#define CONVERTTHREAD_H

#include <QThread>
#include <QMutex>

class convertThread : public QThread
{
    Q_OBJECT
    struct musicInfo {
        QString mpath;
        QString mhash;
        musicInfo(QString path, QString hash)
            : mpath(path)
            , mhash(hash)
        {
        }
    };

public:
    convertThread();
    /**
     * @brief addApeandAmr 添加ape缓存任务
     * @param path  ape路径
     * @param hash  ape哈西音乐名称
     */
    void addApeandAmr(const QString &path, const QString &hash);
    ~convertThread() override;

protected:
    void run() override;
signals:

public slots:
    /**
     * @brief exitToCheckFile 停止标志,且删除正在写的文件
     */
    void exitToCheckFile();

private:
    QList<musicInfo> m_infos;
    bool m_exit;
    QMutex mutex;
};

#endif // CONVERTTHREAD_H
