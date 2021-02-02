#ifndef FFMPEGDYNAMICINSTANCE_H
#define FFMPEGDYNAMICINSTANCE_H

#include <QObject>
#include <QLibrary>
#include <QMap>

class FfmpegDynamicInstance : public QObject
{
    Q_OBJECT
public:
    static FfmpegDynamicInstance *VlcFunctionInstance();
    QFunctionPointer resolveSymbol(const char *symbol, bool bffmpeg = false);
signals:

public slots:

private:
    explicit FfmpegDynamicInstance(QObject *parent = nullptr);
    ~FfmpegDynamicInstance();
    bool loadVlcLibrary();
    /**
     * @brief libPath get absolutely library path
     * @param strlib library name
     * @return
     */
    QString libPath(const QString &strlib);
    QLibrary libdavcode;
    QLibrary libddformate;

    QMap<QString, QFunctionPointer> m_funMap;
};

#endif // FFMPEGDYNAMICINSTANCE_H
