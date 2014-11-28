#include "tinia/trell/FRVQtController.hpp"
#include "tinia/trell/FRVGLJobController.hpp"
#include <QtWebSockets/QWebSocket>
#include <QObject>


tinia::trell::FRVQtController::FRVQtController( FRVGLJobController* glJob, QObject* parent /*= Q_NULLPTR */ )
    :
    m_glJob( glJob )
{

}

tinia::trell::FRVQtController::~FRVQtController()
{
    m_glJob->finish();
    m_socket->close();
}

void tinia::trell::FRVQtController::closed()
{

}

void tinia::trell::FRVQtController::onConnect()
{

}

void tinia::trell::FRVQtController::onMessageReceived()
{

}


