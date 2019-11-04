#pragma once

#include <QByteArray>

class BaseTool
{
public:
    static QByteArray detectEncode(const QByteArray &data, const QString &fileName);

private:
    BaseTool() {}
};
