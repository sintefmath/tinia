#pragma once
#include "tinia/qtcontroller/impl/synchronous_http_get.hpp"

#include <QEventLoop>
namespace tinia {
namespace qtcontroller {
namespace impl {

QNetworkReply* getSynchronousHTTP(const QUrl& url)
{
    QNetworkAccessManager networkAccessManager;
    QNetworkReply* reply = networkAccessManager.get(QNetworkRequest(url));

    // Clever bit here:
    QEventLoop eventLoop;

    // Event loop will end when the finished-signal is emited, i.e. when
    // the download is done.
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));

    eventLoop.exec();

    // Make sure it doesn't delete with networkAccessManager:
    reply->setParent(NULL);


    // Return it
    return reply;
}

}
}
}
