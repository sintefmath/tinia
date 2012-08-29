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
    bool isStatic(const QString& file);

    /** Handles non-static content, if applicable.
     * @returns true if the file is non-static, false otherwise.
     */
    bool handleNonStatic(QTextStream& os, const QString& file,
                         const QString& request);

    /** Writes the error code to the stream formated as HTTP requires,
     * with the optional message formated in HTML
     */
    void errorCode(QTextStream& os, unsigned int code, const QString& msg);
    QString getStaticContent(const QString& uri);
};

}
} // namespace qtcontroller
} // namespace tinia

