#ifndef QTOBSERVER_BUTTON_HPP
#define QTOBSERVER_BUTTON_HPP

#include <QPushButton>
#include <tinia/policy/Policy.hpp>
#include <tinia/policy/StateListener.hpp>
#include <memory>

namespace tinia {
namespace qtobserver {
// Note: This is *not* a statelistener
class Button : public QPushButton
{
    Q_OBJECT
public:
    explicit Button(std::string key, std::shared_ptr<policy::Policy> policy,
                    QWidget *parent = 0);

   ~Button();
signals:

public slots:
   void clickedButton();
private:
   std::string m_key;
   std::shared_ptr<policy::Policy> m_policy;

};

} // namespace qtobserver
} // namespace tinia
#endif // QTOBSERVER_BUTTON_HPP
