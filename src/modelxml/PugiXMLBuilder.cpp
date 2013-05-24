#include "tinia/model/impl/xml/PugiXMLBuilder.hpp"

namespace tinia {
namespace model {
namespace impl {
namespace xml {

PugiXMLBuilder::PugiXMLBuilder(boost::shared_ptr<tinia::model::ExposedModel> exposedModel)
    : m_exposedModel(exposedModel)
{
}

} // namespace xml
} // namespace impl
} // namespace model
} // namespace tinia
