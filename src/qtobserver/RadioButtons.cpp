#include "tinia/qtobserver/moc/RadioButtons.hpp"

namespace tinia {
namespace qtobserver {
RadioButtons::RadioButtons(std::string key,
                           std::shared_ptr<policy::Policy> policy,
                           QWidget *parent) :
   QGroupBox(parent), m_policy(policy), m_key(key)
{

}

}

void qtobserver::RadioButtons::buttonChecked(bool)
{
}

} // of namespace tinia
