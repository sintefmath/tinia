#include "qtobserver/moc/ComboBox.hpp"

namespace qtobserver {

ComboBox::ComboBox(std::string key, std::shared_ptr<policylib::PolicyLib> policyLib,
                   QWidget *parent) :
   QComboBox(parent), m_policyLib(policyLib), m_key(key)
{
   // Set up signals
   connect(this, SIGNAL(setStateFromPolicy(int)), this,
           SLOT(setCurrentIndex(int)));
   connect(this, SIGNAL(activated(QString)), this,
           SLOT(activatedChanged(QString)));

   m_policyLib->addStateListener(m_key, this);
   m_policyLib->addStateSchemaListener(m_key, this);



   auto restrictionSet = m_policyLib->getRestrictionSet(m_key);
   for(auto it = restrictionSet.begin(); it != restrictionSet.end(); it++)
   {
      m_options.append(it->c_str());
   }
   m_options.sort();
   addItems(m_options);
   activated(m_options.indexOf(m_policyLib->getElementValueAsString(m_key).c_str()));
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}

} // namespace qtobserver

void qtobserver::ComboBox::stateElementModified(policylib::StateElement *stateElement)
{
   int index = m_options.indexOf(stateElement->getStringValue().c_str());
   emit setStateFromPolicy(index);
}

void qtobserver::ComboBox::stateSchemaElementAdded(policylib::StateSchemaElement *stateSchemaElement)
{
}

void qtobserver::ComboBox::stateSchemaElementModified(policylib::StateSchemaElement *stateSchemaElement)
{
}

void qtobserver::ComboBox::stateSchemaElementRemoved(policylib::StateSchemaElement *stateSchemaElement)
{
}

qtobserver::ComboBox::~ComboBox()
{
   m_policyLib->removeStateSchemaListener(m_key, this);
   m_policyLib->removeStateListener(m_key, this);

}

void qtobserver::ComboBox::activatedChanged(QString value)
{
   m_policyLib->updateElementFromString(m_key, value.toStdString());
}
