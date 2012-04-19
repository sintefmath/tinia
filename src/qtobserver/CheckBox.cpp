#include "qtobserver/moc/CheckBox.hpp"
#include "qtobserver/utils.hpp"
namespace qtobserver {

CheckBox::CheckBox(std::string key, std::shared_ptr<policylib::PolicyLib> policyLib,
                   QWidget *parent) :
   QCheckBox(parent), m_key(key), m_policyLib(policyLib)
{
   setText(prettyName(m_key, m_policyLib).c_str());
   m_policyLib->addStateListener(m_key, this);
   bool checked;
   m_policyLib->getElementValue<bool>(m_key, checked);
   setChecked(checked);

   connect(this, SIGNAL(setCheckFromPolicy(bool)), this,
           SLOT(setChecked(bool)));
   connect(this, SIGNAL(toggled(bool)), this,
           SLOT(setCheckedFromQt(bool)));
}

CheckBox::~CheckBox()
{
   m_policyLib->removeStateListener(m_key, this);
}
}

void qtobserver::CheckBox::setCheckedFromQt(bool checked)
{
   m_policyLib->updateElement<bool>(m_key, checked);
}

void qtobserver::CheckBox::stateElementModified(policylib::StateElement *stateElement)
{
   bool checked;
   stateElement->getValue(checked);
   emit setCheckFromPolicy(checked);
}
