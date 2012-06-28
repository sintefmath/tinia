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

#include "tinia/model/ExposedModel.hpp"

#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <cstring>
#include <iterator>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "tinia/model/Viewer.hpp"

#include "tinia/model/impl/utils.hpp"

namespace tinia {
namespace model {

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::for_each;
using std::pair;

ExposedModel::ExposedModel() : revisionNumber( 1 ), m_gui(NULL)
{
}

void
ExposedModel::incrementRevisionNumber(impl::ElementData &updatedElement) {
   updatedElement.setRevisionNumber(revisionNumber);
   ++revisionNumber;
}





void ExposedModel::updateElementFromString( const std::string &key, const std::string &value )
{
   impl::ElementData data;
   impl::ElementData before;

   {// Lock scope
      scoped_lock(m_selfMutex);
      const auto it = stateHash.find( key );
      if ( it == stateHash.end() ) {
         throw std::runtime_error( "Trying to update element " + key + " but it is not yet added" );
      }
      before = it->second;
      incrementRevisionNumber(it->second);
      it->second.setStringValue( value );
      data = it->second;
   }

   if(before.getStringValue() != data.getStringValue())
   {
      fireStateElementModified(key, data);
   }
}


// For debugging purposes
void ExposedModel::printCurrentState() {
   scoped_lock(m_selfMutex);
   cout << "----------------- Current stateHash ---------------" << endl;
   std::for_each(stateHash.begin(), stateHash.end(),
                 [this]( std::pair<std::string, impl::ElementData> kv ) {
                 cout << kv.first << " (" << kv.second.getXSDType() << ") : " << endl;
         kv.second.print();
}
);
cout << "---------------------------------------------------" << endl;
}


// Note the similarity with updateElementFromString. This should be possible to combine...?!
void ExposedModel::updateElementFromPTree( const std::string &key, const StringStringPTree &value )
{
      scoped_lock lock(m_selfMutex);
      const auto it = stateHash.find( key );
      if ( it == stateHash.end() ) {
         throw std::runtime_error( "Trying to update element " + key + " but it is not yet added" );
      }
      incrementRevisionNumber(it->second);
      // We jump right past the root, since that is not stored in the impl::ElementData's propertyTree, evidently...
      // pt_print("argument to ExposedModel::updateElementFromPTree", value.begin()->second);
      // printCurrentState();
      stateHash[key].setPropertyTreeValue( value.begin()->second );
      impl::ElementData data  =stateHash[key];
      lock.unlock();

      // For now we don't add checking for complex types
      fireStateElementModified(key, data);
}


void
ExposedModel::addAnnotationHelper( std::string key, std::unordered_map<std::string, std::string> & annotationMap ) {
   impl::ElementData data;
   {
      scoped_lock(m_selfMutex);
      auto it = stateHash.find( key );
      if ( it == stateHash.end() ) {
         throw std::runtime_error( "Trying to set annotation for element " + key + " but it is not in the model." );
      }

      auto& elementData = it->second;
      elementData.setAnnotation( annotationMap );

      // Copy
      data = elementData;
   }
   fireStateSchemaElementModified(key, data);
}


void
ExposedModel::addMatrixElement( std::string matrixName, float const* matrixData ) {

   scoped_lock lock(m_selfMutex);
   addElementErrorChecking( matrixName );

   addMatrixHelper( matrixName, matrixData );
   impl::ElementData data = stateHash[matrixName];
   lock.unlock();
   fireStateSchemaElementAdded(matrixName, data);
}




void
ExposedModel::updateMatrixValue( std::string key, const float* matrixData ) {
   scoped_lock lock(m_selfMutex);
   auto it = stateHash.find( key );
   if ( it == stateHash.end() ) {
      throw std::runtime_error( "Trying to update matrix " + key + " but it is not in the model. " );
   }

   auto& elementData = it->second;
   if ( elementData.getLength() != impl::ElementData::MATRIX_LENGTH ) {
      throw std::runtime_error( "Trying to get element " + key + " as a matrix, but it is not defined as one" );
   }
   impl::ElementData before = elementData;
   addMatrixHelper( key, matrixData );
   impl::ElementData data = stateHash[key];
   lock.unlock();
   if(data.getStringValue() != before.getStringValue())
   {
      fireStateElementModified(key, data);
   }
}

void
ExposedModel::addMatrixHelper(std::string key, const float *matrixData) {
   auto elementData = elementFactory.createMatrixElement( matrixData );
   incrementRevisionNumber( elementData );
   stateHash[key] = elementData;
}


void
ExposedModel::addAnnotation( std::string key, std::string annotation ) {
   std::unordered_map<std::string, std::string> annotationMap;
   annotationMap["en"]= annotation;
   addAnnotationHelper( key, annotationMap );
}







void
ExposedModel::removeElement( std::string key ) {

   scoped_lock lock(m_selfMutex);
   auto it = stateHash.find( key );
   if ( it == stateHash.end() ) {
      throw std::runtime_error( key + " is not in the model." );
   }
   impl::ElementData data = it->second;
   stateHash.erase( it );
   lock.unlock();
   fireStateSchemaElementRemoved(key, data);
}


bool
ExposedModel::hasElement( std::string elementName ) const {
   scoped_lock(m_selfMutex);
   return stateHash.find( elementName) != stateHash.end();
}


string
ExposedModel::getElementValueAsString( string key)
{
   scoped_lock(m_selfMutex);
   const auto key_val = stateHash.find( key );
   if ( key_val == stateHash.end() )
      throw std::runtime_error( "Trying to get element " + key + " but it is not in the model" );
   return key_val->second.getStringValue();
}

void
ExposedModel::getMatrixValue( std::string key, float* matrixData ) const {
   // Locking whole method
   scoped_lock(m_selfMutex);

   const auto it= stateHash.find( key );
   if ( it == stateHash.end() )
      throw std::runtime_error( "Trying to get element " + key + " but it is not in the model" );

   auto& elementData = it->second;
   if ( elementData.getLength() != impl::ElementData::MATRIX_LENGTH ) {
      throw std::runtime_error( "Trying to get element " + key + " as a matrix, but it is not defined as one" );
   }

   elementFactory.createMatrix( elementData, matrixData );

}





// Updates to send *to* the client, version for non-xml-capable jobs/observers.
void ExposedModel::getExposedModelUpdate(std::vector< std::pair<std::string, impl::ElementData> > &updatedElements, const unsigned has_revision ) const
{
   scoped_lock(m_selfMutex);
   updatedElements.resize(0);
   std::for_each(stateHash.begin(), stateHash.end(),
                 [has_revision /* , this */, &updatedElements ]( std::pair<std::string, impl::ElementData> kv ) {
                 impl::ElementData& elementData = kv.second;
         // printf("Current rev=%d, rev_0=%d, element(%s) has rev=%d\n", revisionNumber, has_revision, kv.first.c_str(), elementData.getRevisionNumber());
         if ( elementData.getRevisionNumber() >= has_revision )
         // printf("going to add (%s, %s)\n", kv.first.c_str(), elementData.getStringValue().c_str() );
         updatedElements.push_back( std::make_pair( kv.first, elementData ) );

}
);
}




void
ExposedModel::addElementErrorChecking( std::string key ) const {
   if ( stateHash.find( key ) != stateHash.end() ) {
      throw std::runtime_error( "Trying to add element " + key + " to the model, but it is already defined. " );
   }
}

void
ExposedModel::updateStateHash( std::string key,  impl::ElementData& elementData ) {
   incrementRevisionNumber( elementData );
   stateHash[key] = elementData;
}

}

void model::ExposedModel::addStateSchemaListener(

      model::StateSchemaListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateSchemaListenerHandler.addStateSchemaListener(listener);
}

void model::ExposedModel::removeStateSchemaListener(
      model::StateSchemaListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateSchemaListenerHandler.removeStateSchemaListener(listener);
}

void model::ExposedModel::addStateSchemaListener(std::string key,

      model::StateSchemaListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateSchemaListenerHandler.addStateSchemaListener(key, listener);
}

void model::ExposedModel::removeStateSchemaListener(std::string key,
      model::StateSchemaListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateSchemaListenerHandler.removeStateSchemaListener(key, listener);
}

void model::ExposedModel::addStateListener(model::StateListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateListenerHandler.addStateListener(listener);
}


void model::ExposedModel::removeStateListener(model::StateListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateListenerHandler.removeStateListener(listener);
}

void model::ExposedModel::addStateListener(std::string key, model::StateListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateListenerHandler.addStateListener(key,    listener);
}

void model::ExposedModel::removeStateListener(std::string key, model::StateListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateListenerHandler.removeStateListener(key, listener);
}

void model::ExposedModel::getStateUpdate(
      std::vector<model::StateElement> &updatedElements,
      const unsigned int has_revision)
{
   scoped_lock(m_selfMutex);
   for(auto it = stateHash.begin(); it!=stateHash.end(); it++)
   {
      impl::ElementData& data = it->second;
      if(data.getRevisionNumber()>=has_revision)
      {
         updatedElements.push_back(StateElement(it->first, data));
      }
   }
}

void model::ExposedModel::getStateSchemaUpdate(
      std::vector<model::StateSchemaElement> &updatedElements,
      const unsigned int has_revision)
{
   scoped_lock(m_selfMutex);
   for(auto it = stateHash.begin(); it!=stateHash.end(); it++)
   {
      impl::ElementData& data = it->second;
      if(data.getRevisionNumber()>=has_revision)
      {
         updatedElements.push_back(StateSchemaElement(it->first, data));
      }
   }
}


void model::ExposedModel::getFullStateSchema(std::vector<model::StateSchemaElement> &stateSchemaElements)
{

   scoped_lock(m_selfMutex);

   for(auto it = stateHash.begin(); it != stateHash.end(); it++)
   {
      std::string key(it->first.c_str());
      impl::ElementData data = it->second;
      stateSchemaElements.push_back(StateSchemaElement(key, data));
   }
}

void model::ExposedModel::getFullState(std::vector<model::StateElement> &stateElements)
{
   scoped_lock(m_selfMutex);
   for(auto it = stateHash.begin(); it != stateHash.end(); it++)
   {
      stateElements.push_back(StateElement(it->first, impl::ElementData(it->second)));
   }
}

void model::ExposedModel::fireStateSchemaElementAdded(std::string key,
                                                       const model::impl::ElementData &data)
{
   scoped_lock(m_listenerHandlersMutex);
   model::StateSchemaElement element(key, data);

   m_stateSchemaListenerHandler.fireStateSchemaElementAdded(&element);
}

void model::ExposedModel::fireStateSchemaElementRemoved(std::string key,
                                                         const model::impl::ElementData &data)
{
   scoped_lock(m_listenerHandlersMutex);
   model::StateSchemaElement element(key, data);
   m_stateSchemaListenerHandler.fireStateSchemaElementRemoved(&element);
}

void model::ExposedModel::fireStateSchemaElementModified(std::string key,
                                                          const model::impl::ElementData &data)
{
   scoped_lock(m_listenerHandlersMutex);
   model::StateSchemaElement element(key, data);
   m_stateSchemaListenerHandler.fireStateSchemaElementModified(&element);
}

void model::ExposedModel::fireStateElementModified(std::string key,
                                                    const model::impl::ElementData &data)
{
   scoped_lock(m_listenerHandlersMutex);
   model::StateElement element(key, data);
   m_stateListenerHandler.fireStateElementModified(&element);
}

/**
  \todo Make this dependent on device
  */
void model::ExposedModel::setGUILayout(model::gui::Element *rootElement, int device)
{
   m_gui = rootElement;
}

model::gui::Element* model::ExposedModel::getGUILayout(model::gui::Device device)
{
   if(m_gui == NULL)
   {
      makeDefaultGUILayout();
   }
   return m_gui;
}

std::set<std::string> model::ExposedModel::getRestrictionSet(std::string key)
{
   // This returns a deep copy to avoid any possible threading-problems.
   scoped_lock(m_selfMutex);
   std::set<std::string> destinationSet;
   auto sourceSet = stateHash[key].getEnumerationSet();
   for(auto it = sourceSet.begin(); it!=sourceSet.end(); it++)
   {
      destinationSet.insert(it->c_str());
   }
   return destinationSet;
}

bool model::ExposedModel::emptyRestrictionSet(std::string key)
{
   scoped_lock(m_selfMutex);
   // The whole method locks the model. This could be optimized, but leaving it
   // like this for now.

   const auto it = stateHash.find( key );

   if ( it == stateHash.end() ) {
      throw std::runtime_error( "Trying to get element " + key + " but it is not in the model" );
   }

   auto& elementData = it->second;
   return elementData.emptyRestrictionSet();
}

void model::ExposedModel::releaseAllListeners()
{
}

model::StateSchemaElement model::ExposedModel::getStateSchemaElement(std::string key)
{
   // The whole method locks the model. This could be optimized, but leaving it
   // like this for now.
   scoped_lock(m_selfMutex);

   const auto it = stateHash.find( key );

   if ( it == stateHash.end() ) {
      throw std::runtime_error( "Trying to get element " + key + " but it is not in the model" );
   }

   auto& elementData = it->second;
   return StateSchemaElement(key, elementData);
}

std::string model::ExposedModel::getElementMaxConstraint(std::string key) const
{
   // The whole method locks the model. This could be optimized, but leaving it
   // like this for now.
   scoped_lock(m_selfMutex);

   const auto it = stateHash.find( key );

   if ( it == stateHash.end() ) {
      throw std::runtime_error( "Trying to get element " + key + " but it is not in the model" );
   }

   auto& elementData = it->second;
   return elementData.getMaxConstraint();
}

std::string model::ExposedModel::getElementMinConstraint(std::string key) const
{
   // The whole method locks the model. This could be optimized, but leaving it
   // like this for now.
   scoped_lock(m_selfMutex);

   const auto it = stateHash.find( key );

   if ( it == stateHash.end() ) {
      throw std::runtime_error( "Trying to get element " + key + " but it is not in the model" );
   }

   auto& elementData = it->second;
   return elementData.getMinConstraint();
}

void model::ExposedModel::makeDefaultGUILayout()
{
   using namespace gui;
   // Make something simple:
     Grid* grid = new Grid(stateHash.size(), 1);
   int index = 0;
   for(auto it = stateHash.begin(); it != stateHash.end(); it++)
   {
      impl::ElementData& data = it->second;
      std::string t = data.getWidgetType();
      if(t=="textinput")
      {
         grid->setChild(index++, 0, new TextInput(it->first));
      }
      else if(t=="select")
      {
         grid->setChild(index++, 0, new TextInput(it->first));
      }
      else if(t == "canvas")
      {
         grid->setChild(index++, 0, new Canvas(it->first));
      }
   }
   m_gui = grid;
}

void model::ExposedModel::holdStateEvents()
{
   m_stateListenerHandler.holdEvents();
}

void model::ExposedModel::releaseStateEvents()
{
   m_stateListenerHandler.releaseEvents();
}

model::ExposedModel::mutex_type& model::ExposedModel::getExposedModelMutex()
{
   return m_selfMutex;
}

} // of namespace tinia


