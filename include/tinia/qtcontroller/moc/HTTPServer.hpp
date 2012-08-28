#pragma once
#include <QTcpServer>
#include <QTcpSocket>

namespace tinia {
namespace qtcontroller {
namespace impl {

class HTTPServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit HTTPServer(QObject *parent = 0);

    void incomingConnection(int socket);
    
signals:
    
private slots:
    void readyRead();
    void discardClient();
private:
    QString getStaticContent(const QString& uri);
};

}
} // namespace qtcontroller
} // namespace tinia

