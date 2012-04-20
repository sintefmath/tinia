#include "tinia/policy/Policy.hpp"

#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <cstring>
#include <iterator>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "tinia/policy/Viewer.hpp"

#include "tinia/policy/utils.hpp"

namespace tinia {
namespace policy {

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::for_each;
using std::pair;

Policy::Policy() : revisionNumber( 1 ), m_gui(NULL)
{
}

void
Policy::incrementRevisionNumber(ElementData &updatedElement) {
   updatedElement.setRevisionNumber(revisionNumber);
   ++revisionNumber;
}





void Policy::updateElementFromString( const std::string &key, const std::string &value )
{
   ElementData data;
   ElementData before;

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
void Policy::printCurrentState() {
   scoped_lock(m_selfMutex);
   cout << "----------------- Current stateHash ---------------" << endl;
   std::for_each(stateHash.begin(), stateHash.end(),
                 [this]( std::pair<std::string, ElementData> kv ) {
                 cout << kv.first << " (" << kv.second.getXSDType() << ") : " << endl;
         kv.second.print();
}
);
cout << "---------------------------------------------------" << endl;
}


// Note the similarity with updateElementFromString. This should be possible to combine...?!
void Policy::updateElementFromPTree( const std::string &key, const StringStringPTree &value )
{
      scoped_lock lock(m_selfMutex);
      const auto it = stateHash.find( key );
      if ( it == stateHash.end() ) {
         throw std::runtime_error( "Trying to update element " + key + " but it is not yet added" );
      }
      incrementRevisionNumber(it->second);
      // We jump right past the root, since that is not stored in the ElementData's propertyTree, evidently...
      // pt_print("argument to Policy::updateElementFromPTree", value.begin()->second);
      // printCurrentState();
      stateHash[key].setPropertyTreeValue( value.begin()->second );
      ElementData data  =stateHash[key];
      lock.unlock();

      // For now we don't add checking for complex types
      fireStateElementModified(key, data);
}


void
Policy::addAnnotationHelper( std::string key, std::unordered_map<std::string, std::string> & annotationMap ) {
   ElementData data;
   {
      scoped_lock(m_selfMutex);
      auto it = stateHash.find( key );
      if ( it == stateHash.end() ) {
         throw std::runtime_error( "Trying to set annotation for element " + key + " but it is not in the policy." );
      }

      auto& elementData = it->second;
      elementData.setAnnotation( annotationMap );

      // Copy
      data = elementData;
   }
   fireStateSchemaElementModified(key, data);
}


void
Policy::addMatrixElement( std::string matrixName, float const* matrixData ) {

   scoped_lock lock(m_selfMutex);
   addElementErrorChecking( matrixName );

   addMatrixHelper( matrixName, matrixData );
   ElementData data = stateHash[matrixName];
   lock.unlock();
   fireStateSchemaElementAdded(matrixName, data);
}

void
Policy::updateMatrixValue( std::string key, const float* matrixData ) {
   scoped_lock lock(m_selfMutex);
   auto it = stateHash.find( key );
   if ( it == stateHash.end() ) {
      throw std::runtime_error( "Trying to update matrix " + key + " but it is not in the policy. " );
   }

   auto& elementData = it->second;
   if ( elementData.getLength() != ElementData::MATRIX_LENGTH ) {
      throw std::runtime_error( "Trying to get element " + key + " as a matrix, but it is not defined as one" );
   }
   ElementData before = elementData;
   addMatrixHelper( key, matrixData );
   ElementData data = stateHash[key];
   lock.unlock();
   if(data.getStringValue() != before.getStringValue())
   {
      fireStateElementModified(key, data);
   }
}

void
Policy::addMatrixHelper(std::string key, const float *matrixData) {
   auto elementData = elementFactory.createMatrixElement( matrixData );
   incrementRevisionNumber( elementData );
   stateHash[key] = elementData;
}


void
Policy::addAnnotation( std::string key, std::string annotation ) {
   std::unordered_map<std::string, std::string> annotationMap;
   annotationMap["en"]= annotation;
   addAnnotationHelper( key, annotationMap );
}







void
Policy::removeElement( std::string key ) {

   scoped_lock lock(m_selfMutex);
   auto it = stateHash.find( key );
   if ( it == stateHash.end() ) {
      throw std::runtime_error( key + " is not in the policy." );
   }
   ElementData data = it->second;
   stateHash.erase( it );
   lock.unlock();
   fireStateSchemaElementRemoved(key, data);
}


bool
Policy::hasElement( std::string elementName ) const {
   scoped_lock(m_selfMutex);
   return stateHash.find( elementName) != stateHash.end();
}


string
Policy::getElementValueAsString( string key)
{
   scoped_lock(m_selfMutex);
   const auto key_val = stateHash.find( key );
   if ( key_val == stateHash.end() )
      throw std::runtime_error( "Trying to get element " + key + " but it is not in the policy" );
   return key_val->second.getStringValue();
}

void
Policy::getMatrixValue( std::string key, float* matrixData ) const {
   // Locking whole method
   scoped_lock(m_selfMutex);

   const auto it= stateHash.find( key );
   if ( it == stateHash.end() )
      throw std::runtime_error( "Trying to get element " + key + " but it is not in the policy" );

   auto& elementData = it->second;
   if ( elementData.getLength() != ElementData::MATRIX_LENGTH ) {
      throw std::runtime_error( "Trying to get element " + key + " as a matrix, but it is not defined as one" );
   }

   elementFactory.createMatrix( elementData, matrixData );

}





// Updates to send *to* the client, version for non-xml-capable jobs/observers.
void Policy::getPolicyUpdate(std::vector< std::pair<std::string, ElementData> > &updatedElements, const unsigned has_revision ) const
{
   scoped_lock(m_selfMutex);
   updatedElements.resize(0);
   std::for_each(stateHash.begin(), stateHash.end(),
                 [has_revision /* , this */, &updatedElements ]( std::pair<std::string, ElementData> kv ) {
                 ElementData& elementData = kv.second;
         // printf("Current rev=%d, rev_0=%d, element(%s) has rev=%d\n", revisionNumber, has_revision, kv.first.c_str(), elementData.getRevisionNumber());
         if ( elementData.getRevisionNumber() >= has_revision )
         // printf("going to add (%s, %s)\n", kv.first.c_str(), elementData.getStringValue().c_str() );
         updatedElements.push_back( std::make_pair( kv.first, elementData ) );

}
);
}




void
Policy::addElementErrorChecking( std::string key ) const {
   if ( stateHash.find( key ) != stateHash.end() ) {
      throw std::runtime_error( "Trying to add element " + key + " to the policy, but it is already defined. " );
   }
}

void
Policy::updateStateHash( std::string key,  ElementData& elementData ) {
   incrementRevisionNumber( elementData );
   stateHash[key] = elementData;
}

}

void policy::Policy::addStateSchemaListener(

      policy::StateSchemaListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateSchemaListenerHandler.addStateSchemaListener(listener);
}

void policy::Policy::removeStateSchemaListener(
      policy::StateSchemaListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateSchemaListenerHandler.removeStateSchemaListener(listener);
}

void policy::Policy::addStateSchemaListener(std::string key,

      policy::StateSchemaListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateSchemaListenerHandler.addStateSchemaListener(key, listener);
}

void policy::Policy::removeStateSchemaListener(std::string key,
      policy::StateSchemaListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateSchemaListenerHandler.removeStateSchemaListener(key, listener);
}

void policy::Policy::addStateListener(policy::StateListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateListenerHandler.addStateListener(listener);
}


void policy::Policy::removeStateListener(policy::StateListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateListenerHandler.removeStateListener(listener);
}

void policy::Policy::addStateListener(std::string key, policy::StateListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateListenerHandler.addStateListener(key,    listener);
}

void policy::Policy::removeStateListener(std::string key, policy::StateListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateListenerHandler.removeStateListener(key, listener);
}

void policy::Policy::getStateUpdate(
      std::vector<policy::StateElement> &updatedElements,
      const unsigned int has_revision)
{
   scoped_lock(m_selfMutex);
   for(auto it = stateHash.begin(); it!=stateHash.end(); it++)
   {
      ElementData& data = it->second;
      if(data.getRevisionNumber()>=has_revision)
      {
         updatedElements.push_back(StateElement(it->first, data));
      }
   }
}

void policy::Policy::getStateSchemaUpdate(
      std::vector<policy::StateSchemaElement> &updatedElements,
      const unsigned int has_revision)
{
   scoped_lock(m_selfMutex);
   for(auto it = stateHash.begin(); it!=stateHash.end(); it++)
   {
      ElementData& data = it->second;
      if(data.getRevisionNumber()>=has_revision)
      {
         updatedElements.push_back(StateSchemaElement(it->first, data));
      }
   }
}


void policy::Policy::getFullStateSchema(std::vector<policy::StateSchemaElement> &stateSchemaElements)
{

   scoped_lock(m_selfMutex);

   for(auto it = stateHash.begin(); it != stateHash.end(); it++)
   {
      std::string key(it->first.c_str());
      ElementData data = it->second;
      stateSchemaElements.push_back(StateSchemaElement(key, data));
   }
}

void policy::Policy::getFullState(std::vector<policy::StateElement> &stateElements)
{
   scoped_lock(m_selfMutex);
   for(auto it = stateHash.begin(); it != stateHash.end(); it++)
   {
      stateElements.push_back(StateElement(it->first, ElementData(it->second)));
   }
}

void policy::Policy::fireStateSchemaElementAdded(std::string key,
                                                       const policy::ElementData &data)
{
   scoped_lock(m_listenerHandlersMutex);
   policy::StateSchemaElement element(key, data);

   m_stateSchemaListenerHandler.fireStateSchemaElementAdded(&element);
}

void policy::Policy::fireStateSchemaElementRemoved(std::string key,
                                                         const policy::ElementData &data)
{
   scoped_lock(m_listenerHandlersMutex);
   policy::StateSchemaElement element(key, data);
   m_stateSchemaListenerHandler.fireStateSchemaElementRemoved(&element);
}

void policy::Policy::fireStateSchemaElementModified(std::string key,
                                                          const policy::ElementData &data)
{
   scoped_lock(m_listenerHandlersMutex);
   policy::StateSchemaElement element(key, data);
   m_stateSchemaListenerHandler.fireStateSchemaElementModified(&element);
}

void policy::Policy::fireStateElementModified(std::string key,
                                                    const policy::ElementData &data)
{
   scoped_lock(m_listenerHandlersMutex);
   policy::StateElement element(key, data);
   m_stateListenerHandler.fireStateElementModified(&element);
}

/**
  \todo Make this dependent on device
  */
void policy::Policy::setGUILayout(policy::gui::Element *rootElement, int device)
{
   m_gui = rootElement;
}

policy::gui::Element* policy::Policy::getGUILayout(policy::gui::Device device)
{
   if(m_gui == NULL)
   {
      makeDefaultGUILayout();
   }
   return m_gui;
}

std::unordered_set<std::string> policy::Policy::getRestrictionSet(std::string key)
{
   // This returns a deep copy to avoid any possible threading-problems.
   scoped_lock(m_selfMutex);
   std::unordered_set<std::string> destinationSet;
   auto sourceSet = stateHash[key].getEnumerationSet();
   for(auto it = sourceSet.begin(); it!=sourceSet.end(); it++)
   {
      destinationSet.insert(it->c_str());
   }
   return destinationSet;
}

bool policy::Policy::emptyRestrictionSet(std::string key)
{
   scoped_lock(m_selfMutex);
   // The whole method locks the policy. This could be optimized, but leaving it
   // like this for now.

   const auto it = stateHash.find( key );

   if ( it == stateHash.end() ) {
      throw std::runtime_error( "Trying to get element " + key + " but it is not in the policy" );
   }

   auto& elementData = it->second;
   return elementData.emptyRestrictionSet();
}

void policy::Policy::releaseAllListeners()
{
}

policy::StateSchemaElement policy::Policy::getStateSchemaElement(std::string key)
{
   // The whole method locks the policy. This could be optimized, but leaving it
   // like this for now.
   scoped_lock(m_selfMutex);

   const auto it = stateHash.find( key );

   if ( it == stateHash.end() ) {
      throw std::runtime_error( "Trying to get element " + key + " but it is not in the policy" );
   }

   auto& elementData = it->second;
   return StateSchemaElement(key, elementData);
}

std::string policy::Policy::getElementMaxConstraint(std::string key) const
{
   // The whole method locks the policy. This could be optimized, but leaving it
   // like this for now.
   scoped_lock(m_selfMutex);

   const auto it = stateHash.find( key );

   if ( it == stateHash.end() ) {
      throw std::runtime_error( "Trying to get element " + key + " but it is not in the policy" );
   }

   auto& elementData = it->second;
   return elementData.getMaxConstraint();
}

std::string policy::Policy::getElementMinConstraint(std::string key) const
{
   // The whole method locks the policy. This could be optimized, but leaving it
   // like this for now.
   scoped_lock(m_selfMutex);

   const auto it = stateHash.find( key );

   if ( it == stateHash.end() ) {
      throw std::runtime_error( "Trying to get element " + key + " but it is not in the policy" );
   }

   auto& elementData = it->second;
   return elementData.getMinConstraint();
}

void policy::Policy::makeDefaultGUILayout()
{
   using namespace gui;
   // Make something simple:
     Grid* grid = new Grid(stateHash.size(), 1);
   int index = 0;
   for(auto it = stateHash.begin(); it != stateHash.end(); it++)
   {
      ElementData& data = it->second;
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

void policy::Policy::holdStateEvents()
{
   m_stateListenerHandler.holdEvents();
}

void policy::Policy::releaseStateEvents()
{
   m_stateListenerHandler.releaseEvents();
}

policy::Policy::mutex_type& policy::Policy::getPolicyMutex()
{
   return m_selfMutex;
}

} // of namespace tinia


