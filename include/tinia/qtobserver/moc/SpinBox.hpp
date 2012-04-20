#ifndef QTOBSERVER_SPINBOX_HPP
#define QTOBSERVER_SPINBOX_HPP

#include <QSpinBox>
#include "tinia/policylib/PolicyLib.hpp"
#include "tinia/policylib/StateListener.hpp"
#include <memory>

namespace qtobserver {
/**
  \todo Use max and min if available.
  */
class SpinBox : public QSpinBox, public policylib::StateListener
{
    Q_OBJECT
public:
    explicit SpinBox(std::string key, std::shared_ptr<policylib::PolicyLib> policyLib,
                     QWidget *parent = 0);
   ~SpinBox();

   void stateElementModified(policylib::StateElement *stateElement);

signals:
   void setValueFromPolicy(int val);
public slots:
   void valueSetFromQt(int val);

private:
   std::string m_key;
   std::shared_ptr<policylib::PolicyLib> m_policyLib;
};
}
#endif // QTOBSERVER_SPINBOX_HPP
