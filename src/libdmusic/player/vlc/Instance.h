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

#ifndef VLCQT_VLCINSTANCE_H_
#define VLCQT_VLCINSTANCE_H_

#include <QtCore/QObject>

#include "Enums.h"
//#include "SharedExportCore.h"

//class VlcModuleDescription;

struct libvlc_instance_t;


class VLCQT_CORE_EXPORT VlcInstance : public QObject
{
    Q_OBJECT
public:
    /*!
        \brief VlcInstance constructor.

        \param args libvlc arguments (QStringList)
        \param parent Instance's parent object (QObject *)
    */
    explicit VlcInstance(const QStringList &args,
                         QObject *parent = NULL);

    /*!
        VlcInstance destructor
    */
    ~VlcInstance();

    /*!
        \brief Returns libvlc instance object.
        \return libvlc instance (libvlc_instance_t *)
    */
    libvlc_instance_t *core();

    /*!
        \brief Returns libvlc initialisation status.
        \return libvlc status (bool)
    */
//    bool status() const;

    /*!
        \brief Returns current log level (default Vlc::ErrorLevel)
        \return log level
    */
    Vlc::LogLevel logLevel() const;

    /*!
        \brief Set current log level
        \param level desired log level
        \see Vlc::LogLevel
    */
    //void setLogLevel(Vlc::LogLevel level);

//    static QString libVersion();


//    static int libVersionMajor();


    static int libVersionMinor();

    /*!
        \brief libvlc version info
        \return a string containing the libvlc version (QString)
    */
    static QString version();

    /*!
        \brief libvlc compiler info
        \return a string containing the compiler version (QString)
    */
    //static QString compiler();

    /*!
        \brief libvlc changeset info
        \return a string containing the changeset (QString)
    */
    //static QString changeset();

    /*!
        \brief Sets the application name.

        libvlc passes this as the user agent string when a protocol requires it.

        \param application Application name (QString)
        \param version Application version (QString)

        \see setAppId
    */
//    void setUserAgent(const QString &application,
//                      const QString &version);

    /*!
        \brief Sets the application some meta-information.

        \param id Java-style application identifier, e.g. "com.acme.foobar"
        \param version application version numbers, e.g. "1.2.3"
        \param icon application icon name, e.g. "foobar"

        \see setUserAgent
    */
//    void setAppId(const QString &id,
//                  const QString &version,
//                  const QString &icon);

    /*!
        \brief List audio filter modules
        \return audio filter module description list
    */
//    QList<VlcModuleDescription *> audioFilterList() const;

    /*!
        \brief List video filter modules
        \return video filter module description list
    */
//    QList<VlcModuleDescription *> videoFilterList() const;
    /**
     * @brief catchPulseError  catch the exception of pulseaudio
     * @param err  error level
     */
    void catchPulseError(int err);
signals:
    void sendErrorOccour(int err);
private:
    libvlc_instance_t *_vlcInstance;
    bool _status;
    Vlc::LogLevel _logLevel;
};

#endif // VLCQT_VLCINSTANCE_H_
