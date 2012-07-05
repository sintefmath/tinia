#pragma once
#include <QObject>
#include <tinia/model/ExposedModel.hpp>

namespace tinia {
namespace qtcontroller {
namespace scripting {

class Model : public QObject
{
    Q_OBJECT
public:
    explicit Model(std::shared_ptr<tinia::model::ExposedModel> model, QObject *parent = 0);
    
signals:
    
public slots:

private:
    std::shared_ptr<ExposedModel> m_model;
    
};

} // namespace scripting
} // namespace qtcontroller
} // namespace tinia


