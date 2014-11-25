#pragma once
#include <QRunnable>
#include <QTextStream>
#include "tinia/jobcontroller.hpp"
#include "tinia/model/impl/xml/XMLHandler.hpp"
#include "tinia/qtcontroller/moc/OpenGLServerGrabber.hpp"
#include "tinia/qtcontroller/moc/Invoker.hpp"

namespace tinia {
namespace qtcontroller {
namespace impl {

class ServerThread : public QRunnable
{
public:
    explicit ServerThread(OpenGLServerGrabber* grabber,
                          Invoker* mainthread_invoker,
                          tinia::jobcontroller::Job* job,
                          int socket );

    void run();
    
private:

    bool isLongPoll(const QString& request);

    /** Collects the rgb buffer data and returns it as text. Optionally also collects depth and transformation data.
     */
    void getSnapshotTxt( QTextStream &os, const QString &request,
                         tinia::jobcontroller::Job* job,
                         tinia::qtcontroller::impl::OpenGLServerGrabber* grabber,
                         const bool with_depth );

    /** Collects a rgb buffer data and returns it in a google protocol buffer TiniaProtoBuf object. This object can also
     *  contain depth and transformation data (optional proto buffer fields).
     *  getSnapshotBundleTxt() seems not to be used, and we therefore do not need a replica of it.
     */
    void getSnapshotBytes( QByteArray* &protoBytes, const QString &request,
                           tinia::jobcontroller::Job* job,
                           tinia::qtcontroller::impl::OpenGLServerGrabber* grabber,
                           const bool with_depth );

    /** Collects view matrix, projection matrix, rgb buffer and depth buffer, and writes them out as a JSON object.
     */
    void getSnapshotBundleTxt(QTextStream &os, const QString &request,
                              tinia::jobcontroller::Job* job,
                              tinia::qtcontroller::impl::OpenGLServerGrabber* grabber);

    /** Handles non-static content, if applicable.
     * @returns true if the file is non-static, false otherwise.
     */
    bool handleNonStatic(QTextStream& os, const QString& file,
                         const QString& request);

    /** Handles non-static content where we want to sent raw bytes of images, if applicable.
     * @returns true if the file is non-static, false otherwise.
     */
    bool handleNonStatic(QByteArray* &protoBytes, QTextStream& os, const QString& file,
                         const QString& request);

    void updateState(const QString& request);

    /** Writes the error code to the stream formated as HTTP requires,
     * with the optional message formated in HTML
     */
    void errorCode(QTextStream& os, unsigned int code, const QString& msg);
    QString getStaticContent(const QString& uri);

    int m_socket;

    tinia::model::impl::xml::XMLHandler m_xmlHandler;
    tinia::jobcontroller::Job*          m_job;
    OpenGLServerGrabber*                m_grabber;
    Invoker*                            m_mainthread_invoker;
};

} // namespace impl
} // namespace qtcontroller
} // namespace tinia
