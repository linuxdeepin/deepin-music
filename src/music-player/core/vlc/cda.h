// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CDA_H
#define CDA_H

#include <QThread>
#include "mediameta.h"

struct libvlc_media_player_t;
struct input_item_node_t;

class CdaThread : public QThread
{
    Q_OBJECT

    enum CdromState {
        CDROM_INVALID = -1,
        CDROM_MOUNT_WITHOUT_CD,
        CDROM_MOUNT_WITH_CD,
        CDROM_MOUNT_WITH_OTHERTYPE
    };

public:
    explicit CdaThread(QObject *parent = nullptr);

    void setMediaPlayerPointer(libvlc_media_player_t *play_t) {m_play_t = play_t;}

    /**
     * @brief doQuery 查询cd信息
     * @param query true:查询信息,false:检查是否存在
     */

    QList<MediaMeta> getCdaMetaInfo();

    void closeThread() {m_needRun = 0;}

    int getCdaState() {return m_cdaStat;}
public slots:
    void doQuery();
protected:
    void run() override;
private:
    input_item_node_t *getInputNode();
    QString GetCdRomString();
    void setCdaState(CdromState stat);
signals:
    void sigSendCdaMimeData(const QList<MediaMeta> &);
    void sigSendCdaStatus(int state); //state,0:close ,1:open
public slots:
private:
    libvlc_media_player_t *m_play_t = nullptr;
    int m_needRun = 1;
    CdromState m_cdaStat;
    QList<MediaMeta> m_mediaList;
};

#endif // CDA_H
