/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include "tinia/model/impl/ElementData.hpp"
#include "tinia/model/StateElement.hpp"
#include "tinia/model/StateSchemaElement.hpp"
#include "tinia/model/impl/xml/utils.hpp"
#include "tinia/model/GUILayout.hpp"

namespace tinia {
namespace model {
namespace impl {
namespace xml {


/** \class XMLBuilder
     XMLBuilder is responsible for generating an xml-document from a model.
  */
class XMLBuilder {
public:
    /** Create an XMLBuilder instance.
      \param stateDelta The list of changed StateElements
      \param stateSchemaDelta The list of changed StateSchemaElements
      \param guiLayoutDelta The list of changed GUIElements
      \param revisionNumber The revision number this instance of XMLBuilder is instantiated for.
      */
    XMLBuilder( const std::vector<model::StateElement> &stateDelta,
                const std::vector<model::StateSchemaElement> &stateSchemaDelta,
                model::gui::Element* rootGUIElement,
                unsigned int revisionNumber );

    /** Return a pointer to an XML-document describing the changes to the model since rev_number.
        \param rev_number Integer which determines which determines what starting revision it should be for the
                returned document. An argument of zero will yield all elements in the model.
        \return A pointer to an xml-document. The caller takes ownership of the
                pointer and is responsible for managing it's lifetime.
      */
    xmlDocPtr getDeltaDocument( );

private:
    void setExposedModelAttributes();
    void buildSchemaXML();
    void buildSchemaXMLForElement( xmlNodePtr, const std::string&, const model::StateSchemaElement& );
    void buildComplexTypeSchemaXML( xmlNodePtr, const std::string&, const model::StateSchemaElement& );
    void buildSimpleTypeSchemaXML( xmlNodePtr,  const std::string&, const model::StateSchemaElement& );
    void buildMatrixTypeSchemaXML( xmlNodePtr schemaroot );

    /** The State part of the model is filled.
      \param Only state entries with revision number >= rev_number_start will be included. 0 => all, revisionNumber => nothing
      */
    void buildStateXML( const unsigned rev_number_start );
    void buildStateXMLForElement( xmlNodePtr parent, const std::string& name, const model::StateElement& elementData );
    void buildComplexElementStateXML( xmlNodePtr, const model::StateElement& elementData );
    void buildSimpleElementStateXML( xmlNodePtr, const model::StateElement& elementData );
    void buildSimpleGuiLayout_alpha();
    /**
      Recursive function to build the GUI
      */
    void buildGUILayout(model::gui::Element * root, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addExposedModelGUIElement(model::gui::KeyValue* element,
                             std::string type, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addElementGroup(model::gui::ElementGroup* element, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addLayout(std::string type, model::gui::Container1D<model::gui::Element>* layout,
                         xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addHorizontalLayout(model::gui::HorizontalLayout* layout, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addVerticalLayout(model::gui::VerticalLayout* layout, xmlNodePtr parent);



    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addCanvas(model::gui::Canvas* element, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addGridLayout(std::string type, model::gui::Grid* grid, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addTabLayout(std::string type, model::gui::TabLayout* tabLayout, xmlNodePtr parent);


    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addSpace(std::string type, xmlNodePtr parent);


    xmlNodePtr addVisibilityKeys(xmlNodePtr xmlElementPtr, model::gui::Element* element);

    xmlNodePtr addElementKeys(xmlNodePtr xmlElementPtr, model::gui::KeyValue* element );

    xmlNodePtr addPopupButton(xmlNodePtr parent, model::gui::PopupButton* button);
    bool elementLacksRestrictions( const model::StateSchemaElement& elementData ) const;

    template<class T>
    bool isComplexElement( const T& elementData ) const;

    xmlDocPtr doc;
    xmlNodePtr root;
    xmlNodePtr model;
    xmlNodePtr schema;
    xmlNodePtr state;
    xmlNodePtr guiLayout;

    xmlNsPtr xsd;
    xmlNsPtr xsi;
    xmlNsPtr tns;

    const unsigned int revisionNumber;


    const std::vector<model::StateElement> &m_stateDelta;
    const std::vector<model::StateSchemaElement> &m_stateSchemaDelta;
    model::gui::Element* m_rootGUIElement;
};
}
}
}
}
