#include "tinia/qtobserver/moc/RadioButtonGroup.hpp"
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace tinia {
namespace qtobserver {

RadioButtonGroup::RadioButtonGroup(std::string key,
                           std::shared_ptr<policy::Policy> policy,
                                   bool horizontal,
                           QWidget *parent) :
   QGroupBox(parent), m_policy(policy), m_key(key)
{
   if(horizontal)
   {
      setLayout(new QHBoxLayout());
   }
   else
   {
      setLayout(new QVBoxLayout());
   }
   m_policy->addStateSchemaListener(m_key, this);

   // Populate the buttons
   auto restrictions = m_policy->getRestrictionSet(m_key);
   for(auto it= restrictions.begin(); it != restrictions.end(); it++)
   {
      layout()->addWidget(new RadioButton(*it, m_key, m_policy, this));
   }

   setFlat(true);
}

RadioButtonGroup::~RadioButtonGroup()
{
   m_policy->removeStateSchemaListener(m_key, this);
}

}





void qtobserver::RadioButtonGroup::stateSchemaElementAdded(policy::StateSchemaElement *stateSchemaElement)
{
}

void qtobserver::RadioButtonGroup::stateSchemaElementRemoved(policy::StateSchemaElement *stateSchemaElement)
{
}

void qtobserver::RadioButtonGroup::stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement)
{
   // Do something here eventually
}
} // of namespace tinia
