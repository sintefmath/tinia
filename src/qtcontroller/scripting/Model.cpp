#include <tinia/qtcontroller/scripting/Model.hpp>

namespace tinia {
namespace qtcontroller {
namespace scripting {

Model::Model(std::shared_ptr<tinia::model::ExposedModel> model, QObject *parent) :
    QObject(parent)
{
}

} // namespace scripting
} // namespace qtcontroller
} // namespace tinia
