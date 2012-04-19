#include "qtobserver/moc/RadioButtons.hpp"
namespace qtobserver {
RadioButtons::RadioButtons(std::string key,
                           std::shared_ptr<policylib::PolicyLib> policyLib,
                           QWidget *parent) :
   QGroupBox(parent), m_policyLib(policyLib), m_key(key)
{

}

}

void qtobserver::RadioButtons::buttonChecked(bool)
{
}
