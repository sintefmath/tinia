#include "tinia/qtobserver/moc/Button.hpp"
#include "tinia/qtobserver/utils.hpp"

namespace tinia {
namespace qtobserver {

Button::Button(std::string key, std::shared_ptr<policy::Policy> policy,
               QWidget *parent) :
   QPushButton(parent), m_key(key), m_policy(policy)
{
   connect(this, SIGNAL(clicked()), this, SLOT(clickedButton()));
   setText(prettyName(key, m_policy).c_str());
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}

Button::~Button()
{
}

}

void qtobserver::Button::clickedButton()
{
   m_policy->updateElement<bool>(m_key, true);
}

} // of namespace tinia
