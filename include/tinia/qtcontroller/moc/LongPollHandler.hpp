#pragma once

#include <QThread>
#include <QTextStream>
#include <QTcpSocket>
#include <QMutex>
#include <QWaitCondition>
#include <tinia/model.hpp>
#include <tinia/model/impl/xml/XMLHandler.hpp>

namespace tinia {
namespace qtcontroller {
namespace impl {

class LongPollHandler : public QObject, public tinia::model::StateListener
{
    Q_OBJECT
public:
    explicit LongPollHandler(QTextStream& os,
                             const QString& request,
                             std::shared_ptr<tinia::model::ExposedModel> model,
                             QObject *parent = 0);

    ~LongPollHandler();

    void handle();

    void stateElementModified(model::StateElement *stateElement);
    
signals:
    
public slots:

private:
    char m_buffer[100000];

    bool addExposedModelUpdate(QTextStream& os, unsigned int revision);
    QString m_request;
    QWaitCondition m_waitCondition;
    QMutex m_mutex;
    std::shared_ptr<tinia::model::ExposedModel> m_model;
    tinia::model::impl::xml::XMLHandler m_xmlHandler;
    QTextStream& m_textStream;
};

} // namespace impl
} // namespace qtcontroller
} // namespace tinia
