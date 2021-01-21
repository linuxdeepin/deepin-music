#ifndef SPEECHEXPORTBUS_H
#define SPEECHEXPORTBUS_H

#include <QObject>
#include <QDBusContext>
#include <QScopedPointer>
#include <QDBusVariant>

class ExporteDBusInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.speech")
public Q_SLOTS:
    QDBusVariant invoke(QString action, QString parameters);
    QDBusVariant invokeStrlist(QString action, QStringList parameters);
public:
    QHash<QString, QPair<std::function<QVariant(QString)>, QString>> actions;
    QHash<QString, QPair<std::function<QVariant(QStringList)>, QString>> strListactions;
private:
};

class SpeechExportBus : public QObject
{
    Q_OBJECT
public:
    explicit SpeechExportBus(QObject *parent = nullptr);
    ~SpeechExportBus();
    void registerAction(const QString &action, const QString &description, const std::function<QVariant(QString)> handler = nullptr);
    void registerQStringListAction(const QString &action, const QString &description, const std::function<QVariant(QStringList)> handler = nullptr);
    QStringList list();
private:
    ExporteDBusInterface *m_pExportInterface = nullptr;
};

#endif // SPEECHEXPORTBUS_H
