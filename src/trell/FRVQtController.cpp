#include "tinia/trell/FRVQtController.hpp"
#include "tinia/trell/FRVGLJobController.hpp"
#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>
#include <QObject>

#include "tinia/trell/FRVProtoBuffers.pb.h"

tinia::trell::FRVQtController::FRVQtController( FRVGLJobController* glJob, int port, QObject* parent )
    :
    QObject(parent),
    m_glJob( glJob ),
    m_port(port),
    m_clients()
{
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("Echo Server"),
        QWebSocketServer::NonSecureMode, this);
    // Listen on the specified port in the local network:
   

    if(m_pWebSocketServer->listen(QHostAddress::Any, m_port)) {
        // if listening successful:
        qDebug() << "SocketServer listening on port " << m_port;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
            this, &FRVQtController::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &FRVQtController::closed);
    }
}

tinia::trell::FRVQtController::~FRVQtController()
{
    m_glJob->finish();
    for( int i = 0 ; i < m_clients.length(); ++i ){
        m_clients[i]->close();
    }
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

//void tinia::trell::FRVQtController::closed()
//{
//
//}

void tinia::trell::FRVQtController::onNewConnection()
{
    qDebug() << "got a new connection attempt";
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &FRVQtController::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &FRVQtController::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &FRVQtController::socketDisconnected);
    if(m_glJob)
        m_glJob->render();

    m_clients << pSocket;

}

void tinia::trell::FRVQtController::processTextMessage(QString message)
{
    qDebug() << "got a new text message";

    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());

    if ( pClient ) {
        pClient->sendTextMessage("frv " + message);
    }

    if(m_glJob)
        m_glJob->render();
}

void tinia::trell::FRVQtController::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "got a new binary message";
    if( !pClient || &m_glJob){
        qDebug() << "Major problem, we are not ready for messages yet. Please try again later!";
        return;
    }
    
   

    frv::imageRequest ir;
    frv::imageResponse response;
    ir.ParseFromString( (std::string)message );
    if( ir.has_zscale() ){
        qDebug() << "it has z-scale, something got through and that was: " << ir.zscale();
    }

    char* results =  m_glJob->render();
    response.set_allocated_image_bytes( (std::string*)results );
    pClient->sendBinaryMessage( response.SerializeAsString().c_str() );
    
}

void tinia::trell::FRVQtController::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}




