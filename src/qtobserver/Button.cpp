#include "tinia/qtobserver/moc/Button.hpp"
#include "tinia/qtobserver/utils.hpp"

namespace tinia {
namespace qtobserver {

Button::Button(std::string key, std::shared_ptr<policylib::PolicyLib> policyLib,
               QWidget *parent) :
   QPushButton(parent), m_key(key), m_policyLib(policyLib)
{
   connect(this, SIGNAL(clicked()), this, SLOT(clickedButton()));
   setText(prettyName(key, m_policyLib).c_str());
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}

Button::~Button()
{
}

}

void qtobserver::Button::clickedButton()
{
   m_policyLib->updateElement<bool>(m_key, true);
}

} // of namespace tinia
