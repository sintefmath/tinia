#pragma once

#include <QObject>
#include <boost/shared_ptr.hpp>
#include <QNetworkReply>

#include "tinia/model/impl/xml/XMLHandler.hpp"
namespace tinia {
namespace model {
    class ExposedModel;
}

namespace qtcontroller {

/**
 * @brief The ExposedModelReceiver class handles receiving of the ExposedModel
 *        from a server
 *
 * The ExposedModelReceiver class handles everything from initial setup of
 * the ExposedModel as well as long-polling further on in the application.
 *
 * @note This class uses asynchronous HTTP requests (except for the first
 *  one to get the ExposedModel), so it is safe to use in the main loop
 */
class ExposedModelReceiver : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief ExposedModelReceiver Constructs the receiver.
     * @note Does *not* start receiving, use startExposedModelReceiving()
     *       to initialize
     * @param baseUrl The base URL to use. E.g. "http://localhost:8080/" for
     *                 the QT mini server, or
     *                 "http://<ip>/trell/job/sessionid/jobid/" for the Trell
     *                 server
     * @param exposedModel a pointer to the ExposedModel to update
     * @param parent Usual QT parent
     */
    explicit ExposedModelReceiver(QString baseUrl,
            boost::shared_ptr<tinia::model::ExposedModel> exposedModel,
            QObject *parent = 0);

    /**
     * @brief startExposedModelReceiving starts the receiving
     */
    void startExposedModelReceiving();
signals:
    
public slots:
    
private slots:
    void networkReplyFinished(QNetworkReply* reply);

private:
    QUrl createExposedModelUrl();
    QString m_exposedModelUrl;
    boost::shared_ptr<tinia::model::ExposedModel> m_exposedModel;
    QNetworkAccessManager m_networkAccessManager;
    tinia::model::impl::xml::XMLHandler m_xmlHandler;
};

} // namespace qtcontroller
} // namespace tinia

