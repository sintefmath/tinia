#include "tinia/qtcontroller/moc/ExposedModelReceiver.hpp"
#include "tinia/qtcontroller/impl/synchronous_http_get.hpp"

namespace tinia {
namespace qtcontroller {

ExposedModelReceiver::ExposedModelReceiver(QString baseUrl, boost::shared_ptr<tinia::model::ExposedModel> exposedModel,
                                           QObject *parent) :
    QObject(parent),
    m_exposedModelUrl(baseUrl + "/getExposedModelUpdate.xml"),
    m_exposedModel(exposedModel),
    m_networkAccessManager(this),
    m_xmlHandler(exposedModel)
{
    connect(&m_networkAccessManager, SIGNAL(finished(QNetworkReply*)),
            SLOT(networkReplyFinished(QNetworkReply*)));

}

void ExposedModelReceiver::startExposedModelReceiving()
{
    // First get we need to do synchronously.
    QNetworkReply* reply = impl::getSynchronousHTTP(createExposedModelUrl());
    networkReplyFinished(reply);
    delete reply;
}

void ExposedModelReceiver::networkReplyFinished(QNetworkReply *reply)
{
    std::string xmlReply = QString(reply->readAll()).toStdString();

    throw std::runtime_error("Can not update schema yet");

    m_xmlHandler.updateState(xmlReply.c_str(), xmlReply.size());

    // Post new request (this time async)
    m_networkAccessManager.get(QNetworkRequest(createExposedModelUrl()));
}

QUrl ExposedModelReceiver::createExposedModelUrl()
{
    return QUrl(m_exposedModelUrl + "?revision="
                + QString::number(m_exposedModel->getRevisionNumber()));
}

} // namespace qtcontroller
} // namespace tinia
