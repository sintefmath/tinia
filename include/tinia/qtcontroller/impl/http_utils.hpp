#pragma once
#include <QString>

namespace tinia { namespace qtcontroller { namespace impl {
QString getRequestURI(const QString& request);
bool isGet(const QString& request);
}}}
