#pragma once
#include <QRunnable>
#include <QTextStream>
#include "tinia/jobcontroller.hpp"
#include "tinia/model/impl/xml/XMLHandler.hpp"
#include "tinia/qtcontroller/moc/OpenGLServerGrabber.hpp"

namespace tinia {
namespace qtcontroller {
namespace impl {

class ServerThread : public QRunnable
{
public:
    explicit ServerThread(ImageSource& grabber,
                          tinia::jobcontroller::Job* job,
                          int socket,
                          QObject *parent = 0);

    void run();
    
private:

    bool isLongPoll(const QString& request);
    void getSnapshotTxt(QTextStream& os, const QString& request);

    /** Handles non-static content, if applicable.
     * @returns true if the file is non-static, false otherwise.
     */
    bool handleNonStatic(QTextStream& os, const QString& file,
                         const QString& request);

    void updateState(QTextStream& os, const QString& request);

    void getRenderList(QTextStream& os, const QString& request);

    /** Writes the error code to the stream formated as HTTP requires,
     * with the optional message formated in HTML
     */
    void errorCode(QTextStream& os, unsigned int code, const QString& msg);
    QString getStaticContent(const QString& uri);

    int m_socket;

    tinia::model::impl::xml::XMLHandler m_xmlHandler;
    tinia::jobcontroller::Job* m_job;

    ImageSource& m_grabber;
};

} // namespace impl
} // namespace qtcontroller
} // namespace tinia
