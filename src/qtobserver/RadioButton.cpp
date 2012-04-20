#include "tinia/qtobserver/moc/RadioButton.hpp"

namespace tinia {
namespace qtobserver {

RadioButton::RadioButton(std::string value, std::string key,
                         std::shared_ptr<policy::Policy> policy,
                         QWidget *parent) :
   QRadioButton(value.c_str(), parent), m_value(value), m_key(key), m_policy(policy)
{
   // Connect signals
   connect(this, SIGNAL(toggled(bool)), this, SLOT(setCheckedFromQt(bool)));
   connect(this, SIGNAL(setCheckedFromPolicy(bool)), this, SLOT(setChecked(bool)));

   m_policy->addStateListener(m_key, this);
   if(m_policy->getElementValueAsString(m_key) == m_value)
   {
      setChecked(true);
   }

}
RadioButton::~RadioButton()
{
   m_policy->removeStateListener(m_key, this);
}
}

void qtobserver::RadioButton::setCheckedFromQt(bool checked)
{
   if(checked)
   {
      m_policy->updateElementFromString(m_key, m_value);
   }
}

void qtobserver::RadioButton::stateElementModified(policy::StateElement *stateElement)
{
   if(stateElement->getStringValue() == m_value)
   {
      if(isChecked())
      {
         // Don't do anything, we're already good.
      }
      else
      {
         emit setCheckedFromPolicy(true);
      }

   }
}

} // of namespace tinia
