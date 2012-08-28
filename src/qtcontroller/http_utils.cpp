#include "tinia/qtcontroller/impl/http_utils.hpp"
#include <QRegExp>
#include <QStringList>
#include <stdexcept>

namespace tinia { namespace qtcontroller { namespace impl {
QString getRequestURI(const QString& request) {
    if(!isGet(request)) {
        throw std::invalid_argument("Request line is not a GET request.");
    }
    return request.split(QRegExp("[ \r\n][ \r\n]*"))[1];
}
bool isGet(const QString& request) {
    return request.split(QRegExp("[ \r\n][ \r\n]*"))[0] == "GET";
}

}}}
