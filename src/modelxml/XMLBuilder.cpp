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

#include "tinia/modelxml/XMLBuilder.hpp"

#include <iostream>
#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

namespace tinia {
namespace modelxml {



xmlDocPtr
XMLBuilder::getDeltaDocument() {
   doc = xmlNewDoc( (xmlChar*)( "1.0" ) );
   root = xmlNewNode( 0, (xmlChar*)( "ExposedModelUpdate" ) );
   schema = xmlNewChild( root, 0, BAD_CAST "StateSchema", 0 );

   setExposedModelAttributes();

   xmlDocSetRootElement( doc, root );

   buildSchemaXML();

   state = xmlNewChild( root, 0, BAD_CAST "State", 0 );
   buildStateXML( 0 );

   guiLayout = xmlNewChild( root, 0, BAD_CAST "GuiLayout", 0 );
   buildGUILayout(m_rootGUIElement, guiLayout);
   return doc;
}

void
XMLBuilder::setExposedModelAttributes() {
   tns = xmlNewNs( root, BAD_CAST "http://cloudviz.sintef.no/V1/model", 0 );
   xsi = xmlNewNs( root, BAD_CAST "http://www.w3.org/2001/XMLSchema-instance", BAD_CAST "xsi" );
   xsd = xmlNewNs( schema, BAD_CAST "http://www.w3.org/2001/XMLSchema", BAD_CAST "xsd" );

   xmlSetNsProp( root, xsi, BAD_CAST "schemaLocation", BAD_CAST "http://cloudviz.sintef.no/V1/model ExposedModelUpdateSchema.xsd" );
   xmlSetNsProp( root, tns, BAD_CAST "revision", BAD_CAST boost::lexical_cast<std::string>( revisionNumber ).c_str() );
}

void
XMLBuilder::buildSchemaXML() {
   auto schemaroot = xmlNewChild( schema, xsd, BAD_CAST "schema", 0 );

   auto state = xmlNewChild( schemaroot, xsd, BAD_CAST "element", 0 );
   xmlSetProp( state, BAD_CAST "name", BAD_CAST "State" );

   auto complexType = xmlNewChild( state, xsd, BAD_CAST"complexType", 0 );
   auto all = xmlNewChild( complexType, xsd, BAD_CAST "all", 0 );

   for(auto it = m_stateSchemaDelta.begin(); it!=m_stateSchemaDelta.end(); it++)
   {
      buildSchemaXMLForElement( all, (*it).getKey(), *it );
   }
}

void
XMLBuilder::buildSchemaXMLForElement( xmlNodePtr parent, const std::string& name, const model::StateSchemaElement& elementData ) {
   if ( isComplexElement( elementData ) ) {
      buildComplexTypeSchemaXML( parent, name, elementData );
   } else {
      buildSimpleTypeSchemaXML( parent, name, elementData );
   }
}


void

XMLBuilder::buildComplexTypeSchemaXML( xmlNodePtr parent, const std::string& name, const model::StateSchemaElement& elementData )   {
   auto elementNode = xmlNewChild( parent, xsd, BAD_CAST "complexType", 0 );
   xmlSetProp( elementNode, BAD_CAST "name", BAD_CAST name.c_str() );
   auto sequenceNode = xmlNewChild( elementNode, xsd, BAD_CAST "sequence", 0 );

   const auto& ptree = elementData.getPropertyTree();

   typedef model::StateSchemaElement::PropertyTree::value_type value_type;
   for_each( ptree.begin(), ptree.end(), [sequenceNode, this]( const value_type& kv ) {
             buildSchemaXMLForElement( sequenceNode, kv.first, kv.second.data() );
} );
}

void
    XMLBuilder::buildSimpleTypeSchemaXML( xmlNodePtr parent, const std::string& name, const model::StateSchemaElement& elementData ) {
        auto elementNode = xmlNewChild( parent, xsd, BAD_CAST "element", 0 );

        xmlSetProp( elementNode, BAD_CAST "name", BAD_CAST name.c_str() );

        if ( elementLacksRestrictions( elementData ) ) {
            xmlSetProp( elementNode, BAD_CAST "type", BAD_CAST elementData.getXSDType().c_str() );
        } else {
            if ( elementData.getLength() != elementData.LENGTH_NOT_SET ) {
                buildMatrixTypeSchemaXML( elementNode );
            } else {
                auto simpleType = xmlNewChild( elementNode, xsd, BAD_CAST "simpleType", 0 );
                auto restriction = xmlNewChild( simpleType, xsd, BAD_CAST "restriction", 0 );
                xmlSetProp( restriction, BAD_CAST "base", BAD_CAST elementData.getXSDType().c_str() );

                if ( !elementData.emptyConstraints() ) {
                    auto minInclusive = xmlNewChild( restriction, xsd, BAD_CAST "minInclusive", 0 );
                    xmlSetProp( minInclusive, BAD_CAST "value", BAD_CAST elementData.getMinConstraint().c_str() );

                    auto maxInclusive = xmlNewChild( restriction, xsd, BAD_CAST "maxInclusive", 0 );
                    xmlSetProp( maxInclusive, BAD_CAST "value", BAD_CAST elementData.getMaxConstraint().c_str() );
                }

                if ( !elementData.emptyRestrictionSet() ) {
                    auto& restrictions = elementData.getEnumerationSet();
                    for_each( restrictions.begin(), restrictions.end(), [restriction, this]( std::string s ) {
                        auto enumeration = xmlNewChild( restriction, xsd, BAD_CAST "enumeration", 0 );
                        xmlSetProp( enumeration, BAD_CAST "value", BAD_CAST s.c_str() );
                    });
                }
            }
        }

        if ( !elementData.emptyAnnotation() ) {
            auto annotationNode = xmlNewChild( elementNode, xsd, BAD_CAST "annotation", 0 );
            const auto& annotation  = elementData.getAnnotation();
            for_each( annotation.begin(), annotation.end(), [annotationNode, this]( const std::pair<std::string, std::string>& pair ) {
                auto docNode = xmlNewChild( annotationNode, xsd, BAD_CAST "documentation", BAD_CAST pair.second.c_str() );
                xmlSetProp( docNode, BAD_CAST "xml:lang", BAD_CAST pair.first.c_str() );
            }
            );
        }
}


void
XMLBuilder::buildStateXML( const unsigned rev_number_start ) {

   for(auto it = m_stateDelta.begin(); it!=m_stateDelta.end(); it++)
   {
      buildStateXMLForElement( state, (*it).getKey(), *it );
   }
}

void
XMLBuilder::buildStateXMLForElement( xmlNodePtr parent, const std::string& name, const model::StateElement& elementData ) {
   auto child = xmlNewChild( parent, 0, BAD_CAST name.c_str(), 0 );
   if ( isComplexElement( elementData ) ) {
      buildComplexElementStateXML( child, elementData );
   } else {
      buildSimpleElementStateXML( child, elementData );
   }
}

void
XMLBuilder::buildSimpleElementStateXML( xmlNodePtr node, const model::StateElement& elementData ) {
   xmlNodeAddContent( node, BAD_CAST elementData.getStringValue().c_str() );
}

void
XMLBuilder::buildComplexElementStateXML( xmlNodePtr node, const model::StateElement& elementData ) {
   const auto& ptree = elementData.getPropertyTree();

   typedef model::StateElement::PropertyTree::value_type value_type;
   for_each( ptree.begin(), ptree.end(), [=]( const value_type& kv ) {
         buildStateXMLForElement( node, kv.first, kv.second.data() );
} );
}


void
XMLBuilder::buildMatrixTypeSchemaXML( xmlNodePtr elementRoot ) {
   auto restriction = xmlNewChild( elementRoot, xsd, BAD_CAST "restriction", 0 );

   auto simpleType = xmlNewChild( restriction, xsd, BAD_CAST "simpleType", 0 );
   auto listNode = xmlNewChild( simpleType, xsd, BAD_CAST "list", 0 );
   xmlSetProp( listNode, BAD_CAST "itemType", BAD_CAST "xsd:float" );

   auto lengthNode = xmlNewChild( restriction, xsd, BAD_CAST "length", 0 );
   xmlSetProp( lengthNode, BAD_CAST "value", BAD_CAST "16" );
}

template<class T>
bool XMLBuilder::isComplexElement( const T& elementData ) const {
   return ( elementData.getXSDType() == "xsd:complexType" );
}

bool
XMLBuilder::elementLacksRestrictions( const model::StateSchemaElement& elementData ) const {
   if ( elementData.emptyConstraints() && elementData.emptyRestrictionSet() && ( elementData.getLength() == elementData.LENGTH_NOT_SET ) ) {
      return true;
   }
   return false;
}



// The first generation
void
XMLBuilder::buildSimpleGuiLayout_alpha() {
   auto tabs = xmlNewChild( guiLayout, 0, BAD_CAST "tabs", 0 );
   auto tab = xmlNewChild( tabs, 0, BAD_CAST "tab", 0 );
   /*auto title = */xmlNewChild( tab, 0, BAD_CAST "title", BAD_CAST "Simple Layout" );
   auto grid = xmlNewChild(  tab, 0, BAD_CAST "grid", 0 );
   auto row = xmlNewChild( grid, 0, BAD_CAST "row", 0 );
   auto cella = xmlNewChild( row, 0, BAD_CAST "cell", 0 );
   /*auto canvas = */xmlNewChild( cella, 0, BAD_CAST "Canvas", 0 );

   row = xmlNewChild( grid, 0, BAD_CAST "row", 0 );
   auto cellb = xmlNewChild( row, 0, BAD_CAST "cell", 0 );
   grid = xmlNewChild( cellb, 0, BAD_CAST "grid", 0 );

   row = xmlNewChild( grid, 0, BAD_CAST "row", 0 );
   //   for_each( stateHash.begin(), stateHash.end(),
   //             [row]( const std::pair<std::string, model::impl::ElementData>& kv ) {
   //             auto cell0 = xmlNewChild( row, 0, BAD_CAST "cell", 0 );
   //         auto key = xmlNewChild( cell0, 0, BAD_CAST "string", 0 );
   //   xmlSetProp( key, BAD_CAST "parameter", BAD_CAST kv.first.c_str() );
   //   auto cell1 = xmlNewChild( row, 0, BAD_CAST "cell", 0 );
   //   const string widgetType = kv.second.getWidgetType();
   //   auto value = xmlNewChild( cell1, 0, BAD_CAST widgetType.c_str(), 0 );
   //   xmlSetProp( value, BAD_CAST "parameter", BAD_CAST kv.first.c_str() );
   //}
   //);

}

void
XMLBuilder::buildSimpleGuiLayout() {

   auto tabs = xmlNewChild( guiLayout, 0, BAD_CAST "tabs", 0 );
   auto tab = xmlNewChild( tabs, 0, BAD_CAST "tab", 0 );
   /*auto title = */xmlNewChild( tab, 0, BAD_CAST "title", BAD_CAST "Simple Layout" );
   auto grid = xmlNewChild(  tab, 0, BAD_CAST "grid", 0 );
   auto row = xmlNewChild( grid, 0, BAD_CAST "row", 0 );
   auto cella = xmlNewChild( row, 0, BAD_CAST "cell", 0 );
   /*auto canvas = */xmlNewChild( cella, 0, BAD_CAST "Canvas", 0 );

   row = xmlNewChild( grid, 0, BAD_CAST "row", 0 );
   auto cellb = xmlNewChild( row, 0, BAD_CAST "cell", 0 );
   grid = xmlNewChild( cellb, 0, BAD_CAST "grid", 0 );

   for(auto it = m_stateSchemaDelta.begin(); it !=m_stateSchemaDelta.end();
       it++)
   {
      auto row2 = xmlNewChild( grid, 0, BAD_CAST "row", 0 );
      auto cell0 = xmlNewChild( row2, 0, BAD_CAST "cell", 0 );
      auto key = xmlNewChild( cell0, 0, BAD_CAST "string", 0 );
      xmlSetProp( key, BAD_CAST "parameter", BAD_CAST it->getKey().c_str() );
      auto cell1 = xmlNewChild( row2, 0, BAD_CAST "cell", 0 );
      const string widgetType = it->getWidgetType();
      auto value = xmlNewChild( cell1, 0, BAD_CAST widgetType.c_str(), 0 );
      xmlSetProp( value, BAD_CAST "parameter", BAD_CAST it->getKey().c_str() );
   }
   //   for_each( stateHash.begin(), stateHash.end(),
   //             [grid]( const std::pair<std::string, model::impl::ElementData>& kv ) {
   //             auto row2 = xmlNewChild( grid, 0, BAD_CAST "row", 0 );
   //         auto cell0 = xmlNewChild( row2, 0, BAD_CAST "cell", 0 );
   //   auto key = xmlNewChild( cell0, 0, BAD_CAST "string", 0 );
   //   xmlSetProp( key, BAD_CAST "parameter", BAD_CAST kv.first.c_str() );
   //   auto cell1 = xmlNewChild( row2, 0, BAD_CAST "cell", 0 );
   //   const string widgetType = kv.second.getWidgetType();
   //   auto value = xmlNewChild( cell1, 0, BAD_CAST widgetType.c_str(), 0 );
   //   xmlSetProp( value, BAD_CAST "parameter", BAD_CAST kv.first.c_str() );
   //}
   //);

}



}

modelxml::XMLBuilder::XMLBuilder(const std::vector<model::StateElement> &stateDelta,
                                     const std::vector<model::StateSchemaElement> &stateSchemaDelta,
                                     model::gui::Element* rootGUIElement,
                                     unsigned int revisionNumber)
   : revisionNumber(revisionNumber), m_stateDelta(stateDelta), m_stateSchemaDelta(stateSchemaDelta),
     m_rootGUIElement(rootGUIElement)
{

}

void modelxml::XMLBuilder::buildGUILayout(model::gui::Element *root,
                                              xmlNodePtr parent )
{


   using namespace model::gui;
   switch(root->type())
   {
   case CANVAS:


      addVisibilityKeys(addCanvas(dynamic_cast<Canvas*>(root), parent), root);
      break;
   case TEXTINPUT:


      addVisibilityKeys(addExposedModelGUIElement(dynamic_cast<KeyValue*>(root),
                                            "TextInput", parent), root);
      break;
   case LABEL:


      addVisibilityKeys(addExposedModelGUIElement(dynamic_cast<KeyValue*>(root),
                                            "Label", parent), root);
      break;
   case COMBOBOX:


      addVisibilityKeys(addExposedModelGUIElement(dynamic_cast<KeyValue*>(root),
                                            "ComboBox", parent), root);
      break;
   case RADIOBUTTONS:


      addVisibilityKeys(addExposedModelGUIElement(dynamic_cast<KeyValue*>(root),
                                            "RadioButtons", parent), root);
      break;
   case SPINBOX:


      addVisibilityKeys(addExposedModelGUIElement(dynamic_cast<KeyValue*>(root),
                                            "SpinBox", parent), root);
      break;
   case CHECKBOX:


      addVisibilityKeys(addExposedModelGUIElement(dynamic_cast<KeyValue*>(root),
                                            "Checkbox", parent), root);
      break;
   case BUTTON:


      addVisibilityKeys(addExposedModelGUIElement(dynamic_cast<KeyValue*>(root),
                                            "Button", parent), root);
      break;
   case HORIZONTAL_SLIDER:


      addVisibilityKeys(addExposedModelGUIElement(dynamic_cast<KeyValue*>(root),
                                            "HorizontalSlider", parent), root);
      break;
   case DOUBLE_SPINBOX:


      addVisibilityKeys(addExposedModelGUIElement(dynamic_cast<KeyValue*>(root),
                                            "DoubleSpinBox", parent), root);
      break;


   case ELEMENTGROUP:


      addVisibilityKeys(addElementGroup(dynamic_cast<ElementGroup*>(root), parent),
                        root);
      break;
   case VERTICAL_LAYOUT:


      addVisibilityKeys(addVerticalLayout(dynamic_cast<VerticalLayout*>(root), parent),
                        root);
      break;
   case HORIZONTAL_LAYOUT:


      addVisibilityKeys(addHorizontalLayout(dynamic_cast<HorizontalLayout*>(root), parent),
                        root);
      break;
   case GRID:


      addVisibilityKeys(addGridLayout("Grid", dynamic_cast<Grid*>(root), parent),
                        root);
      break;
   case TAB_LAYOUT:


      addVisibilityKeys(addTabLayout("TabLayout", dynamic_cast<TabLayout*>(root), parent),
                         root);
      break;
   case TAB:
      // Error, this isn't supposed to happen (all tabs are to be handled in
      // addTabLayout)
      throw new std::runtime_error("Found a Tab without a direct TabLayout parent");
      break;

   case HORIZONTAL_SPACE:
      addVisibilityKeys(addSpace("HorizontalSpace", parent), root);
      break;
   case VERTICAL_SPACE:
      addVisibilityKeys(addSpace("VerticalSpace", parent), root);
      break;
   case VERTICAL_EXPANDING_SPACE:
      addVisibilityKeys(addSpace("VerticalExpandingSpace", parent), root);
      break;
   case HORIZONTAL_EXPANDING_SPACE:
      addVisibilityKeys(addSpace("HorizontalExpandingSpace", parent), root);
      break;

   case POPUP_BUTTON:
      addVisibilityKeys(addPopupButton(parent, dynamic_cast<model::gui::PopupButton*>(root)), root);
      break;

   }


}

xmlNodePtr modelxml::XMLBuilder::addExposedModelGUIElement(model::gui::KeyValue *element,
                                                   std::string type, xmlNodePtr parent)
{


   auto xmlElement = xmlNewChild(parent, 0, BAD_CAST type.c_str(), 0);
   addElementKeys(xmlElement, element);

   return xmlElement;
}

xmlNodePtr modelxml::XMLBuilder::addElementGroup(model::gui::ElementGroup *element,
                                                            xmlNodePtr parent)
{


   using namespace model::gui;
   auto xmlElement = xmlNewChild(parent, 0, BAD_CAST "ElementGroup", 0);
   xmlSetProp(xmlElement, BAD_CAST "key", BAD_CAST element->key().c_str());

   buildGUILayout(element->child(), xmlElement);


   xmlSetProp(xmlElement, BAD_CAST "showValue", BAD_CAST (element->showValue() ?
                                                                "1" : "0"));


   return xmlElement;
}

xmlNodePtr modelxml::XMLBuilder::addLayout(std::string type,
                                               model::gui::Container1D<model::gui::Element> *layout,
                                               xmlNodePtr parent)
{


   auto element = xmlNewChild(parent, 0, BAD_CAST type.c_str(), 0);
   for(size_t i = 0; i < layout->children(); i++)
   {
      buildGUILayout(layout->child(i), element);
   }
   return element;
}

xmlNodePtr modelxml::XMLBuilder::addGridLayout(std::string type, model::gui::Grid *grid, xmlNodePtr parent)
{


   auto xmlGrid = xmlNewChild(parent, 0, BAD_CAST type.c_str(), 0);
   for(size_t i = 0; i < grid->height(); i++)
   {
      auto row = xmlNewChild(xmlGrid, 0, BAD_CAST "Row", 0);
      for(size_t j = 0; j < grid->width(); j++)
      {
         auto cell = xmlNewChild(row, 0, BAD_CAST "Cell", 0);
         if(grid->child(i,j) != NULL)
         {
            buildGUILayout(grid->child(i,j), cell);
         }
      }
   }


   return xmlGrid;
}

xmlNodePtr modelxml::XMLBuilder::addTabLayout(std::string type, model::gui::TabLayout *tabLayout, xmlNodePtr parent)
{
using namespace model::gui;
   auto element = xmlNewChild(parent, 0, BAD_CAST type.c_str(), 0);



   for(size_t i = 0; i < tabLayout->children(); i++)
   {
      Tab* child = tabLayout->child(i);
      auto tab = xmlNewChild(element, 0, BAD_CAST "Tab", 0);
      addElementKeys(tab, child);
      buildGUILayout(child->child(), tab);
   }
   return element;
}

xmlNodePtr modelxml::XMLBuilder::addCanvas(model::gui::Canvas* element,
                                               xmlNodePtr parent)
{


   auto xmlElement = addExposedModelGUIElement(element, "Canvas", parent);




   xmlSetProp(xmlElement, BAD_CAST "renderlistKey", BAD_CAST element->renderlistKey().c_str());


   xmlSetProp(xmlElement, BAD_CAST "boundingboxKey", BAD_CAST element->boundingBoxKey().c_str());
   xmlSetProp(xmlElement, BAD_CAST "resetViewKey", BAD_CAST element->resetViewKey().c_str());
   return xmlElement;
}

xmlNodePtr modelxml::XMLBuilder::addHorizontalLayout(model::gui::HorizontalLayout *layout, xmlNodePtr parent)
{


   auto xmlElement = addLayout("HorizontalLayout", layout, parent);
   return xmlElement;

}

xmlNodePtr modelxml::XMLBuilder::addVerticalLayout(model::gui::VerticalLayout *layout, xmlNodePtr parent)
{


   auto xmlElement = addLayout("VerticalLayout", layout, parent);
   return xmlElement;

}

xmlNodePtr modelxml::XMLBuilder::addSpace(std::string type, xmlNodePtr parent)
{
   return xmlNewChild(parent, 0, BAD_CAST type.c_str(), 0);
}

xmlNodePtr modelxml::XMLBuilder::addVisibilityKeys(xmlNodePtr xmlElement, model::gui::Element* element)
{
   if(element->enabledKey() != "")
   {
      xmlSetProp(xmlElement, BAD_CAST "enabledKey", BAD_CAST element->enabledKey().c_str());
      xmlSetProp(xmlElement, BAD_CAST "enabledInverted",
                 BAD_CAST boost::lexical_cast<std::string>(element->enabledKey()).c_str());
   }
   if(element->visibilityKey() != "")
   {
      xmlSetProp(xmlElement, BAD_CAST "visibilityKey", BAD_CAST element->visibilityKey().c_str());
      xmlSetProp(xmlElement, BAD_CAST "visibilityKeyInverted",
                 BAD_CAST boost::lexical_cast<std::string>(element->visibilityInverted()).c_str());

   }
   return xmlElement;
}

xmlNodePtr modelxml::XMLBuilder::addElementKeys(xmlNodePtr xmlElementPtr,
                                              model::gui::KeyValue *element)
{
   xmlSetProp(xmlElementPtr, BAD_CAST "key", BAD_CAST element->key().c_str());
   xmlSetProp(xmlElementPtr, BAD_CAST "showValue",
              BAD_CAST boost::lexical_cast<std::string>(element->showValue()).c_str());
   return xmlElementPtr;
}

xmlNodePtr modelxml::XMLBuilder::addPopupButton(xmlNodePtr parent, model::gui::PopupButton *button)
{
   auto buttonXml = addExposedModelGUIElement(button, "PopupButton", parent);


   buildGUILayout(button->child(), buttonXml);

   return buttonXml;

}


}

























