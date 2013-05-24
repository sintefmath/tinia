#pragma once
#include <string>
#include <boost/shared_ptr.hpp>
#include "tinia/model/ExposedModel.hpp"

namespace tinia { namespace model { namespace impl { namespace xml {

/**
 * @brief The PugiXMLReader class uses pugixml (pugixml.googlecode.com) to read
 *        the XML-properties.
 */
class PugiXMLReader
{
public:
    PugiXMLReader(boost::shared_ptr<tinia::model::ExposedModel> exposedModel);

    /**
     * @brief readState reads the state of the XML-document into the exposedmodel
     * @param documentString the string to read from
     */
    void readState(const std::string& documentString);

    /**
     * @brief readSchema reads the schema of the XML document and updates the model
     * @param documentString the document string to read.
     */
    void readSchema(const std::string& documentString);
private:
    boost::shared_ptr<tinia::model::ExposedModel> m_exposedModel;
};

}}}}

