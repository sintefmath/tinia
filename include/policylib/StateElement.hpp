#ifndef STATEELEMENT_HPP
#define STATEELEMENT_HPP
#include "policylib/ElementData.hpp"
#include "policylib/ElementDataFactory.hpp"
#include <boost/shared_ptr.hpp>
#include <string>
namespace policylib {
class StateElement
{
public:
   typedef boost::property_tree::basic_ptree<std::string, StateElement> PropertyTree;

   StateElement();
   StateElement(std::string name, const ElementData& data);

   std::string getKey() const;
   std::string getXSDType() const;
   template<typename T>
   void getValue(T &t) const;

   std::string getStringValue() const;

   PropertyTree getPropertyTree() const;

private:
   std::string m_name;
   ElementData m_data;
   ElementDataFactory m_factory;
};
template<typename T>
void StateElement::getValue(T& t) const
{
   // TODO: Type checking
   m_factory.createT( m_data, t );
}
}
#endif // STATEELEMENT_HPP
