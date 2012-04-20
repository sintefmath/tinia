#ifndef QTOBSERVER_BUTTON_HPP
#define QTOBSERVER_BUTTON_HPP

#include <QPushButton>
#include <tinia/policylib/PolicyLib.hpp>
#include <tinia/policylib/StateListener.hpp>
#include <memory>

namespace qtobserver {
// Note: This is *not* a statelistener
class Button : public QPushButton
{
    Q_OBJECT
public:
    explicit Button(std::string key, std::shared_ptr<policylib::PolicyLib> policyLib,
                    QWidget *parent = 0);

   ~Button();
signals:

public slots:
   void clickedButton();
private:
   std::string m_key;
   std::shared_ptr<policylib::PolicyLib> m_policyLib;

};

} // namespace qtobserver

#endif // QTOBSERVER_BUTTON_HPP
