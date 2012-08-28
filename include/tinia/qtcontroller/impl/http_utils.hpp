#pragma once
#include <QString>
#include <QMap>
namespace tinia { namespace qtcontroller { namespace impl {

QString getRequestURI(const QString& request);

bool isGet(const QString& request);

QString getMimeType(const QString& file);

QMap<QString, QString> decodeGetParameters(const QString& request);

}}}
