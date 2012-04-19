#include "qtobserver/moc/RadioButtonGroup.hpp"
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
namespace qtobserver {
RadioButtonGroup::RadioButtonGroup(std::string key,
                           std::shared_ptr<policylib::PolicyLib> policyLib,
                                   bool horizontal,
                           QWidget *parent) :
   QGroupBox(parent), m_policyLib(policyLib), m_key(key)
{
   if(horizontal)
   {
      setLayout(new QHBoxLayout());
   }
   else
   {
      setLayout(new QVBoxLayout());
   }
   m_policyLib->addStateSchemaListener(m_key, this);

   // Populate the buttons
   auto restrictions = m_policyLib->getRestrictionSet(m_key);
   for(auto it= restrictions.begin(); it != restrictions.end(); it++)
   {
      layout()->addWidget(new RadioButton(*it, m_key, m_policyLib, this));
   }

   setFlat(true);
}

RadioButtonGroup::~RadioButtonGroup()
{
   m_policyLib->removeStateSchemaListener(m_key, this);
}

}





void qtobserver::RadioButtonGroup::stateSchemaElementAdded(policylib::StateSchemaElement *stateSchemaElement)
{
}

void qtobserver::RadioButtonGroup::stateSchemaElementRemoved(policylib::StateSchemaElement *stateSchemaElement)
{
}

void qtobserver::RadioButtonGroup::stateSchemaElementModified(policylib::StateSchemaElement *stateSchemaElement)
{
   // Do something here eventually
}
