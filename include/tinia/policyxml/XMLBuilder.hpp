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

#include <unordered_map>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include "tinia/policy/ElementData.hpp"
#include "tinia/policy/StateElement.hpp"
#include "tinia/policy/StateSchemaElement.hpp"
#include "tinia/policyxml/utils.hpp"
#include "tinia/policy/GUILayout.hpp"

namespace tinia {
namespace policyxml {

/** \class XMLBuilder
     XMLBuilder is responsible for generating an xml-document from a policy.
  */
class XMLBuilder {
public:
    /** Create an XMLBuilder instance.
      \param stateDelta The list of changed StateElements
      \param stateSchemaDelta The list of changed StateSchemaElements
      \param guiLayoutDelta The list of changed GUIElements
      \param revisionNumber The revision number this instance of XMLBuilder is instantiated for.
      */
    XMLBuilder( const std::vector<policy::StateElement> &stateDelta,
                const std::vector<policy::StateSchemaElement> &stateSchemaDelta,
                policy::gui::Element* rootGUIElement,
                unsigned int revisionNumber );

    /** Return a pointer to an XML-document describing the changes to the policy since rev_number.
        \param rev_number Integer which determines which determines what starting revision it should be for the
                returned document. An argument of zero will yield all elements in the policy.
        \return A pointer to an xml-document. The caller takes ownership of the
                pointer and is responsible for managing it's lifetime.
      */
    xmlDocPtr getDeltaDocument( );

private:
    void setPolicyAttributes();
    void buildSchemaXML();
    void buildSchemaXMLForElement( xmlNodePtr, const std::string&, const policy::StateSchemaElement& );
    void buildComplexTypeSchemaXML( xmlNodePtr, const std::string&, const policy::StateSchemaElement& );
    void buildSimpleTypeSchemaXML( xmlNodePtr,  const std::string&, const policy::StateSchemaElement& );
    void buildMatrixTypeSchemaXML( xmlNodePtr schemaroot );

    /** The State part of the policy is filled.
      \param Only state entries with revision number >= rev_number_start will be included. 0 => all, revisionNumber => nothing
      */
    void buildStateXML( const unsigned rev_number_start );
    void buildStateXMLForElement( xmlNodePtr parent, const std::string& name, const policy::StateElement& elementData );
    void buildComplexElementStateXML( xmlNodePtr, const policy::StateElement& elementData );
    void buildSimpleElementStateXML( xmlNodePtr, const policy::StateElement& elementData );
    void buildSimpleGuiLayout_alpha();
    void buildSimpleGuiLayout();
    /**
      Recursive function to build the GUI
      */
    void buildGUILayout(policy::gui::Element * root, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addPolicyGUIElement(policy::gui::KeyValue* element,
                             std::string type, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addElementGroup(policy::gui::ElementGroup* element, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addLayout(std::string type, policy::gui::Container1D<policy::gui::Element>* layout,
                         xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addHorizontalLayout(policy::gui::HorizontalLayout* layout, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addVerticalLayout(policy::gui::VerticalLayout* layout, xmlNodePtr parent);



    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addCanvas(policy::gui::Canvas* element, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addGridLayout(std::string type, policy::gui::Grid* grid, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addTabLayout(std::string type, policy::gui::TabLayout* tabLayout, xmlNodePtr parent);


    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addSpace(std::string type, xmlNodePtr parent);


    xmlNodePtr addVisibilityKeys(xmlNodePtr xmlElementPtr, policy::gui::Element* element);

    xmlNodePtr addElementKeys(xmlNodePtr xmlElementPtr, policy::gui::KeyValue* element );

    xmlNodePtr addPopupButton(xmlNodePtr parent, policy::gui::PopupButton* button);
    bool elementLacksRestrictions( const policy::StateSchemaElement& elementData ) const;

    template<class T>
    bool isComplexElement( const T& elementData ) const;

    xmlDocPtr doc;
    xmlNodePtr root;
    xmlNodePtr policy;
    xmlNodePtr schema;
    xmlNodePtr state;
    xmlNodePtr guiLayout;

    xmlNsPtr xsd;
    xmlNsPtr xsi;
    xmlNsPtr tns;

    //const std::unordered_map<std::string, policy::ElementData> stateHash;
    const unsigned int revisionNumber;


    const std::vector<policy::StateElement> &m_stateDelta;
    const std::vector<policy::StateSchemaElement> &m_stateSchemaDelta;
    policy::gui::Element* m_rootGUIElement;
};
}
}
