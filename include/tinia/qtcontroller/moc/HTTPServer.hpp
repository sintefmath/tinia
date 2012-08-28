#pragma once
#include <QTcpServer>
#include <QTcpSocket>

namespace tinia {
namespace qtcontroller {

class HTTPServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit HTTPServer(QObject *parent = 0);

    void request(int socket);
    
signals:
    
private slots:
    void readClient();
};

} // namespace qtcontroller
} // namespace tinia

