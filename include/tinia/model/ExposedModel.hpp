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

#ifndef POLICY_HPP
#define POLICY_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <stdexcept>
// #include <initializer_list> // This include makes Visual Studio sad.
#include <iostream>
#include <algorithm>
#include <type_traits>


#include <boost/lexical_cast.hpp>

#include <boost/thread.hpp>

#include "tinia/model/impl/ElementData.hpp"
#include "tinia/model/impl/TypeToXSDType.hpp"
#include "tinia/model/impl/ElementDataFactory.hpp"
#include "tinia/model/impl/StateListenerHandler.hpp"
#include "tinia/model/impl/StateSchemaListenerHandler.hpp"
#include "tinia/model/GUILayout.hpp"
#include <boost/property_tree/ptree.hpp>
#include "tinia/model/Viewer.hpp"
#include <tinia/model/exceptions/BoundsExceededException.hpp>
#include <tinia/model/exceptions/RestrictionException.hpp>
#include <tinia/model/exceptions/TypeException.hpp>

/** \mainpage
     ExposedModel is a small library used to create, update and query policies.
     A model is a set of typed-parameters with potential restrictions that is sent
     between a client and a server in a distributed application.

     The values in a model is expected to change often, and very often contains
     GUI-state, short strings or integral values.

     \section sec_compilation Compilation and installation
     Compilation is done by using cmake and to compile the package you need the following
     Ubuntu packages from Ubuntu 11.04:
     - libxml2-dev
     - cmake
     - libboost-test-dev (For unit tests).

     Once these are installed a simple
     \verbatim
        $ cmake .
        $ make \endverbatim
     Should be enough to compile it. You can optionally issue
     \verbatim
        $ make package \endverbatim
     Which will build a debian package which installs into /usr/lib and /usr/include/model .
     This package can be installed on your system with the following command
     \verbatim
        $ sudo dpkg -i ExposedModel<version>.deb \endverbatim

    \section sec_example Example

    \section sec_extension Extension

    In order to extend a complex type, e.g., the Viewer, follow these steps:
    1) Update the data structure (E.g., Viewer.hpp)
    2) Update the two template specializations for the type in impl::ElementDataFactory.hpp, 'createElement'
       and 'createT'.

    These instructions are up to date and tested as of 110912.

    \include main.cpp



  */
//
/** \namespace model Everything in model resides in the model namespace.
  */

namespace tinia {
namespace model {

typedef boost::property_tree::basic_ptree<std::string, std::string> StringStringPTree;

struct Viewer; // Forward declaration.
class ExposedModelLock; // Forward declartion

/** ExposedModel is responsible for holding the contents of a model. */
class ExposedModel {
   // The locking mechanism for ExposedModel
   friend class ExposedModelLock;
public:
   typedef class ExposedModelLock ExposedModelLock;
   ExposedModel();

   std::string getElementMaxConstraint(std::string key) const;
   std::string getElementMinConstraint(std::string key) const;



   StateSchemaElement getStateSchemaElement(std::string key);


   /**
     Releases _all_ listeners of the model.
     */
   void releaseAllListeners();


   /**
     \param key the key of the ExposedModelElement
     \return true if the element has an empty restriction set, false otherwise
     */
   bool emptyRestrictionSet(std::string key);

   /**
     \param key the key of the ExposedModelElement
     */
   std::set<std::string> getRestrictionSet(std::string key);

   /**
      Gets the full Schema in the current model.
      \param schemaElements the vector to put the SchemaElements
      */
   void getFullStateSchema(std::vector<StateSchemaElement> &stateSchemaElements);

   /**
      Gets the full State of the current model.
      \param stateElements the vector to put the StateElements.
      */
   void getFullState(std::vector<StateElement> &stateElements);

   /**
      The job can use this to get an update meant for the client. If a fundamental
      change has occured in the Schema, only the update from the point of the
      fundamental change is given.
      \param updatedElements A list of elements that has been updated in the stateHash
      \param has_revision Base number from which the delta is to be computed.
      */
   void getStateUpdate(std::vector<StateElement> &updatedElements,
                       const unsigned int has_revision);


   /**
      The job can use this to get an update meant for the client. If a fundamental
      change has occured in the Schema, only the update from the point of the
      fundamental change is given.
      \param updatedElements A list of elements that has been updated in the stateHash
      \param has_revision Base number from which the delta is to be computed.
      */
   void getStateSchemaUpdate(std::vector<StateSchemaElement> &updatedElements,
                             const unsigned int has_revision);


   /**
The job can use this to get an update meant for the client. This version are for desktop-jobs/controllers that do
      not have xml capability.
      \param updatedElements A list of elements that has been updated in the state, relative to the given revision number.
      \param has_revision Base number from which the delta is to be computed. The client needs updates for revisions after this.
      */
   void getExposedModelUpdate(std::vector< std::pair<std::string, impl::ElementData> > &updatedElements, const unsigned int has_revision ) const;


   /** Add an unconstrained element to the hash. This will update the StateSchema and State with the element.
      \param key The key for the element
      \param value The value for the element. The type of the value is deduced from the template parameter.
     */
   template<typename T>
   void
   addElement( std::string key, T value, const std::string annotation="" );

   /** Add 4x4 matrix to the model.
      \param matrixName The name of the matrix.
      \param matrixData pointer to a continious block of memory that is assumed to hold 16 floats in column-major mode.
      \throw model::tinia::ExistingKeyException if the matrixName is already in use.
      */
   void addMatrixElement( std::string matrixName, float const* matrixData );

   /** Add an annotation to an element. The annotation is assumed to be in english. */
   void
   addAnnotation( std::string key, std::string annotation );

   /** Add an annotation to an element. The annotation allows for multiple languages. Each language MUST
        be prefixed with the language code, eg addAnnotation( "timestep", { "en:Time step", "no:Tidssteg" ] );
        \throw tinia::model::KeyNotFoundException if the key is not in the model.
        \throw std::invalid_argument if the annotation strings are not prefixed with a language code.
    */
   template<class InputIterator>
   void
   addAnnotation( std::string key, const InputIterator &begin, const InputIterator& end);

   /** Add a constrained element to the hash. This will update the StateSchema and State with the element.
      \param key The key for the element
      \param value The value for the element. The type of the value is deduced from the template parameter.
      \param minConstraint the minimum-inclusive value of the element.
      \param maxConstraint the maximum-inclusuve value of the element.
      \throw tinia::model::BoundsExceededException if the constraint is violated.
     */
   template<typename T>
   void
   addConstrainedElement( std::string key, T value, T minConstraint, T maxConstraint, const std::string annotation="" );


   /** Add or updates contraints for an element. To ensure the contraints are valid, the programmer must supply
     a value in addition to the new constraints.
      \param key The key for the element
      \param value The value for the element. The type of the value is deduced from the template parameter.
      \param minConstraint the minimum-inclusive value of the element.
      \param maxConstraint the maximum-inclusuve value of the element.
      \throw tinia::model::BoundsExceededException if the constraint is violated.
     */
   template<typename T>
   void
   updateConstraints( std::string key, T value, T minConstraint, T maxConstraint );

   /** Update an element with a new value.
      \param key The key to update.
      \param value The value for the element. The type of the value is deduced from the template parameter.
      \throws KeyNotFoundException if the key does not exist
      \throws TypeException if the types does not match
      */
   template<typename T>
   void
   updateElement( std::string key, T value );

   /** Update a matrix with a new value.
      \param key The key to update
      \param matrixData Pointer to a continious block of memory that can holds 16 floats.
      \throw KeyNotFoundException if the key does not exist
      \throw TypeException if the element was something other than a matrix
      */
   void
   updateMatrixValue( std::string key, const float* matrixData );

   /** Remove an element from the model.
      \param key Name of element to remove.
      \throws KeyNotFoundException if the element is not in the model.
      */
   void
   removeElement( std::string key );

   /** Get a typed element value by reference.
      \param key Name of element to update.
      \param t Reference which will be set by the function.
      */
   template<class T>
   void
   getElementValue( std::string key, T& t );

   /** Get the contents of a matrix.
      \param key Name of an element added as a matrix.
      \param matrixData Pointer to a continious block of memory that can hold at least 16 floats.
      \throw KeyNotFoundException if the key does not exist
      \throw TypeException if the element is not added as a matrix.
      */
   void
   getMatrixValue( std::string key, float* matrixData ) const;

   /** Get an element value as string. */
   std::string getElementValueAsString( std::string key );

   /** Add an element with a restriction. */
   template<class T, class TContainer>
   void
   addElementWithRestriction( std::string key, T value, TContainer restrictions );

   template<class T, class InputIterator>
   void
   addElementWithRestriction( std::string key, T value, InputIterator begin,
                              InputIterator end );


   /** Check if an element with the given name is in the model.
      \param elementName Name of element.
      \return true if the element is in the model, false otherwise.
      */
   bool
   hasElement( std::string elementName ) const;

   /** Returning the revision number.
 \deprecated An controller should implement its own revision scheme.
*/
   inline int getRevisionNumber(void) const { scoped_lock(m_selfMutex); return revisionNumber; }


   /**
      Adds a listener to the StateSchema event. Note: The caller is responsible
      for removing the listener upon deletion of the listener.
      */
   void addStateSchemaListener(StateSchemaListener* listener);

   /**
    Removes a listener from the StateSchema event
    */
   void removeStateSchemaListener(StateSchemaListener* listener);

   /**
      Adds a listener to the StateSchema event. Note: The caller is responsible
      for removing the listener upon deletion of the listener.
      */
   void addStateSchemaListener(std::string key, StateSchemaListener* listener);

   /**
    Removes a listener from the StateSchema event
    */
   void removeStateSchemaListener(std::string key, StateSchemaListener* listener);

   /**
      Adds a listener to the State event. Note: The caller is responsible
      for removing the listener upon deletion of the listener.
      */
   void addStateListener(StateListener* listener);

   /**
    Removes a listener from the State event.
    */
   void removeStateListener(StateListener* listener);


   /**
      Adds a listener to the State event. Note: The caller is responsible
      for removing the listener upon deletion of the listener.
      */
   void addStateListener(std::string key, StateListener* listener);

   /**
    Removes a listener from the State event.
    */
   void removeStateListener(std::string key, StateListener* listener);


   /** Update an element with a new value given as a string.
     \param key The key to update.
     \param value The value for the element. The type of the value is already known, since this is an update only.
     \throws KeyNotFoundException if the key does not exist.
     */
   void updateElementFromString( const std::string &key, const std::string &value );


   /** Update an element with a new value given as a ptree containing ptrees and finally (name, value)-pairs.
     \param key The key to update.
     \param value The value for the element. The type of the value is already known, since this is an update only.
     \throws KeyNotFoundException if the key does not exist.
     \throws std::runtime_error if the value-tree has the wrong topology.
     */
   void updateElementFromPTree( const std::string &key, const StringStringPTree &value );


   /**
     Sets the GUI (represented by its root element) for the given device. Use
     model::gui::Device to specify this.
     */
   void setGUILayout(gui::Element *rootElement, int device);

   /**
     \return the GUI (represented by its root element) for the given device.
   */
   gui::Element* getGUILayout(gui::Device device);




private:

   /**
    * Finds the element in the Model, throws if it isn't found
    */
   impl::ElementData& findElementInternal(const std::string& key);

   /**
    * Finds the element in the Model, throws if it isn't found
    */
   const impl::ElementData& findElementInternal(const std::string& key) const;

   /**
     Holds the StateEvents until releaseStateEvents is called
     */
   void holdStateEvents();
   /**
     Releases the StateEvents
     */
   void releaseStateEvents();


   /**
  Used for internal adding, this implements (partial) error checking.
  \note: This does *not* lock on the mutex
  */
   template<typename T>
   impl::ElementData& addElementInternal(std::string key, T val);

   /** The revision number for the model is incremented, and the element that caused this increment gets
      its revision number set to the global value prior to this incrementation. */
   void incrementRevisionNumber(impl::ElementData &updatedElement);


   void addAnnotationHelper( std::string element, std::unordered_map<std::string, std::string>& );

   void addMatrixHelper( std::string key, const float* matrixData );

   std::unordered_map<std::string, impl::ElementData> stateHash;
//   std::unordered_map<std::string, impl::ElementData> stateHash;

   unsigned int revisionNumber;

   /** Helper function for element updating that handles the complexity of simple versus complex
        types. Just a wrapper around UpdateElementHelper-class.
       */
   template<class T>
   void updateElementHelper( std::string key, impl::ElementData& elementData, const T& value );


   template<class T>
   std::string stringify( const T& t );

   void addElementErrorChecking( std::string key ) const;
   void updateStateHash( std::string key, impl::ElementData& elmentData );

   impl::ElementDataFactory elementFactory;


   // Event handlers
   impl::StateSchemaListenerHandler m_stateSchemaListenerHandler;
   impl::StateListenerHandler m_stateListenerHandler;
   void fireStateSchemaElementAdded(std::string key, const impl::ElementData& data);
   void fireStateSchemaElementRemoved(std::string key, const impl::ElementData& data);
   void fireStateSchemaElementModified(std::string key, const impl::ElementData& data);
   void fireStateElementModified(std::string key, const impl::ElementData& data);


   // Locking made easy
   typedef boost::recursive_mutex mutex_type;
   typedef mutex_type::scoped_lock scoped_lock;
   mutex_type m_selfMutex;
   mutex_type m_listenerHandlersMutex;

   mutex_type& getExposedModelMutex();

   // Temporary GUILayout-structure:
   model::gui::Element* m_gui;
   /**
     Simple method for making a linear GUI system when none is provided by the
     job.
     \note Does not lock on the mutex
     */
   void makeDefaultGUILayout();

   // --- private utilities used during development ---
   // (must be public in order to be used from unit tests...)
public:
   void printCurrentState();
};

template<>
inline
std::string
ExposedModel::stringify<model::Viewer>( const Viewer& v ) {
   return "";
}

template<class T>
std::string
ExposedModel::stringify( const T& t ) {
   std::stringstream ss;
   ss << t;

   return ss.str();
}

template<typename T>
void
ExposedModel::addElement( std::string key, T value, const std::string annotation ) {

   impl::ElementData data;
   {
      scoped_lock(m_selfMutex);
      data = addElementInternal(key, value);
   }
   if( !annotation.empty() ) {
       addAnnotation( key, annotation );
   }
   fireStateSchemaElementAdded(key, data);
}


template<typename T>
void
ExposedModel::addConstrainedElement( std::string key, T value, T minConstraint, T maxConstraint, const std::string annotation ) {
    impl::checkBounds(value, minConstraint, maxConstraint);


   impl::ElementData data;
   { // Lock scope
      scoped_lock(m_selfMutex);
      impl::ElementData& elementData = addElementInternal(key, value);

      {
         std::stringstream ss;
         ss << minConstraint;
         elementData.setMinConstraint( ss.str() );
      }

      {
         std::stringstream ss;
         ss << maxConstraint;
         elementData.setMaxConstraint( ss.str() );
      }
      // Copy
      data = elementData;
   }
   addAnnotation( key, annotation );

   fireStateSchemaElementAdded(key, data);
}

template<class T, class TContainer>
void
ExposedModel::addElementWithRestriction( std::string key, T value, TContainer restrictions ) {
   addElementWithRestriction(key, value, restrictions.begin(), restrictions.end());
}

template<class T, class InputIterator>
void
ExposedModel::addElementWithRestriction( std::string key, T value, InputIterator start, InputIterator end) {
   std::set<T> restrictionSet( start, end );

   if ( restrictionSet.find( value ) == restrictionSet.end() ) {
       throw RestrictionException(boost::lexical_cast<std::string>(value));
   }

   std::set<std::string> restrictionStrings;

   for(auto it=restrictionSet.begin(); it!=restrictionSet.end(); it++)
   {

      std::string s = boost::lexical_cast<std::string>( *it );
      restrictionStrings.insert( s );
   }

   // The copy given to the listener outside the lock.
   impl::ElementData data;
   {
      scoped_lock(m_selfMutex);
      impl::ElementData& elementData = addElementInternal(key, value);
      elementData.setRestrictionSet( restrictionStrings );

      data = elementData;
   }
   fireStateSchemaElementAdded(key, data);
}


/** \class UpdateElementHelper
    \tparam B True if the type that will be passed i a complexType.


    UpdateElementHelper is responsible for performing the subpart of an elementUpdate that
    is different for simple and complexTypes. This is done by specializations of the
    operator(). This has to be realized as a class to allow for partial template spesialization.

  */
template<bool B>
struct UpdateElementHelper {
   UpdateElementHelper( std::unordered_map<std::string, impl::ElementData>& stateHash,
                        impl::ElementDataFactory& elementFactory )
      : stateHash( stateHash ),
        elementFactory( elementFactory )
   {}

   template<class T>
   void operator()( std::string key, impl::ElementData& elementData, const T& value );

   std::unordered_map<std::string, impl::ElementData>& stateHash;
   impl::ElementDataFactory& elementFactory;
};

template<>
template<class T>
inline
void
UpdateElementHelper<true>::operator()( std::string key, impl::ElementData& elementData, const T& value ) {
   elementData = elementFactory.createElement( value );
   stateHash[key] = elementData;
}


template<>
template<class T>
inline
void
UpdateElementHelper<false>::operator()( std::string key, impl::ElementData& elementData, const T& value ) {
   const std::string stringValue = boost::lexical_cast<std::string>( value );

   if ( elementData.invalidConstraints( value) ) {
       throw BoundsExceededException(boost::lexical_cast<std::string>(value), elementData.getMinConstraint(), elementData.getMaxConstraint());
   }

   if ( elementData.violatingRestriction(  value ) ) {
       throw RestrictionException(value);
   }

   stateHash[key].setStringValue( stringValue );
}

template<class T>
void
ExposedModel::updateElementHelper( std::string key, impl::ElementData& elementData, const T& value ) {
   // All classes except std::string are assumed to be complexTypes which require specialization
   // in impl::ElementDataFactory for serialization.
   UpdateElementHelper<std::is_class<T>::value && !std::is_same<std::string, T>::value >
         updater( stateHash, elementFactory );
   updater( key, elementData, value );
}

template<typename T>
void
ExposedModel::updateElement( std::string key, T value ) {

   impl::ElementData data;
   std::string stringValueBeforeUpdate;
   {// Lock block
      scoped_lock(m_selfMutex);



      std::string myType = impl::TypeToXSDType<T>::getTypename();
      impl::ElementData& elementData = findElementInternal(key);
      std::string storedType = elementData.getXSDType();

      if ( storedType != myType ) {
          throw TypeException(myType, storedType);
      }
      stringValueBeforeUpdate = elementData.getStringValue();
      updateElementHelper( key, elementData, value );

      incrementRevisionNumber( elementData );
      data = elementData;
   }

   // Only fire listeners when we have to (in the case of complex values, we fire
   // the event no matter what).
   if(stringValueBeforeUpdate != data.getStringValue() || data.isComplexType())
   {
      fireStateElementModified(key, data);
   }

}



template<typename T>
void
ExposedModel::getElementValue( std::string key, T& t ) {
   // The whole method locks the model. This could be optimized, but leaving it
   // like this for now.
   scoped_lock(m_selfMutex);


   auto& elementData = findElementInternal(key);
   auto myType = impl::TypeToXSDType<T>::getTypename();
   auto storedType = elementData.getXSDType();
   if ( storedType !=  myType ) {
       throw TypeException(myType, storedType);
   }


   elementFactory.createT( elementData, t );

}

template<typename T>
impl::ElementData& model::ExposedModel::addElementInternal(std::string key,
                                                      T value)
{

   addElementErrorChecking( key );

   auto elementData = elementFactory.createElement( value );

   updateStateHash( key, elementData );

   impl::ElementData& data = stateHash[key];
   return data;
}


template<class InputIterator>
void
ExposedModel::addAnnotation( std::string key, const InputIterator& begin, const InputIterator& end ) {
   using std::string;
   std::unordered_map<std::string, std::string> annotationMap;

   std::for_each( begin, end, [&]( std::string s ) {
                  auto pos = s.find( ":" );
         if ( pos == string::npos ) {
      throw std::invalid_argument( "Trying to set annotation that is not qualified with language code." );
   }

   string lang( s.begin(), s.begin() + pos );
   string annotation( s.begin() + pos + 1, s.end() );
   annotationMap[lang] = annotation;
}
);

addAnnotationHelper( key, annotationMap );
}

template<typename T>
void
ExposedModel::updateConstraints( std::string key, T value, T minValue, T maxValue) {
    impl::checkBounds(value, minValue, maxValue);

    impl::ElementData data;
    bool emitChange = false;
    bool emitValueChange = false;
    { // Lock block
        scoped_lock(m_selfMutex);

        auto& elementData = findElementInternal(key);

        {
            std::stringstream ss;
            ss << minValue;
            if( ss.str() != elementData.getMinConstraint() ) {
                emitChange = true;
            }
            elementData.setMinConstraint(ss.str());
        }
        {
            std::stringstream ss;
            ss << maxValue;
            if( ss.str() != elementData.getMaxConstraint() ) {
                emitChange = true;
            }
            elementData.setMaxConstraint(ss.str());
        }
        {
            std::stringstream ss;
            ss << value;
            if( ss.str() != elementData.getStringValue()) {
                emitValueChange = true;
            }
            elementData.setStringValue(ss.str());
        }
        if( emitChange || emitValueChange ) {
            data = elementData;
        }
    }

    if(emitChange) {
        fireStateSchemaElementModified( key, data );
    }
    if(emitValueChange) {
        fireStateElementModified( key, data );
    }
}
} }

#endif // POLICY_HPP


