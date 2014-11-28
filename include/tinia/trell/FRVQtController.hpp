#pragma once

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QWebSocket);

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
    void onConnect();
    void onMessageReceived();

private:
    QWebSocket* m_socket;
    FRVGLJobController* m_glJob;
    
};


}} //end tinia::trell
