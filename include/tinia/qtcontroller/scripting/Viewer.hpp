#ifndef TINIA_QTOBSERVER_SCRIPTING_VIEWER_HPP
#define TINIA_QTOBSERVER_SCRIPTING_VIEWER_HPP

#include <QObject>

namespace tinia {
namespace qtobserver {
namespace scripting {

class Viewer : public QObject
{
    Q_OBJECT
public:
    explicit Viewer(QObject *parent = 0);
    
signals:
    
public slots:
    
};

} // namespace scripting
} // namespace qtobserver
} // namespace tinia

#endif // TINIA_QTOBSERVER_SCRIPTING_VIEWER_HPP
