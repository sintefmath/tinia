#include "qtobserver/moc/RadioButton.hpp"

namespace qtobserver {

RadioButton::RadioButton(std::string value, std::string key,
                         std::shared_ptr<policylib::PolicyLib> policyLib,
                         QWidget *parent) :
   QRadioButton(value.c_str(), parent), m_value(value), m_key(key), m_policyLib(policyLib)
{
   // Connect signals
   connect(this, SIGNAL(toggled(bool)), this, SLOT(setCheckedFromQt(bool)));
   connect(this, SIGNAL(setCheckedFromPolicyLib(bool)), this, SLOT(setChecked(bool)));

   m_policyLib->addStateListener(m_key, this);
   if(m_policyLib->getElementValueAsString(m_key) == m_value)
   {
      setChecked(true);
   }

}
RadioButton::~RadioButton()
{
   m_policyLib->removeStateListener(m_key, this);
}
}

void qtobserver::RadioButton::setCheckedFromQt(bool checked)
{
   if(checked)
   {
      m_policyLib->updateElementFromString(m_key, m_value);
   }
}

void qtobserver::RadioButton::stateElementModified(policylib::StateElement *stateElement)
{
   if(stateElement->getStringValue() == m_value)
   {
      if(isChecked())
      {
         // Don't do anything, we're already good.
      }
      else
      {
         emit setCheckedFromPolicyLib(true);
      }

   }
}
