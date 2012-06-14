#include "tinia/qtobserver/moc/ComboBox.hpp"

namespace tinia {
namespace qtobserver {

ComboBox::ComboBox(std::string key, std::shared_ptr<policy::Policy> policy,
                   QWidget *parent) :
   QComboBox(parent), m_policy(policy), m_key(key)
{
   // Set up signals
   connect(this, SIGNAL(setStateFromPolicy(int)), this,
           SLOT(setCurrentIndex(int)));
   connect(this, SIGNAL(activated(QString)), this,
           SLOT(activatedChanged(QString)));

   m_policy->addStateListener(m_key, this);
   m_policy->addStateSchemaListener(m_key, this);



   auto restrictionSet = m_policy->getRestrictionSet(m_key);
   for(auto it = restrictionSet.begin(); it != restrictionSet.end(); it++)
   {
      m_options.append(it->c_str());
   }
   m_options.sort();
   addItems(m_options);
   setCurrentIndex(m_options.indexOf(m_policy->getElementValueAsString(m_key).c_str()));
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}

} // namespace qtobserver

void qtobserver::ComboBox::stateElementModified(policy::StateElement *stateElement)
{
   int index = m_options.indexOf(stateElement->getStringValue().c_str());
   emit setStateFromPolicy(index);
}

void qtobserver::ComboBox::stateSchemaElementAdded(policy::StateSchemaElement *stateSchemaElement)
{
}

void qtobserver::ComboBox::stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement)
{
}

void qtobserver::ComboBox::stateSchemaElementRemoved(policy::StateSchemaElement *stateSchemaElement)
{
}

qtobserver::ComboBox::~ComboBox()
{
   m_policy->removeStateSchemaListener(m_key, this);
   m_policy->removeStateListener(m_key, this);

}

void qtobserver::ComboBox::activatedChanged(QString value)
{
   m_policy->updateElementFromString(m_key, std::string( value.toLocal8Bit() ));
}

} // of namespace tinia
