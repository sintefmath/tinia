#include "tinia/qtcontroller/impl/ServerThread.hpp"
#include <QTcpSocket>
#include "tinia/qtcontroller/impl/http_utils.hpp"
#include "GL/glew.h"
#include <QImage>
#include <QBuffer>
#include <QRegExp>
#include "tinia/renderlist.hpp"
#include <QFile>
#include <QMutexLocker>
#include "tinia/qtcontroller/moc/LongPollHandler.hpp"
#include "tinia/model/ExposedModelLock.hpp"
#include "tinia/protobuf/TiniaProtoBuf.pb.h"

namespace {

class RenderListFetcher : public QRunnable
{
public:
    explicit RenderListFetcher( QTextStream& reply,
                                const QString& request,
                                tinia::jobcontroller::Job* job )
        : m_reply( reply ),
          m_request( request )
    {
        using namespace tinia::qtcontroller::impl;
        
        m_job = dynamic_cast<tinia::jobcontroller::OpenGLJob*>( job );
        if( m_job == NULL ) {
            throw std::invalid_argument("This is not an OpenGL job!");
        }

        m_params = parseGet<boost::tuple<std::string, unsigned int> >( decodeGetParameters(request),
                                                                       "key timestamp" );
    }
    
    ~RenderListFetcher()
    {
        using namespace tinia::qtcontroller::impl;

        m_reply << httpHeader("application/xml") << "\r\n";
        m_reply << m_update << "\n";
    }
    
    void
    run()
    {
        // runs as GUI thread
        using namespace tinia::renderlist;
        const DataBase* db = m_job->getRenderList( "session", m_params.get<0>() );
        if(db) {
            std::string list = getUpdateXML( db, ENCODING_JSON, m_params.get<1>() );
            m_update = QString( list.c_str() );
        }
    }
    
protected:
    QTextStream&                            m_reply;
    const QString&                          m_request;
    tinia::jobcontroller::OpenGLJob*        m_job;
    boost::tuple<std::string, unsigned int> m_params;
    QString                                 m_update;
};

// Need a copy of this class that can take a snapshot of the screen and store it as binary data.
class SnapshotAsTextFetcher : public QRunnable
{
public:
    
    explicit SnapshotAsTextFetcher( QTextStream& reply,
                                    const QString& request,
                                    const std::string &proper_key_to_use,
                                    tinia::jobcontroller::Job* job,
                                    tinia::qtcontroller::impl::OpenGLServerGrabber* gl_grabber,
                                    const bool getRBGsnapshot)
        : m_reply( reply ),
          m_request( request ),
          m_job( NULL ),
          m_gl_grabber( gl_grabber ),
          m_gl_grabber_locker( gl_grabber->exclusiveAccessMutex() ),
          m_getRGBsnapshot( getRBGsnapshot )
    {
        using namespace tinia::qtcontroller::impl;
        
        m_job = dynamic_cast<tinia::jobcontroller::OpenGLJob*>( job );
        if( m_job == NULL ) {
            throw std::invalid_argument("This is not an OpenGL job!");
        }
        
        typedef boost::tuple<unsigned int, unsigned int, std::string> params_t;
        params_t arguments = parseGet<params_t >(decodeGetParameters(request),
                                                 "width height key" );
        m_width  = arguments.get<0>();
        m_height = arguments.get<1>();
        m_key    = ( proper_key_to_use != "" ? proper_key_to_use : arguments.get<2>() );
    }
    
    ~SnapshotAsTextFetcher()
    {
        using namespace tinia::qtcontroller::impl;
        QImage img( m_gl_grabber->imageBuffer(),
                    m_width,
                    m_height,
                    QImage::Format_RGB888 );
    
        // This is a temporary fix. The image is reflected through the horizontal
        // line y=height ((x, y) |--> (x, h-y) ).
        QTransform flipTransformation(1, 0,
                                      0, -1,
                                      0, m_height);
        img = img.transformed(flipTransformation);
        QBuffer qBuffer;
        img.save(&qBuffer, "png");
        //m_gl_grabber_locker.unlock();
        
        QString str( QByteArray( qBuffer.data(),
                                 int(qBuffer.size()) ).toBase64() );
        m_reply << str;
    }
    
    void
    run()
    {
        if ( m_getRGBsnapshot ) {
            m_gl_grabber->grabRGB( m_job, m_width, m_height, m_key );
        } else {
            m_gl_grabber->grabDepth( m_job, m_width, m_height, m_key );
        }
    }
    
protected:
    QTextStream&                                    m_reply;
    const QString&                                  m_request;
    tinia::jobcontroller::OpenGLJob*                m_job;
    tinia::qtcontroller::impl::OpenGLServerGrabber* m_gl_grabber;
    QMutexLocker                                    m_gl_grabber_locker;
    unsigned int                                    m_width;
    unsigned int                                    m_height;
    std::string                                     m_key;
    bool                                            m_getRGBsnapshot;
};


/** This will be an almost copy of the above class SnapshotAsTextFetcher.
 *  It will get the snapshot of the sceen as but not do the base 64 encoding,
 *  but rather store it in its binary form.
 */
class SnapshotAsBytesFetcher : public QRunnable
{
public:

    explicit SnapshotAsBytesFetcher( QDataStream& reply,
                                    const QString& request,
                                    const std::string &proper_key_to_use,
                                    tinia::jobcontroller::Job* job,
                                    tinia::qtcontroller::impl::OpenGLServerGrabber* gl_grabber,
                                    const bool getRBGsnapshot)
        : m_reply( reply ),
          m_request( request ),
          m_job( NULL ),
          m_gl_grabber( gl_grabber ),
          m_gl_grabber_locker( gl_grabber->exclusiveAccessMutex() ),
          m_getRGBsnapshot( getRBGsnapshot )
    {
        using namespace tinia::qtcontroller::impl;

        m_job = dynamic_cast<tinia::jobcontroller::OpenGLJob*>( job );
        if( m_job == NULL ) {
            throw std::invalid_argument("This is not an OpenGL job!");
        }

        typedef boost::tuple<unsigned int, unsigned int, std::string> params_t;
        params_t arguments = parseGet<params_t >(decodeGetParameters(request),
                                                 "width height key" );
        m_width  = arguments.get<0>();
        m_height = arguments.get<1>();
        m_key    = ( proper_key_to_use != "" ? proper_key_to_use : arguments.get<2>() );
    }

    ~SnapshotAsBytesFetcher()
    {
        using namespace tinia::qtcontroller::impl;
        QImage img( m_gl_grabber->imageBuffer(),
                    m_width,
                    m_height,
                    QImage::Format_RGB888 );

        // This is a temporary fix. The image is reflected through the horizontal
        // line y=height ((x, y) |--> (x, h-y) ).
        QTransform flipTransformation(1, 0,
                                      0, -1,
                                      0, m_height);
        img = img.transformed(flipTransformation);
        QBuffer qBuffer;
        img.save(&qBuffer, "png");
        //m_gl_grabber_locker.unlock();

        QByteArray str( qBuffer.data(), int(qBuffer.size()) );
        std::cout << "SnapshotAsBytesFetcher::~SnapshotAsBytesFetcher - returning " << qBuffer.size() << " bytes." << std::endl;
        m_reply << str;
    }

    void
    run()
    {
        if ( m_getRGBsnapshot ) {
            m_gl_grabber->grabRGB( m_job, m_width, m_height, m_key );
        } else {
            m_gl_grabber->grabDepth( m_job, m_width, m_height, m_key );
        }
    }

protected:
    QDataStream&                                    m_reply;
    const QString&                                  m_request;
    tinia::jobcontroller::OpenGLJob*                m_job;
    tinia::qtcontroller::impl::OpenGLServerGrabber* m_gl_grabber;
    QMutexLocker                                    m_gl_grabber_locker;
    unsigned int                                    m_width;
    unsigned int                                    m_height;
    std::string                                     m_key;
    bool                                            m_getRGBsnapshot;
};




}


namespace tinia {
namespace qtcontroller {
namespace impl {

ServerThread::ServerThread(OpenGLServerGrabber* grabber,
                           Invoker* mainthread_invoker,
                           tinia::jobcontroller::Job* job,
                           int socket ) :
    m_socket(socket),
    m_xmlHandler(job->getExposedModel()),
    m_job(job),
    m_grabber(grabber),
    m_mainthread_invoker(mainthread_invoker)
{
}

// This is the caller for where we create the JSON (protobuf object to be).
// This should perhaps be duplicated as "runBinary()"
void ServerThread::run()
{
    QTcpSocket socket;

    socket.setSocketDescriptor(m_socket);
    socket.waitForReadyRead();

    QByteArray* responseBuffer = nullptr;

    if (socket.canReadLine()) {

        QByteArray request = socket.readAll();
        while(!request.contains("\r\n\r\n")) {
            socket.waitForBytesWritten();
            request += socket.readAll();
        }
        //Now we have the whole header
        QRegExp contentLengthExpression("Content-Length: (\\d+)\\s*\\r\\n");

        if (contentLengthExpression.indexIn(request) != -1) {
            QString contentLengthGroup = contentLengthExpression.cap(1);
            int contentLength = contentLengthGroup.toInt();
            
            int headerSize = request.indexOf("\r\n\r\n") + 4;
            while(request.size()  - headerSize < contentLength) {
                socket.waitForBytesWritten();
                request += socket.readAll();
            }
        }

        if(isLongPoll(request)) {
            QTextStream os(&socket);
            LongPollHandler handler(os, request, m_job->getExposedModel());
            handler.handle();
        }
        else if (isGetOrPost(request)) {

            QString requestURI = getRequestURI(request); // Should return a filename with .txt or .xml
            if (requestURI.split('.').last() == "txt") {
                std::cout << "\n\nServerThread::run() - Starting on txt-request" << std::endl;

                // Still need a QTextStream in case handleNonStatic throws an error, but it should not be connected on socket yet!
                QString errorString;
                QTextStream textStream(&errorString);
                if (!handleNonStatic(responseBuffer, textStream, requestURI, request)) {
                    std::cout << "ServerThread::run() - Not able to handleNonStatic..." << std::endl;
                    //QTextStream os(&socket);
                    textStream.setAutoDetectUnicode(true);
                    textStream << getStaticContent(getRequestURI(request)) << "\r\n";
                    QTextStream os(&socket);
                    os << errorString;
                }
                else {
                    std::cout << "ServerThread::run() - Handled nonStatic" << std::endl;
                    std::cout << "ServerThread::run() - responseBuffer is at addresse " << responseBuffer << std::endl;
                    std::cout << "ServerThread::run() - responseBuffer is size " << responseBuffer->size() << std::endl;

                    QDataStream ds(&socket);
                    ds << httpHeader(getMimeType("file.bin"), 200, QString("utf-8"), (unsigned int)(responseBuffer->size())).toLocal8Bit();
                    //std::cerr << httpHeader(getMimeType("file.bin"), 200, QString("utf-8"), (unsigned int)(responseBuffer->size())); //.toLocal8Bit();
                    ds << QString("\r\n").toLocal8Bit();
                    //ds << QString("\r\n").toAscii();
                    //std::cerr << QString("\r\n");
                    ds << responseBuffer;
                    //std::cerr << "trying to dump response buffer" << std::endl;
                    //std::cerr << responseBuffer;
                    std::cout << "ServerThread::run() - Finished filling QDataStream connected to socket." << std::endl;
                }

            }
            else {
                std::cout << "\n\nServerThread::run() - non-txt request." << std::endl;
                QTextStream os(&socket);
                os.setAutoDetectUnicode(true);
                if(!handleNonStatic(os, getRequestURI(request), request)) {
                    os << getStaticContent(getRequestURI(request)) << "\r\n";
                }
            }
           // socket.disconnectFromHost();

        }

        socket.close();
        socket.waitForDisconnected();

        // Since we allocate memory on the heap for the protocol buffer:
        if ( responseBuffer != nullptr ) {
            delete(responseBuffer);
        }
    }
}


bool ServerThread::isLongPoll(const QString &request)
{
    return getRequestURI(request) == "/getExposedModelUpdate.xml";
}


void ServerThread::getSnapshotTxt( QTextStream &os, const QString &request,
                                   tinia::jobcontroller::Job* job,
                                   tinia::qtcontroller::impl::OpenGLServerGrabber* grabber,
                                   const bool with_depth )
{
    boost::tuple<unsigned int, unsigned int, std::string, std::string> arguments =
            parseGet< boost::tuple<unsigned int, unsigned int, std::string, std::string> >( decodeGetParameters(request), "width height key viewer_key_list" );
    std::string key = arguments.get<2>();
    std::string viewer_key_list = arguments.get<3>();

    os << httpHeader(getMimeType("file.txt")) << "\r\n{ ";

    QString viewer_keys(viewer_key_list.c_str());
    QStringList vk_list = viewer_keys.split(',');

    for (int i=0; i<vk_list.size(); i++) {
        QString k = vk_list[i];

        // Now building the JSON entry for this viewer/key
        os << k << ": { \"rgb\": \"";
        {
            SnapshotAsTextFetcher f( os, request, k.toStdString(), job, grabber, true /* RGB requested */ );
            m_mainthread_invoker->invokeInMainThread( &f, true );
        }
        os << "\"";
        if (with_depth) {
            os << ", \"depth\": \"";
            {
                SnapshotAsTextFetcher f( os, request, k.toStdString(), job, grabber, false /* Depth requested */ );
                m_mainthread_invoker->invokeInMainThread( &f, true );
            }
            os << "\", \"view\": \"";
            tinia::model::Viewer viewer;
            m_job->getExposedModel()->getElementValue( k.toStdString(), viewer );
            {
                for (size_t i=0; i<15; i++) {
                    os << viewer.modelviewMatrix[i] << " ";
                }
                os << viewer.modelviewMatrix[15];
            }
            os << "\", \"proj\": \"";
            {
                for (size_t i=0; i<15; i++) {
                    os << viewer.projectionMatrix[i] << " ";
                }
                os << viewer.projectionMatrix[15];
            }
            os << "\"";
        }
        os << " }";
        if ( i < vk_list.size() - 1 ) {
            os << ", ";
        }
    }

    os << "}";
}

// This should be something similar to the gtetSnapshotTxt, but with binary data instead.
// This function should return a google protocol buffer, which should be used instead of the QTextStream object.
void ServerThread::getSnapshotBytes(QByteArray* &protoBytes, const QString &request,
                                     tinia::jobcontroller::Job* job,
                                     tinia::qtcontroller::impl::OpenGLServerGrabber* grabber,
                                     const bool with_depth )
{
    boost::tuple<unsigned int, unsigned int, std::string, std::string> arguments =
            parseGet< boost::tuple<unsigned int, unsigned int, std::string, std::string> >( decodeGetParameters(request), "width height key viewer_key_list" );
    std::string key = arguments.get<2>();
    std::string viewer_key_list = arguments.get<3>();

    // QTextStream os;
    // os << httpHeader(getMimeType("file.bin")) << "\r\n{ ";
    // httpHeader should be attached at a later stage, from this function's caller.
    // Binary body will require that we know the size of blob.


    tinia::protobuf::TiniaProtoBuf proto;

    std::cout << "ServerThread::getSnapshotBytes() - Start building protocol buffer" << std::endl;
    QString viewer_keys(viewer_key_list.c_str());
    QStringList vk_list = viewer_keys.split(',');

    for (int i=0; i<vk_list.size(); i++) {
        std::cout << "ServerThread::getSnapshotBytes() - Creating viewer " << i << std::endl;
        QString k = vk_list[i];
        tinia::protobuf::TiniaProtoBuf_Viewer *proto_viewer = proto.add_viewer();
        proto_viewer->set_viewer_key(k.toStdString());

        // Now building the JSON entry for this viewer/key
        //os << k << ": { \"rgb\": \"";

       QByteArray rgb_bytes;
       QDataStream rgb_ds(&rgb_bytes, QIODevice::ReadWrite);
       {
            // The Snapshot is written in the destructor, so we need to close the block before we write to the protocol buffer!
            SnapshotAsBytesFetcher f( rgb_ds, request, k.toStdString(), job, grabber, true /* RGB requested */ );
            m_mainthread_invoker->invokeInMainThread( &f, true );
        }
        proto_viewer->set_rgb(rgb_bytes, rgb_bytes.size());
        //proto_viewer->set_rgb(QByteArray("A small byte array created from a char*"));
        std::cout << "ServerThread::getSnapshotBytes() - Added " << rgb_bytes.size() << " bytes as rgb." << std::endl;

        // os << "\"";
        if (with_depth) {
            //os << ", \"depth\": \"";
            QByteArray depth_bytes;
            QDataStream depth_ds(&depth_bytes, QIODevice::ReadWrite);
            {
                SnapshotAsBytesFetcher f( depth_ds, request, k.toStdString(), job, grabber, false /* Depth requested */ );
                m_mainthread_invoker->invokeInMainThread( &f, true );
            }
            proto_viewer->set_depth(depth_bytes, depth_bytes.size());
            std::cout << "ServerThread::getSnapshotBytes() - Added " << depth_bytes.size() << " bytes as depth." << std::endl;

            //os << "\", \"view\": \"";
            tinia::model::Viewer viewer;
            m_job->getExposedModel()->getElementValue( k.toStdString(), viewer );

            QString view_qString;
            QTextStream view_stream(&view_qString);
            {
                for (size_t i=0; i<15; i++) {
                    view_stream << viewer.modelviewMatrix[i] << " ";
                }
                view_stream << viewer.modelviewMatrix[15];
            }
            proto_viewer->set_view(view_qString.toStdString());
            std::cout << "ServerThread::getSnapshotBytes() - Added the view matrix as text." << std::endl;

            //os << "\", \"proj\": \"";
            QString proj_qString;
            QTextStream proj_stream(&proj_qString);
            {
                for (size_t i=0; i<15; i++) {
                    proj_stream << viewer.projectionMatrix[i] << " ";
                }
                proj_stream << viewer.projectionMatrix[15];
            }
            proto_viewer->set_proj(proj_qString.toStdString());
            std::cout << "ServerThread::getSnapshotBytes() - Added the proj matrix as text." << std::endl;

            //os << "\"";
        }
        //os << " }";
        //if ( i < vk_list.size() - 1 ) {
        //    os << ", ";
        //}
    }

    protoBytes = new QByteArray(proto.SerializeAsString().c_str(), proto.ByteSize());
    std::cout << "ServerThread::getSnapshotBytes() - Created protocol buffer of size " << protoBytes->size() << " and returning." << std::endl;
    //os << "}";
}


bool ServerThread::handleNonStatic(QByteArray* &protoBytes, QTextStream &os, const QString &file,
                                   const QString &request)

{
    try {
        if(file == "/snapshot.txt") { // Will be used for non-autoProxy mode
            updateState(request);
            getSnapshotBytes( protoBytes, request, m_job, m_grabber, false );
            return true;
        }
        else if ( file == "/snapshot_bundle.txt" ) { // Will be used when in autoProxy-mode
            updateState(request);
            getSnapshotBytes( protoBytes, request, m_job, m_grabber, true );
            return true;
        }

    } catch(std::invalid_argument& e) {
        errorCode(os, 400, e.what());
        return true;
    }

   return false;
}


bool ServerThread::handleNonStatic(QTextStream &os, const QString& file,
                                   const QString& request)
{
    try {
        if(file == "/getRenderList.xml") {
            RenderListFetcher f( os, request, m_job );
            m_mainthread_invoker->invokeInMainThread( &f, true );
            return true;
        }
        else if(file =="/updateState.xml") {
            updateState(request);
            os << httpHeader("application/xml") << "\r\n";
            return true;
        }

    } catch(std::invalid_argument& e) {
        errorCode(os, 400, e.what());
        return true;
    }

    return false;
}


void ServerThread::updateState(const QString &request)
{
	tinia::model::ExposedModelLock lock(m_job->getExposedModel());
    std::string content = getPostContent(request).toStdString();
    if( !content.empty() ) {
        m_xmlHandler.updateState(content.c_str(), content.size());
    }
}


void ServerThread::errorCode(QTextStream &os, unsigned int code, const QString &msg)
{
    os << "HTTP/1.1 " << QString::number(code) << "\r\n"
       << "Content-Type: text/html; charset=\"utf-8\"\r\n"
       << "<html>\n<head>\n<title>Error: " << QString::number(code) << "</title>"
       << "</head>\n" <<"<body>\n" << "<h1>Error code: " << code << "</h1>\n"<<msg
       <<"</body>\n"<<"</html>\n";
}


QString ServerThread::getStaticContent(const QString &uri)
{

    QString fullPath = ":javascript/" + uri;

    QFile file(fullPath);
    if(file.open(QIODevice::ReadOnly)) {
        QString reply =QString("HTTP/1.1 200 Ok\r\n") +
                QString("Content-Type: ") + getMimeType(uri) +
                QString("; charset=\"utf-8\"\r\n\r\n\r\n") + file.readAll() + "\r\n";
        return reply;
    }
    else {
        return "HTTP/1.1 404 Not Found\r\n";
    }
}

} // namespace impl
} // namespace qtcontroller
} // namespace tinia
