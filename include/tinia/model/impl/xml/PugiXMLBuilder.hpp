#pragma once
#include <tinia/model/ExposedModel.hpp>

namespace tinia {
namespace model {
namespace impl {
namespace xml {

class PugiXMLBuilder
{
public:
    PugiXMLBuilder(boost::shared_ptr<tinia::model::ExposedModel> exposedModel);

private:
    boost::shared_ptr<tinia::model::ExposedModel> m_exposedModel;

};

} // namespace xml
} // namespace impl
} // namespace model
} // namespace tinia

