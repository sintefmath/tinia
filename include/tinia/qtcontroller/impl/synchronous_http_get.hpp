#pragma once
#include <QNetworkReply>
#include <QUrl>
namespace tinia {
namespace qtcontroller {
namespace impl {

/**
 * @brief getSynchronousHTTP gets synchronously the URL specified
 * @param url the URL to get
 * @return the QNetworkReply generated.
 */
QNetworkReply* getSynchronousHTTP(const QUrl& url);

}
}
}
