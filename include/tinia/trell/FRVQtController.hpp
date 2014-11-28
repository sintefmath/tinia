#pragma once

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QWebSocket);
QT_FORWARD_DECLARE_CLASS(QWebSocketServer);

namespace tinia{ namespace trell {

class FRVGLJobController;


class FRVQtController : public QObject
{
    Q_OBJECT
public:
    //FRVQtController();
    explicit FRVQtController( tinia::trell::FRVGLJobController* glJob, QObject* parent = Q_NULLPTR );
    ~FRVQtController();

Q_SIGNALS:
    void closed();
    
private Q_SLOTS:
     void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();

private:
    QWebSocketServer* m_pWebSocketServer;
    QList<QWebSocket*> m_clients;

    FRVGLJobController* m_glJob;
    unsigned int m_port;
};


}} //end tinia::trell
