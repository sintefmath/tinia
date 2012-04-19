#ifndef QTOBSERVER_DOUBLESPINBOX_HPP
#define QTOBSERVER_DOUBLESPINBOX_HPP

#include <QDoubleSpinBox>
#include "policylib/PolicyLib.hpp"
#include "policylib/StateListener.hpp"
#include <memory>

namespace qtobserver {
/**
  \todo Use max and min if available.
  */
class DoubleSpinBox : public QDoubleSpinBox, public policylib::StateListener
{
    Q_OBJECT
public:
    explicit DoubleSpinBox(std::string key, std::shared_ptr<policylib::PolicyLib> policyLib,
                     QWidget *parent = 0);
   ~DoubleSpinBox();

   void stateElementModified(policylib::StateElement *stateElement);

signals:
   void setValueFromPolicy(double val);
public slots:
   void valueSetFromQt(double val);

private:
   std::string m_key;
   std::shared_ptr<policylib::PolicyLib> m_policyLib;
};
}

#endif // QTOBSERVER_DOUBLESPINBOX_HPP
