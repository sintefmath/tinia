#include "tinia/qtobserver/moc/CheckBox.hpp"
#include "tinia/qtobserver/utils.hpp"

namespace tinia {
namespace qtobserver {

CheckBox::CheckBox(std::string key, std::shared_ptr<policy::Policy> policy,
                   QWidget *parent) :
   QCheckBox(parent), m_key(key), m_policy(policy)
{
   setText(prettyName(m_key, m_policy).c_str());
   m_policy->addStateListener(m_key, this);
   bool checked;
   m_policy->getElementValue<bool>(m_key, checked);
   setChecked(checked);

   connect(this, SIGNAL(setCheckFromPolicy(bool)), this,
           SLOT(setChecked(bool)));
   connect(this, SIGNAL(toggled(bool)), this,
           SLOT(setCheckedFromQt(bool)));
}

CheckBox::~CheckBox()
{
   m_policy->removeStateListener(m_key, this);
}
}

void qtobserver::CheckBox::setCheckedFromQt(bool checked)
{
   m_policy->updateElement<bool>(m_key, checked);
}

void qtobserver::CheckBox::stateElementModified(policy::StateElement *stateElement)
{
   bool checked;
   stateElement->getValue(checked);
   emit setCheckFromPolicy(checked);
}

} // of namespace tinia
