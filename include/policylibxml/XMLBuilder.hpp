#pragma once

#include <unordered_map>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include "policylib/ElementData.hpp"
#include "policylib/StateElement.hpp"
#include "policylib/StateSchemaElement.hpp"
#include "policylibxml/utils.hpp"
#include "policylib/GUILayout.hpp"


namespace policylibxml {

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
    XMLBuilder( const std::vector<policylib::StateElement> &stateDelta,
                const std::vector<policylib::StateSchemaElement> &stateSchemaDelta,
                policylib::gui::Element* rootGUIElement,
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
    void buildSchemaXMLForElement( xmlNodePtr, const std::string&, const policylib::StateSchemaElement& );
    void buildComplexTypeSchemaXML( xmlNodePtr, const std::string&, const policylib::StateSchemaElement& );
    void buildSimpleTypeSchemaXML( xmlNodePtr,  const std::string&, const policylib::StateSchemaElement& );
    void buildMatrixTypeSchemaXML( xmlNodePtr schemaroot );

    /** The State part of the policy is filled.
      \param Only state entries with revision number >= rev_number_start will be included. 0 => all, revisionNumber => nothing
      */
    void buildStateXML( const unsigned rev_number_start );
    void buildStateXMLForElement( xmlNodePtr parent, const std::string& name, const policylib::StateElement& elementData );
    void buildComplexElementStateXML( xmlNodePtr, const policylib::StateElement& elementData );
    void buildSimpleElementStateXML( xmlNodePtr, const policylib::StateElement& elementData );
    void buildSimpleGuiLayout_alpha();
    void buildSimpleGuiLayout();
    /**
      Recursive function to build the GUI
      */
    void buildGUILayout(policylib::gui::Element * root, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addPolicyGUIElement(policylib::gui::KeyValue* element,
                             std::string type, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addElementGroup(policylib::gui::ElementGroup* element, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addLayout(std::string type, policylib::gui::Container1D<policylib::gui::Element>* layout,
                         xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addHorizontalLayout(policylib::gui::HorizontalLayout* layout, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addVerticalLayout(policylib::gui::VerticalLayout* layout, xmlNodePtr parent);



    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addCanvas(policylib::gui::Canvas* element, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addGridLayout(std::string type, policylib::gui::Grid* grid, xmlNodePtr parent);

    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addTabLayout(std::string type, policylib::gui::TabLayout* tabLayout, xmlNodePtr parent);


    /**
      Simple helper function to buildGUILayout
      \return the pointer to the created node
      */
    xmlNodePtr addSpace(std::string type, xmlNodePtr parent);


    xmlNodePtr addVisibilityKeys(xmlNodePtr xmlElementPtr, policylib::gui::Element* element);

    xmlNodePtr addElementKeys(xmlNodePtr xmlElementPtr, policylib::gui::KeyValue* element );

    xmlNodePtr addPopupButton(xmlNodePtr parent, policylib::gui::PopupButton* button);
    bool elementLacksRestrictions( const policylib::StateSchemaElement& elementData ) const;

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

    //const std::unordered_map<std::string, policylib::ElementData> stateHash;
    const unsigned int revisionNumber;


    const std::vector<policylib::StateElement> &m_stateDelta;
    const std::vector<policylib::StateSchemaElement> &m_stateSchemaDelta;
    policylib::gui::Element* m_rootGUIElement;
};
}
