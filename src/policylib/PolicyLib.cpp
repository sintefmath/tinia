#include "policylib/PolicyLib.hpp"

#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <cstring>
#include <iterator>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "policylib/Viewer.hpp"

#include "policylib/utils.hpp"

namespace policylib {

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::for_each;
using std::pair;

PolicyLib::PolicyLib() : revisionNumber( 1 ), m_gui(NULL)
{
}

void
PolicyLib::incrementRevisionNumber(ElementData &updatedElement) {
   updatedElement.setRevisionNumber(revisionNumber);
   ++revisionNumber;
}





void PolicyLib::updateElementFromString( const std::string &key, const std::string &value )
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
void PolicyLib::printCurrentState() {
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
void PolicyLib::updateElementFromPTree( const std::string &key, const StringStringPTree &value )
{
      scoped_lock lock(m_selfMutex);
      const auto it = stateHash.find( key );
      if ( it == stateHash.end() ) {
         throw std::runtime_error( "Trying to update element " + key + " but it is not yet added" );
      }
      incrementRevisionNumber(it->second);
      // We jump right past the root, since that is not stored in the ElementData's propertyTree, evidently...
      // pt_print("argument to PolicyLib::updateElementFromPTree", value.begin()->second);
      // printCurrentState();
      stateHash[key].setPropertyTreeValue( value.begin()->second );
      ElementData data  =stateHash[key];
      lock.unlock();

      // For now we don't add checking for complex types
      fireStateElementModified(key, data);
}


void
PolicyLib::addAnnotationHelper( std::string key, std::unordered_map<std::string, std::string> & annotationMap ) {
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
PolicyLib::addMatrixElement( std::string matrixName, float const* matrixData ) {

   scoped_lock lock(m_selfMutex);
   addElementErrorChecking( matrixName );

   addMatrixHelper( matrixName, matrixData );
   ElementData data = stateHash[matrixName];
   lock.unlock();
   fireStateSchemaElementAdded(matrixName, data);
}

void
PolicyLib::updateMatrixValue( std::string key, const float* matrixData ) {
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
PolicyLib::addMatrixHelper(std::string key, const float *matrixData) {
   auto elementData = elementFactory.createMatrixElement( matrixData );
   incrementRevisionNumber( elementData );
   stateHash[key] = elementData;
}


void
PolicyLib::addAnnotation( std::string key, std::string annotation ) {
   std::unordered_map<std::string, std::string> annotationMap;
   annotationMap["en"]= annotation;
   addAnnotationHelper( key, annotationMap );
}







void
PolicyLib::removeElement( std::string key ) {

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
PolicyLib::hasElement( std::string elementName ) const {
   scoped_lock(m_selfMutex);
   return stateHash.find( elementName) != stateHash.end();
}


string
PolicyLib::getElementValueAsString( string key)
{
   scoped_lock(m_selfMutex);
   const auto key_val = stateHash.find( key );
   if ( key_val == stateHash.end() )
      throw std::runtime_error( "Trying to get element " + key + " but it is not in the policy" );
   return key_val->second.getStringValue();
}

void
PolicyLib::getMatrixValue( std::string key, float* matrixData ) const {
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
void PolicyLib::getPolicyUpdate(std::vector< std::pair<std::string, ElementData> > &updatedElements, const unsigned has_revision ) const
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
PolicyLib::addElementErrorChecking( std::string key ) const {
   if ( stateHash.find( key ) != stateHash.end() ) {
      throw std::runtime_error( "Trying to add element " + key + " to the policy, but it is already defined. " );
   }
}

void
PolicyLib::updateStateHash( std::string key,  ElementData& elementData ) {
   incrementRevisionNumber( elementData );
   stateHash[key] = elementData;
}

}

void policylib::PolicyLib::addStateSchemaListener(

      policylib::StateSchemaListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateSchemaListenerHandler.addStateSchemaListener(listener);
}

void policylib::PolicyLib::removeStateSchemaListener(
      policylib::StateSchemaListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateSchemaListenerHandler.removeStateSchemaListener(listener);
}

void policylib::PolicyLib::addStateSchemaListener(std::string key,

      policylib::StateSchemaListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateSchemaListenerHandler.addStateSchemaListener(key, listener);
}

void policylib::PolicyLib::removeStateSchemaListener(std::string key,
      policylib::StateSchemaListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateSchemaListenerHandler.removeStateSchemaListener(key, listener);
}

void policylib::PolicyLib::addStateListener(policylib::StateListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateListenerHandler.addStateListener(listener);
}


void policylib::PolicyLib::removeStateListener(policylib::StateListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateListenerHandler.removeStateListener(listener);
}

void policylib::PolicyLib::addStateListener(std::string key, policylib::StateListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateListenerHandler.addStateListener(key,    listener);
}

void policylib::PolicyLib::removeStateListener(std::string key, policylib::StateListener *listener)
{
   scoped_lock(m_listenerHandlersMutex);
   m_stateListenerHandler.removeStateListener(key, listener);
}

void policylib::PolicyLib::getStateUpdate(
      std::vector<policylib::StateElement> &updatedElements,
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

void policylib::PolicyLib::getStateSchemaUpdate(
      std::vector<policylib::StateSchemaElement> &updatedElements,
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


void policylib::PolicyLib::getFullStateSchema(std::vector<policylib::StateSchemaElement> &stateSchemaElements)
{

   scoped_lock(m_selfMutex);

   for(auto it = stateHash.begin(); it != stateHash.end(); it++)
   {
      std::string key(it->first.c_str());
      ElementData data = it->second;
      stateSchemaElements.push_back(StateSchemaElement(key, data));
   }
}

void policylib::PolicyLib::getFullState(std::vector<policylib::StateElement> &stateElements)
{
   scoped_lock(m_selfMutex);
   for(auto it = stateHash.begin(); it != stateHash.end(); it++)
   {
      stateElements.push_back(StateElement(it->first, ElementData(it->second)));
   }
}

void policylib::PolicyLib::fireStateSchemaElementAdded(std::string key,
                                                       const policylib::ElementData &data)
{
   scoped_lock(m_listenerHandlersMutex);
   policylib::StateSchemaElement element(key, data);

   m_stateSchemaListenerHandler.fireStateSchemaElementAdded(&element);
}

void policylib::PolicyLib::fireStateSchemaElementRemoved(std::string key,
                                                         const policylib::ElementData &data)
{
   scoped_lock(m_listenerHandlersMutex);
   policylib::StateSchemaElement element(key, data);
   m_stateSchemaListenerHandler.fireStateSchemaElementRemoved(&element);
}

void policylib::PolicyLib::fireStateSchemaElementModified(std::string key,
                                                          const policylib::ElementData &data)
{
   scoped_lock(m_listenerHandlersMutex);
   policylib::StateSchemaElement element(key, data);
   m_stateSchemaListenerHandler.fireStateSchemaElementModified(&element);
}

void policylib::PolicyLib::fireStateElementModified(std::string key,
                                                    const policylib::ElementData &data)
{
   scoped_lock(m_listenerHandlersMutex);
   policylib::StateElement element(key, data);
   m_stateListenerHandler.fireStateElementModified(&element);
}

/**
  \todo Make this dependent on device
  */
void policylib::PolicyLib::setGUILayout(policylib::gui::Element *rootElement, int device)
{
   m_gui = rootElement;
}

policylib::gui::Element* policylib::PolicyLib::getGUILayout(policylib::gui::Device device)
{
   if(m_gui == NULL)
   {
      makeDefaultGUILayout();
   }
   return m_gui;
}

std::unordered_set<std::string> policylib::PolicyLib::getRestrictionSet(std::string key)
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

bool policylib::PolicyLib::emptyRestrictionSet(std::string key)
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

void policylib::PolicyLib::releaseAllListeners()
{
}

policylib::StateSchemaElement policylib::PolicyLib::getStateSchemaElement(std::string key)
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

std::string policylib::PolicyLib::getElementMaxConstraint(std::string key) const
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

std::string policylib::PolicyLib::getElementMinConstraint(std::string key) const
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

void policylib::PolicyLib::makeDefaultGUILayout()
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

void policylib::PolicyLib::holdStateEvents()
{
   m_stateListenerHandler.holdEvents();
}

void policylib::PolicyLib::releaseStateEvents()
{
   m_stateListenerHandler.releaseEvents();
}

policylib::PolicyLib::mutex_type& policylib::PolicyLib::getPolicyMutex()
{
   return m_selfMutex;
}



